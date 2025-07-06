#pragma once

#include "Nodes/Widgets/Widget.h"
#include "ScriptFunc.h"

class Texture;
class Quad;
class Text;

enum class ButtonState : uint8_t
{
    Normal,
    Hovered,
    Pressed,
    Locked,

    Count
};

class Button : public Widget
{
public:

    DECLARE_NODE(Button, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;
    void UpdateAppearance();

    bool IsSelected() const;
    virtual void Activate();

    ButtonState GetState();
    virtual void SetState(ButtonState newState);

    void SetNormalTexture(Texture* texture);
    void SetHoveredTexture(Texture* texture);
    void SetPressedTexture(Texture* texture);
    void SetLockedTexture(Texture* texture);

    Texture* GetNormalTexture();
    Texture* GetHoveredTexture();
    Texture* GetPressedTexture();
    Texture* GetLockedTexture();

    void SetNormalColor(glm::vec4 color);
    void SetHoveredColor(glm::vec4 color);
    void SetPressedColor(glm::vec4 color);
    void SetLockedColor(glm::vec4 color);

    glm::vec4 GetNormalColor();
    glm::vec4 GetHoveredColor();
    glm::vec4 GetPressedColor();
    glm::vec4 GetLockedColor();

    void SetUseQuadStateColor(bool inUse);
    void SetUseTextStateColor(bool inUse);
    static void SetHandleMouse(bool inHandle);
    static void SetHandleGamepad(bool inHandle);
    static void SetHandleKeyboard(bool inHandle);

    void EnableRightClickPress(bool enable);
    bool IsRightClickPressEnabled();

    virtual void SetTextString(const std::string& newTextString);
    const std::string& GetTextString() const;

    void SetNavUp(Node* up);
    void SetNavDown(Node* down);
    void SetNavLeft(Node* left);
    void SetNavRight(Node* right);

    Node* GetNavUp();
    Node* GetNavDown();
    Node* GetNavLeft();
    Node* GetNavRight();

    Text* GetText();
    Quad* GetQuad();

    static Button* GetSelectedButton();
    static void SetSelectedButton(Button* button);

    static void StaticUpdate();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    TextureRef mNormalTexture;
    TextureRef mHoveredTexture;
    TextureRef mPressedTexture;
    TextureRef mLockedTexture;

    glm::vec4 mNormalColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    glm::vec4 mHoveredColor = { 0.6f, 0.6f, 0.6f, 1.0f };
    glm::vec4 mPressedColor = { 0.4, 0.4f, 0.4f, 1.0f };
    glm::vec4 mLockedColor = { 0.2f, 0.2f, 0.2f, 1.0f };

    ButtonState mState = ButtonState::Normal;

    bool mUseTextStateColor = false;
    bool mUseQuadStateColor = true;
    bool mAutoSizeText = false;
    bool mRightClickPress = false;
    glm::vec2 mTextPaddingRatio = { 0.035f, 0.05f };

    WeakPtr<Button> mNavUp;
    WeakPtr<Button> mNavDown;
    WeakPtr<Button> mNavLeft;
    WeakPtr<Button> mNavRight;

    Quad* mQuad = nullptr;
    Text* mText = nullptr;

    static WeakPtr<Button> sSelectedButton;
    static bool sSelButtonChangedThisFrame;

    static bool sHandleMouseInput;
    static bool sHandleGamepadInput;
    static bool sHandleKeyboardInput;
};