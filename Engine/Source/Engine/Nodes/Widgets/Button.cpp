
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"
#include "Script.h"
#include "ScriptUtils.h"
#include "LuaBindings/Node_Lua.h"

FORCE_LINK_DEF(Button);
DEFINE_NODE(Button, Widget);

// Button that can be "selected" programmatically
// Will not become unhovered when mouse is off of it.
WeakPtr<Button> Button::sSelectedButton;
bool Button::sSelButtonChangedThisFrame = false;

bool Button::sHandleMouseInput = true;
bool Button::sHandleGamepadInput = true;
bool Button::sHandleKeyboardInput = true;

Button* Button::GetSelectedButton()
{
    return sSelectedButton.Get();
}

void Button::SetSelectedButton(Button* button)
{
    if (sSelectedButton != button)
    {
        Button* oldSel = sSelectedButton.Get();
        sSelectedButton = ResolveWeakPtr<Button>(button);

        if (oldSel != nullptr)
        {
            oldSel->SetState(ButtonState::Normal);
        }

        if (sSelectedButton)
        {
            sSelectedButton->SetState(ButtonState::Hovered);
        }

        sSelButtonChangedThisFrame = true;
    }
}

void Button::StaticUpdate()
{
    sSelButtonChangedThisFrame = false;
}

bool Button::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Button* button = static_cast<Button*>(prop->mOwner);
    bool success = false;

    // Just always mark the button dirty?
    button->MarkDirty();

    return success;
}

void Button::Create()
{
    Super::Create();

    SetName("Button");
    mUseScissor = true;

    mQuad = CreateChild<Quad>("Quad");
    mText = CreateChild<Text>("Text");

    mQuad->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mQuad->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
    mText->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
    mQuad->SetTransient(true);
    mText->SetTransient(true);

    mText->SetHorizontalJustification(Justification::Center);
    mText->SetVerticalJustification(Justification::Center);
    mQuad->SetColor(mNormalColor);

#if EDITOR
    mQuad->mHiddenInTree = true;
    mText->mHiddenInTree = true;
#endif

    // Default dimensions
    SetDimensions(100, 30);
}

void Button::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("Button");

        props.push_back(Property(DatumType::Asset, "Normal Texture", this, &mNormalTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Normal Color", this, &mNormalColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Asset, "Hovered Texture", this, &mHoveredTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Hovered Color", this, &mHoveredColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Asset, "Pressed Texture", this, &mPressedTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Pressed Color", this, &mPressedColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Asset, "Locked Texture", this, &mLockedTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Locked Color", this, &mLockedColor, 1, HandlePropChange));

        props.push_back(Property(DatumType::Bool, "Text State Color", this, &mUseTextStateColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Quad State Color", this, &mUseQuadStateColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Auto Size Text", this, &mAutoSizeText, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Right Click Press", this, &mRightClickPress, 1, HandlePropChange));

        props.push_back(Property(DatumType::Node, "Nav Up", this, &mNavUp));
        props.push_back(Property(DatumType::Node, "Nav Down", this, &mNavDown));
        props.push_back(Property(DatumType::Node, "Nav Left", this, &mNavLeft));
        props.push_back(Property(DatumType::Node, "Nav Right", this, &mNavRight));
    }

    {
        SCOPED_CATEGORY("Text");
        props.push_back(Property(DatumType::Byte, "Text-Anchor", this, &mText->mAnchorMode, 1, Widget::HandlePropChange, NULL_DATUM, int32_t(AnchorMode::Count), Widget::sAnchorModeStrings));
        props.push_back(Property(DatumType::Vector2D, "Text-Offset", this, &mText->mOffset, 1, Widget::HandlePropChange));
        props.push_back(Property(DatumType::Vector2D, "Text-Size", this, &mText->mSize, 1, Widget::HandlePropChange));
        props.push_back(Property(DatumType::Color, "Text-Color", this, &mText->mColor, 1, HandlePropChange));
        mText->GatherTextProperties(props);
    }

    {
        SCOPED_CATEGORY("Quad");
        props.push_back(Property(DatumType::Color, "Quad-Color", this, &mQuad->mColor, 1, HandlePropChange));
        props.push_back(Property(DatumType::Byte, "Quad-Opacity", this, &mOpacity, 1, Widget::HandlePropChange));
        mQuad->GatherQuadProperties(props);
    }
}

