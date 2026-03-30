#pragma once

#include "Nodes/Widgets/Window.h"
#include "AssetRef.h"
#include "Datum.h"

class Button;
class Canvas;
class Node;
class Quad;
class Texture;

enum class ButtonBarAlignment : uint8_t
{
    Left,
    Center,
    Right,
    Spread,
    Count
};

class OCTAVE_API DialogWindow : public Window
{
public:

    DECLARE_NODE(DialogWindow, Window);

    virtual void Create() override;
    virtual void Start() override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;
    virtual void GatherProperties(std::vector<Property>& props) override;

#if EDITOR
    virtual bool DrawCustomProperty(Property& prop) override;
#endif

    // Confirm Button
    void SetConfirmText(const std::string& text);
    const std::string& GetConfirmText() const;
    void SetConfirmNormalColor(glm::vec4 color);
    glm::vec4 GetConfirmNormalColor() const;
    void SetConfirmHoveredColor(glm::vec4 color);
    glm::vec4 GetConfirmHoveredColor() const;
    void SetConfirmPressedColor(glm::vec4 color);
    glm::vec4 GetConfirmPressedColor() const;
    void SetConfirmTexture(Texture* texture);
    Texture* GetConfirmTexture();
    void SetShowConfirmButton(bool show);
    bool GetShowConfirmButton() const;

    // Reject Button
    void SetRejectText(const std::string& text);
    const std::string& GetRejectText() const;
    void SetRejectNormalColor(glm::vec4 color);
    glm::vec4 GetRejectNormalColor() const;
    void SetRejectHoveredColor(glm::vec4 color);
    glm::vec4 GetRejectHoveredColor() const;
    void SetRejectPressedColor(glm::vec4 color);
    glm::vec4 GetRejectPressedColor() const;
    void SetRejectTexture(Texture* texture);
    Texture* GetRejectTexture();
    void SetShowRejectButton(bool show);
    bool GetShowRejectButton() const;

    // Button Bar Layout
    void SetButtonBarHeight(float height);
    float GetButtonBarHeight() const;
    void SetButtonSpacing(float spacing);
    float GetButtonSpacing() const;
    void SetButtonBarAlignment(ButtonBarAlignment alignment);
    ButtonBarAlignment GetButtonBarAlignment() const;
    void SetButtonBarColor(glm::vec4 color);
    glm::vec4 GetButtonBarColor() const;
    void SetButtonWidth(float width);
    float GetButtonWidth() const;
    void SetButtonBarPadding(float padding);
    float GetButtonBarPadding() const;

    // Dialog Actions
    void Confirm();
    void Reject();
    void RefreshButtonBar();

    // Internal widget access
    Canvas* GetButtonBar();
    Button* GetConfirmButton();
    Button* GetRejectButton();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);
    static void OnConfirmButtonActivated(Node* listener, const std::vector<Datum>& args);
    static void OnRejectButtonActivated(Node* listener, const std::vector<Datum>& args);
    void EnsureButtonBar();
    void UpdateButtonBar();

    // Confirm Button Properties
    std::string mConfirmText = "Confirm";
    glm::vec4 mConfirmNormalColor = glm::vec4(0.2f, 0.5f, 0.2f, 1.0f);
    glm::vec4 mConfirmHoveredColor = glm::vec4(0.3f, 0.6f, 0.3f, 1.0f);
    glm::vec4 mConfirmPressedColor = glm::vec4(0.15f, 0.4f, 0.15f, 1.0f);
    TextureRef mConfirmTexture;
    bool mShowConfirmButton = true;

    // Reject Button Properties
    std::string mRejectText = "Cancel";
    glm::vec4 mRejectNormalColor = glm::vec4(0.5f, 0.2f, 0.2f, 1.0f);
    glm::vec4 mRejectHoveredColor = glm::vec4(0.6f, 0.3f, 0.3f, 1.0f);
    glm::vec4 mRejectPressedColor = glm::vec4(0.4f, 0.15f, 0.15f, 1.0f);
    TextureRef mRejectTexture;
    bool mShowRejectButton = true;

    // Button Bar Layout Properties
    float mButtonBarHeight = 40.0f;
    float mButtonSpacing = 10.0f;
    float mButtonWidth = 80.0f;
    float mButtonBarPadding = 8.0f;
    ButtonBarAlignment mButtonBarAlignment = ButtonBarAlignment::Right;
    glm::vec4 mButtonBarColor = glm::vec4(0.18f, 0.18f, 0.25f, 1.0f);

    // Transient children
    Canvas* mButtonBar = nullptr;
    Quad* mButtonBarBackground = nullptr;
    Button* mConfirmButton = nullptr;
    Button* mRejectButton = nullptr;
    bool mButtonSignalsConnected = false;
};
