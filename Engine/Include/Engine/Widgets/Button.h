#pragma once

#include "Widgets/Widget.h"
#include "ScriptFunc.h"

class Texture;
class Quad;
class Text;

typedef void(*ButtonHandlerFP)(class Button* button);

enum class ButtonState
{
    Normal,
    Hovered,
    Pressed,
    Disabled,
    Num
};

class Button : public Widget
{
public:

    DECLARE_WIDGET(Button, Widget);

    Button();
    virtual ~Button();

    virtual void Update() override;
    void UpdateAppearance();

    // These functions need to adjust quad/text widget pos/dim.
    virtual void SetPosition(float x, float y) override;
    virtual void SetDimensions(float width, float height) override;

    ButtonState GetState();
    virtual void SetState(ButtonState newState);

    void SetNormalTexture(Texture* texture);
    void SetHoveredTexture(Texture* texture);
    void SetPressedTexture(Texture* texture);
    void SetDisabledTexture(Texture* texture);

    void SetNormalColor(glm::vec4 color);
    void SetHoveredColor(glm::vec4 color);
    void SetPressedColor(glm::vec4 color);
    void SetDisabledColor(glm::vec4 color);

    void SetUseQuadStateColor(bool inUse);
    void SetUseTextStateColor(bool inUse);
    void SetHandleMouseInput(bool inHandle);

    void SetHoverHandler(ButtonHandlerFP newHandler);
    void SetPressedHandler(ButtonHandlerFP newHandler);
    void SetScriptHoverHandler(const char* tableName, const char* funcName);
    void SetScriptPressedHandler(const char* tableName, const char* funcName);

    void EnableRightClickPress(bool enable);
    bool IsRightClickPressEnabled();

    virtual void SetTextString(const std::string& newTextString);
    const std::string& GetTextString() const;

    Text* GetText();
    Quad* GetQuad();

    virtual void OnPressed();
    virtual void OnHover();

protected:

    Texture* mNormalTexture;
    Texture* mHoveredTexture;
    Texture* mPressedTexture;
    Texture* mDisabledTexture;

    glm::vec4 mNormalColor;
    glm::vec4 mHoveredColor;
    glm::vec4 mPressedColor;
    glm::vec4 mDisabledColor;

    ButtonState mState;
    float mStateColorChangeSpeed;

    bool mUseTextStateColor;
    bool mUseQuadStateColor;
    bool mHandleMouseInput;
    bool mAutoSizeText;
    bool mRightClickPress;
    glm::vec2 mTextPaddingRatio;

    ScriptableFP<ButtonHandlerFP> mHoveredHandler;
    ScriptableFP<ButtonHandlerFP> mPressedHandler;

    Quad* mQuad;
    Text* mText;
};