void Button::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (sHandleMouseInput &&
        mState != ButtonState::Locked &&
        ShouldHandleInput())
    {
        // Need accurate Rect to detect if cursor is inside mouse
        // Was seeing issue where mouse at 0,0 would always overlap buttons that
        // were just spawned in.
        if (IsDirty())
        {
            UpdateRect();
        }

        bool containsMouse = ContainsMouse();
        const bool mouseDown = IsPointerDown(0) || (mRightClickPress && IsMouseButtonDown(MOUSE_RIGHT));
        const bool mouseJustDown = IsPointerJustDown(0) || (mRightClickPress && IsMouseButtonJustDown(MOUSE_RIGHT));
        const bool mouseJustUp = IsPointerJustUp(0) || (mRightClickPress && IsMouseButtonJustUp(MOUSE_RIGHT));

#if PLATFORM_3DS || PLATFORM_ANDROID
        if (!mouseJustUp && !IsPointerDown(0))
        {
            containsMouse = false;
        }
#endif

        if (containsMouse)
        {
            SetSelectedButton(this);

            if (mouseJustUp &&
                mState == ButtonState::Pressed)
            {
                Activate();

                // Handle situation where activating the button
                // changed the selected button.
                if (GetSelectedButton() == this)
                {
                    SetState(ButtonState::Hovered);
                }
            }
            else if (mouseJustDown)
            {
                SetState(ButtonState::Pressed);
            }
            else if (!mouseDown)
            {
                SetState(ButtonState::Hovered);
            }
        }
        else if (sSelectedButton == this && mouseDown)
        {
            sSelectedButton->SetState(ButtonState::Hovered);
        }
    }

    if (sSelectedButton == this &&
        !sSelButtonChangedThisFrame)
    {
        if (sHandleGamepadInput)
        {
            // Allow activating the button with A button
            if (IsGamepadButtonJustDown(GAMEPAD_A, 0))
            {
                Activate();
            }

            // Allow hopping to different options based on up/down/left/right input
            if (mNavUp.IsValid() &&
                (IsGamepadButtonJustDown(GAMEPAD_L_UP, 0) || IsGamepadButtonJustDown(GAMEPAD_UP, 0)))
            {
                SetSelectedButton(mNavUp.Get());
            }

            if (mNavDown.IsValid() &&
                (IsGamepadButtonJustDown(GAMEPAD_L_DOWN, 0) || IsGamepadButtonJustDown(GAMEPAD_DOWN, 0)))
            {
                SetSelectedButton(mNavDown.Get());
            }

            if (mNavLeft.IsValid() &&
                (IsGamepadButtonJustDown(GAMEPAD_L_LEFT, 0) || IsGamepadButtonJustDown(GAMEPAD_LEFT, 0)))
            {
                SetSelectedButton(mNavLeft.Get());
            }

            if (mNavRight.IsValid() &&
                (IsGamepadButtonJustDown(GAMEPAD_L_RIGHT, 0) || IsGamepadButtonJustDown(GAMEPAD_RIGHT, 0)))
            {
                SetSelectedButton(mNavRight.Get());
            }
        }

        if (sHandleKeyboardInput)
        {
            if (IsKeyJustDown(KEY_SPACE) || IsKeyJustDown(KEY_ENTER))
            {
                Activate();
            }

            // Allow hopping to different options based on up/down/left/right input
            if (mNavUp.IsValid() &&
                IsKeyJustDown(KEY_UP))
            {
                SetSelectedButton(mNavUp.Get());
            }

            if (mNavDown.IsValid() &&
                IsKeyJustDown(KEY_DOWN))
            {
                SetSelectedButton(mNavDown.Get());
            }

            if (mNavLeft.IsValid() &&
                IsKeyJustDown(KEY_LEFT))
            {
                SetSelectedButton(mNavLeft.Get());
            }

            if (mNavRight.IsValid() &&
                IsKeyJustDown(KEY_RIGHT))
            {
                SetSelectedButton(mNavRight.Get());
            }
        }
    }
}

void Button::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void Button::UpdateAppearance()
{
    glm::vec4 stateColor = mNormalColor;
    Texture* quadTexture = mNormalTexture.Get<Texture>();

    switch (mState)
    {
    case ButtonState::Normal:
        quadTexture = mNormalTexture.Get<Texture>();
        stateColor = mNormalColor;
        break;
    case ButtonState::Hovered:
        quadTexture = mHoveredTexture.Get<Texture>();
        stateColor = mHoveredColor;
        break;
    case ButtonState::Pressed:
        quadTexture = mPressedTexture.Get<Texture>();
        stateColor = mPressedColor;
        break;
    case ButtonState::Locked:
        quadTexture = mLockedTexture.Get<Texture>();
        stateColor = mLockedColor;
        break;
    default:
        break;
    }

    // If a given state's texture is null, fall back to normal texture.
    // If that is still null, then it will be rendered as pure white.
    if (quadTexture == nullptr)
    {
        quadTexture = mNormalTexture.Get<Texture>();
    }

    mQuad->SetTexture(quadTexture);

    if (mUseQuadStateColor)
    {
        mQuad->SetColor(stateColor);
    }

    if (mUseTextStateColor)
    {
        mText->SetColor(stateColor);
    }

    if (mAutoSizeText)
    {
        const float textScaleFactor = 0.75f;
        const float textSize = textScaleFactor * GetHeight() * (1 - 2 * mTextPaddingRatio.y);
        mText->SetTextSize(textSize);
    }

    //mText->SetPosition(GetWidth() * mTextPaddingRatio.x, GetHeight() * mTextPaddingRatio.y);
}

