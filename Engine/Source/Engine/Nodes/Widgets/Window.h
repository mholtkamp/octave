#pragma once

#include "Nodes/Widgets/Widget.h"

class Text;
class Button;
class Canvas;
class ScrollContainer;
class Quad;

enum class TitleAlignment : uint8_t
{
    Left,
    Center,
    Right,
    Count
};

class OCTAVE_API Window : public Widget
{
public:

    DECLARE_NODE(Window, Widget);

    virtual void Create() override;
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;
    virtual void GatherProperties(std::vector<Property>& props) override;

    // Window ID (for WindowManager registration)
    void SetWindowId(const std::string& id);
    const std::string& GetWindowId() const;

    // Title
    void SetTitle(const std::string& title);
    const std::string& GetTitle() const;
    void SetTitleAlignment(TitleAlignment alignment);
    TitleAlignment GetTitleAlignment() const;
    void SetTitleFontSize(float size);
    float GetTitleFontSize() const;
    void SetTitleOffset(glm::vec2 offset);
    glm::vec2 GetTitleOffset() const;

    // Content Padding
    void SetContentPadding(float left, float top, float right, float bottom);
    void SetContentPaddingLeft(float left);
    void SetContentPaddingTop(float top);
    void SetContentPaddingRight(float right);
    void SetContentPaddingBottom(float bottom);
    float GetContentPaddingLeft() const;
    float GetContentPaddingTop() const;
    float GetContentPaddingRight() const;
    float GetContentPaddingBottom() const;

    // Visibility
    void Show();
    void Hide();
    void Close(); // Emits OnClose signal, hides window

    // Window properties
    void SetDraggable(bool draggable);
    bool IsDraggable() const;
    void SetResizable(bool resizable);
    bool IsResizable() const;
    void SetShowCloseButton(bool show);
    bool GetShowCloseButton() const;
    void SetTitleBarHeight(float height);
    float GetTitleBarHeight() const;
    void SetMinSize(glm::vec2 minSize);
    glm::vec2 GetMinSize() const;
    void SetMaxSize(glm::vec2 maxSize);
    glm::vec2 GetMaxSize() const;
    void SetResizeHandleSize(float size);
    float GetResizeHandleSize() const;

    // Content management - use this instead of AddChild for user content
    void SetContentWidget(Widget* widget);

    // Content access
    ScrollContainer* GetContentContainer();
    Widget* GetContentWidget(); // First non-transient child of content container

    // Colors
    void SetTitleBarColor(glm::vec4 color);
    glm::vec4 GetTitleBarColor() const;
    void SetBackgroundColor(glm::vec4 color);
    glm::vec4 GetBackgroundColor() const;

    // Internal widget access
    Canvas* GetTitleBar();
    Text* GetTitleText();
    Button* GetCloseButton();
    Quad* GetResizeHandle();
    Quad* GetBackground();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);
    void EnsureContentContainer();
    void RerouteChildrenToContent();
    void HandleDrag();
    void HandleResize();
    void UpdateLayout();

    // Properties
    std::string mWindowId;
    std::string mTitle = "Window";
    TitleAlignment mTitleAlignment = TitleAlignment::Center;
    float mTitleFontSize = 14.0f;
    glm::vec2 mTitleOffset = glm::vec2(0.0f,-8.0f);
    float mTitleBarHeight = 24.0f;
    float mContentPaddingLeft = 0.0f;
    float mContentPaddingTop = 0.0f;
    float mContentPaddingRight = 0.0f;
    float mContentPaddingBottom = 0.0f;
    bool mDraggable = true;
    bool mResizable = true;
    bool mShowCloseButton = true;
    float mResizeHandleSize = 12.0f;
    glm::vec2 mMinSize = glm::vec2(100.0f, 80.0f);
    glm::vec2 mMaxSize = glm::vec2(0.0f); // 0 = no max
    glm::vec4 mTitleBarColor = glm::vec4(0.2f, 0.2f, 0.3f, 1.0f);
    glm::vec4 mBackgroundColor = glm::vec4(0.15f, 0.15f, 0.2f, 0.95f);

    // Drag/Resize state
    bool mDragging = false;
    bool mResizing = false;
    glm::vec2 mDragOffset = glm::vec2(0.0f);
    glm::vec2 mResizeStartSize = glm::vec2(0.0f);
    glm::vec2 mResizeStartMouse = glm::vec2(0.0f);

    // Transient children
    Quad* mBackground = nullptr;
    Canvas* mTitleBar = nullptr;
    Text* mTitleText = nullptr;
    Button* mCloseButton = nullptr;
    ScrollContainer* mContentContainer = nullptr;
    Quad* mResizeHandle = nullptr;
};
