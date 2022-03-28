
#include "Widgets/Button.h"
#include "Widgets/Quad.h"
#include "Widgets/Text.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"
#include "ScriptEvent.h"

#include "Components/ScriptComponent.h"

FORCE_LINK_DEF(Button);
DEFINE_FACTORY(Button, Widget);

Button::Button() :
    mNormalTexture(nullptr),
    mHoveredTexture(nullptr),
    mPressedTexture(nullptr),
    mDisabledTexture(nullptr),
    mNormalColor({1, 1, 1, 1}),
    mHoveredColor({1, 1, 0.5f, 1}),
    mPressedColor({1, 1, 0, 1}),
    mDisabledColor({0.5f, 0.5f, 0.5f, 1}),
    mState(ButtonState::Normal),
    mStateColorChangeSpeed(5.0f),
    mUseTextStateColor(false),
    mUseQuadStateColor(true),
    mHandleMouseInput(true),
    mAutoSizeText(true),
    mRightClickPress(false),
    mTextPaddingRatio(glm::vec2(0.035f, 0.05f)),
    mQuad(nullptr),
    mText(nullptr)

{
    mUseScissor = true;

    mQuad = new Quad();
    mText = new Text();

    AddChild(mQuad);
    AddChild(mText);

    mQuad->SetAnchorMode(AnchorMode::FullStretch);
    mText->SetAnchorMode(AnchorMode::FullStretch);
    mQuad->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mText->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);

    // Default dimensions
    SetDimensions(100, 30);

    mQuad->SetColor({ 0.2f, 0.2f, 0.8f, 1.0f },
        { 0.2f, 0.2f, 0.8f, 1.0f },
        { 0.5f, 0.2f, 0.8f, 0.5f },
        { 0.5f, 0.2f, 0.8f, 0.5f });
;}

Button::~Button()
{
    // Children are deleted automatically in ~Widget().
}

void Button::Update()
{
    Widget::Update();

    if (mHandleMouseInput &&
        mState != ButtonState::Disabled &&
        ShouldHandleInput())
    {
        const bool containsMouse = ContainsMouse();
        const bool mouseDown = IsMouseButtonDown(MOUSE_LEFT) || (mRightClickPress && IsMouseButtonDown(MOUSE_RIGHT));
        const bool mouseJustDown = IsMouseButtonJustDown(MOUSE_LEFT) || (mRightClickPress && IsMouseButtonJustDown(MOUSE_RIGHT));
        const bool mouseJustUp = IsMouseButtonJustUp(MOUSE_LEFT) || (mRightClickPress && IsMouseButtonJustUp(MOUSE_RIGHT));

        if (containsMouse)
        {
            if (mouseJustUp &&
                mState == ButtonState::Pressed)
            {
                OnPressed();
            }
            else if (mouseJustDown)
            {
                SetState(ButtonState::Pressed);
            }
            else if (!mouseDown)
            {
                SetState(ButtonState::Hovered);
                OnHover();
            }
        }
        else
        {
            SetState(ButtonState::Normal);
        }
    }

    if (IsDirty())
    {
        UpdateAppearance();
    }
}

void Button::UpdateAppearance()
{
    glm::vec4 stateColor = glm::vec4(1, 1, 1, 1);
    Texture* quadTexture = nullptr;

    switch (mState)
    {
    case ButtonState::Normal:
        quadTexture = mNormalTexture;
        stateColor = mNormalColor;
        break;
    case ButtonState::Hovered:
        quadTexture = mHoveredTexture;
        stateColor = mHoveredColor;
        break;
    case ButtonState::Pressed:
        quadTexture = mPressedTexture;
        stateColor = mPressedColor;
        break;
    case ButtonState::Disabled:
        quadTexture = mDisabledTexture;
        stateColor = mDisabledColor;
        break;
    default:
        break;
    }

    mQuad->SetTexture(quadTexture);

    if (mUseQuadStateColor)
    {
        mQuad->SetTint(stateColor);
    }

    if (mUseTextStateColor)
    {
        mText->SetColor(stateColor);
    }
}

void Button::SetPosition(float x, float y)
{
    Widget::SetPosition(x, y);
    mQuad->SetPosition(0, 0);
}

void Button::SetDimensions(float width, float height)
{
    Widget::SetDimensions(width, height);
    mQuad->SetDimensions(width, height);
    mText->SetDimensions(width, height);

    if (mAutoSizeText)
    {
        const float textScaleFactor = 0.75f;
        const float textSize = textScaleFactor * height * (1 - 2 * mTextPaddingRatio.y);
        mText->SetSize(textSize);
    }

    mText->SetPosition(width * mTextPaddingRatio.x, height * mTextPaddingRatio.y);
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

        // If not handling mouse input, then call these when SetState is manually called.
        if (!mHandleMouseInput)
        {
            if (newState == ButtonState::Hovered && 
                (mHoveredHandler.mFuncPointer != nullptr || mHoveredHandler.mScriptTableName != ""))
            {
                OnHover();
            }
            else if (newState == ButtonState::Pressed && 
                (mPressedHandler.mFuncPointer != nullptr || mPressedHandler.mScriptTableName != ""))
            {
                OnPressed();
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

void Button::SetDisabledTexture(Texture* texture)
{
    if (mDisabledTexture != texture)
    {
        mDisabledTexture = texture;
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

void Button::SetDisabledColor(glm::vec4 color)
{
    if (mDisabledColor != color)
    {
        mDisabledColor = color;
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

void Button::SetHoverHandler(ButtonHandlerFP newHandler)
{
    mHoveredHandler.mFuncPointer = newHandler;
}

void Button::SetPressedHandler(ButtonHandlerFP newHandler)
{
    mPressedHandler.mFuncPointer = newHandler;
}

void Button::SetScriptHoverHandler(const char* tableName, const char* funcName)
{
    mHoveredHandler.mScriptTableName = tableName;
    mHoveredHandler.mScriptFuncName = funcName;
}

void Button::SetScriptPressedHandler(const char* tableName, const char* funcName)
{
    mPressedHandler.mScriptTableName = tableName;
    mPressedHandler.mScriptFuncName = funcName;
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

void Button::OnPressed()
{
    if (mPressedHandler.mFuncPointer != nullptr)
    {
        mPressedHandler.mFuncPointer(this);
    }
    if (mPressedHandler.mScriptTableName != "")
    {
        ScriptEvent::WidgetState(
            mPressedHandler.mScriptTableName,
            mPressedHandler.mScriptFuncName,
            this);
    }
}

void Button::OnHover()
{
    if (mHoveredHandler.mFuncPointer != nullptr)
    {
        mHoveredHandler.mFuncPointer(this);
    }
    if (mHoveredHandler.mScriptTableName != "")
    {
        ScriptEvent::WidgetState(
            mHoveredHandler.mScriptTableName,
            mHoveredHandler.mScriptFuncName,
            this);
    }
}
