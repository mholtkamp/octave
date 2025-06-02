
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
static bool sUpdatingMouse = false;

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
        props.push_back(Property(DatumType::Bool, "Handle Mouse", this, &mHandleMouseInput, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Auto Size Text", this, &mAutoSizeText, 1, HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Right Click Press", this, &mRightClickPress, 1, HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Text");
        props.push_back(Property(DatumType::Byte, "Text-Anchor", this, &mText->mAnchorMode, 1, Widget::HandlePropChange, 0, int32_t(AnchorMode::Count), Widget::sAnchorModeStrings));
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

    if (mHandleMouseInput &&
        mState != ButtonState::Locked &&
        ShouldHandleInput())
    {
        sUpdatingMouse = true;

        const bool containsMouse = ContainsMouse();
        const bool mouseDown = IsMouseButtonDown(MOUSE_LEFT) || (mRightClickPress && IsMouseButtonDown(MOUSE_RIGHT));
        const bool mouseJustDown = IsMouseButtonJustDown(MOUSE_LEFT) || (mRightClickPress && IsMouseButtonJustDown(MOUSE_RIGHT));
        const bool mouseJustUp = IsMouseButtonJustUp(MOUSE_LEFT) || (mRightClickPress && IsMouseButtonJustUp(MOUSE_RIGHT));

        if (containsMouse)
        {
            if (mouseJustUp &&
                mState == ButtonState::Pressed)
            {
                Activate();
                SetState(ButtonState::Hovered);
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
        else if (sSelectedButton != this)
        {
            SetState(ButtonState::Normal);
        }

        sUpdatingMouse = false;
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

ButtonState Button::GetState()
{
    return mState;
}

void Button::SetState(ButtonState newState)
{
    if (mState != newState)
    {
        ButtonState oldState = mState;
        mState = newState;
        MarkDirty();

        if (newState == ButtonState::Hovered)
        {
            EmitSignal("Hovered", {});
        }
        else if (newState == ButtonState::Pressed)
        {
            EmitSignal("Pressed", {});
        }

        if (!sUpdatingMouse)
        {
            bool selected = (mState == ButtonState::Hovered || mState == ButtonState::Pressed);
            if (selected)
            {
                sSelectedButton = ResolveWeakPtr<Button>(this);
            }
            else if (sSelectedButton == this)
            {
                // This button was programmatically selected, now it isn't.
                sSelectedButton = nullptr;
            }
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

void Button::SetUseQuadStateColor(bool inUse)
{
    mUseQuadStateColor = inUse;
}

void Button::SetUseTextStateColor(bool inUse)
{
    mUseTextStateColor = inUse;
}

void Button::SetHandleMouseInput(bool inHandle)
{
    mHandleMouseInput = inHandle;
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
    EmitSignal("Activated", {});
    Datum selfArg = this;
    const Datum* args[] = { &selfArg };
    ScriptUtils::CallMethod(this, "OnActivated", OCT_ARRAY_SIZE(args), args, nullptr);
}