bool Button::IsSelected() const
{
    return (Button::GetSelectedButton() == this);
}

ButtonState Button::GetState()
{
    return mState;
}

void Button::SetState(ButtonState newState)
{
    if (mState != newState)
    {
        mState = newState;
        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("StateChanged", { this });
            CallFunction("OnStateChanged", { this });
        }
    }
}

void Button::SetNormalTexture(Texture* texture)
{
    if (mNormalTexture != texture)
    {
        mNormalTexture = texture;
        MarkDirty();
    }
}

void Button::SetHoveredTexture(Texture* texture)
{
    if (mHoveredTexture != texture)
    {
        mHoveredTexture = texture;
        MarkDirty();
    }
}

void Button::SetPressedTexture(Texture* texture)
{
    if (mPressedTexture != texture)
    {
        mPressedTexture = texture;
        MarkDirty();
    }
}

void Button::SetLockedTexture(Texture* texture)
{
    if (mLockedTexture != texture)
    {
        mLockedTexture = texture;
        MarkDirty();
    }
}

Texture* Button::GetNormalTexture()
{
    return mNormalTexture.Get<Texture>();
}

Texture* Button::GetHoveredTexture()
{
    return mHoveredTexture.Get<Texture>();
}

Texture* Button::GetPressedTexture()
{
    return mPressedTexture.Get<Texture>();
}

Texture* Button::GetLockedTexture()
{
    return mLockedTexture.Get<Texture>();
}

void Button::SetNormalColor(glm::vec4 color)
{
    if (mNormalColor != color)
    {
        mNormalColor = color;
        MarkDirty();
    }
}

void Button::SetHoveredColor(glm::vec4 color)
{
    if (mHoveredColor != color)
    {
        mHoveredColor = color;
        MarkDirty();
    }
}

void Button::SetPressedColor(glm::vec4 color)
{
    if (mPressedColor != color)
    {
        mPressedColor = color;
        MarkDirty();
    }
}

void Button::SetLockedColor(glm::vec4 color)
{
    if (mLockedColor != color)
    {
        mLockedColor = color;
        MarkDirty();
    }
}

glm::vec4 Button::GetNormalColor()
{
    return mNormalColor;
}

glm::vec4 Button::GetHoveredColor()
{
    return mHoveredColor;
}

glm::vec4 Button::GetPressedColor()
{
    return mPressedColor;
}

glm::vec4 Button::GetLockedColor()
{
    return mLockedColor;
}

void Button::SetUseQuadStateColor(bool inUse)
{
    mUseQuadStateColor = inUse;
}

void Button::SetUseTextStateColor(bool inUse)
{
    mUseTextStateColor = inUse;
}

void Button::SetHandleMouse(bool inHandle)
{
    sHandleMouseInput = inHandle;
}

void Button::SetHandleGamepad(bool inHandle)
{
    sHandleGamepadInput = inHandle;
}

void Button::SetHandleKeyboard(bool inHandle)
{
    sHandleKeyboardInput = inHandle;
}

void Button::EnableRightClickPress(bool enable)
{
    mRightClickPress = enable;
}

bool Button::IsRightClickPressEnabled()
{
    return mRightClickPress;
}

void Button::SetTextString(const std::string& newTextString)
{
    mText->SetText(newTextString);
}

const std::string& Button::GetTextString() const
{
    return mText->GetText();
}

void Button::SetNavUp(Node* up)
{
    mNavUp = ResolveWeakPtr<Button>(up);
}

void Button::SetNavDown(Node* down)
{
    mNavDown = ResolveWeakPtr<Button>(down);
}

void Button::SetNavLeft(Node* left)
{
    mNavLeft = ResolveWeakPtr<Button>(left);
}

void Button::SetNavRight(Node* right)
{
    mNavRight = ResolveWeakPtr<Button>(right);
}

Node* Button::GetNavUp()
{
    return mNavUp.Get();
}

Node* Button::GetNavDown()
{
    return mNavDown.Get();
}

Node* Button::GetNavLeft()
{
    return mNavLeft.Get();
}

Node* Button::GetNavRight()
{
    return mNavRight.Get();
}

Text* Button::GetText()
{
    return mText;
}

Quad* Button::GetQuad()
{
    return mQuad;
}

void Button::Activate()
{
    EmitSignal("Activated", { this });
    CallFunction("OnActivated", { this });
}
