#include "Nodes/Widgets/Window.h"
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/ScrollContainer.h"
#include "WindowManager.h"
#include "InputDevices.h"
#include "Assets/Texture.h"
#include "Log.h"

FORCE_LINK_DEF(Window);
DEFINE_NODE(Window, Widget);

static const char* sTitleAlignmentStrings[] = {
    "Left",
    "Center",
    "Right"
};

bool Window::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop->mOwner != nullptr);
    Window* window = static_cast<Window*>(prop->mOwner);

    bool success = false;

    if (prop->mName == "Window Id")
    {
        window->SetWindowId(*static_cast<const std::string*>(newValue));
        success = true;
    }
    else if (prop->mName == "Title")
    {
        window->SetTitle(*static_cast<const std::string*>(newValue));
        success = true;
    }
    else if (prop->mName == "Title Alignment")
    {
        window->SetTitleAlignment((TitleAlignment)*static_cast<const uint8_t*>(newValue));
        success = true;
    }
    else if (prop->mName == "Title Font Size")
    {
        window->SetTitleFontSize(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Title Offset")
    {
        window->SetTitleOffset(*static_cast<const glm::vec2*>(newValue));
        success = true;
    }
    else if (prop->mName == "Content Padding Left")
    {
        window->SetContentPaddingLeft(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Content Padding Top")
    {
        window->SetContentPaddingTop(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Content Padding Right")
    {
        window->SetContentPaddingRight(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Content Padding Bottom")
    {
        window->SetContentPaddingBottom(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Title Bar Height")
    {
        window->SetTitleBarHeight(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Draggable")
    {
        window->SetDraggable(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Resizable")
    {
        window->SetResizable(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Show Close Button")
    {
        window->SetShowCloseButton(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Show Title Bar")
    {
        window->SetShowTitleBar(*static_cast<const bool*>(newValue));
        success = true;
    }
    else if (prop->mName == "Start Hidden")
    {
        window->SetStartHidden(*static_cast<const bool*>(newValue));
        success = true;
    }
#if EDITOR
    else if (prop->mName == "Toggle Visibility")
    {
        window->SetVisible(!window->IsVisible());
        success = true;
    }
#endif
    else if (prop->mName == "Min Size")
    {
        window->SetMinSize(*static_cast<const glm::vec2*>(newValue));
        success = true;
    }
    else if (prop->mName == "Max Size")
    {
        window->SetMaxSize(*static_cast<const glm::vec2*>(newValue));
        success = true;
    }
    else if (prop->mName == "Resize Handle Size")
    {
        window->SetResizeHandleSize(*static_cast<const float*>(newValue));
        success = true;
    }
    else if (prop->mName == "Title Bar Color")
    {
        window->SetTitleBarColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Background Color")
    {
        window->SetBackgroundColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Background Texture")
    {
        window->SetBackgroundTexture(*(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Title Bar Texture")
    {
        window->SetTitleBarTexture(*(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Close Button Texture")
    {
        window->SetCloseButtonTexture(*(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Close Button Normal Color")
    {
        window->SetCloseButtonNormalColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Close Button Hovered Color")
    {
        window->SetCloseButtonHoveredColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }
    else if (prop->mName == "Close Button Pressed Color")
    {
        window->SetCloseButtonPressedColor(*static_cast<const glm::vec4*>(newValue));
        success = true;
    }

    return success;
}

void Window::Create()
{
    Widget::Create();
    SetName("Window");

    // Enable scissor clipping for content overflow
    mUseScissor = true;

    // Set default size
    SetDimensions(300.0f, 200.0f);

    // Create background quad
    mBackground = CreateChild<Quad>("Background");
    mBackground->SetTransient(true);
    mBackground->SetAnchorMode(AnchorMode::FullStretch);
    mBackground->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    mBackground->SetColor(mBackgroundColor);
#if EDITOR
    mBackground->mHiddenInTree = true;
#endif

    // Create title bar canvas
    mTitleBar = CreateChild<Canvas>("TitleBar");
    mTitleBar->SetTransient(true);
    mTitleBar->SetAnchorMode(AnchorMode::TopStretch);
    mTitleBar->SetHeight(mTitleBarHeight);
    mTitleBar->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
#if EDITOR
    mTitleBar->mHiddenInTree = true;
#endif

    // Create title bar background quad (child of title bar)
    Quad* titleBg = mTitleBar->CreateChild<Quad>("TitleBg");
    titleBg->SetTransient(true);
    titleBg->SetAnchorMode(AnchorMode::FullStretch);
    titleBg->SetMargins(0.0f, 0.0f, 0.0f, 0.0f);
    titleBg->SetColor(mTitleBarColor);
#if EDITOR
    titleBg->mHiddenInTree = true;
#endif

    // Create title text
    mTitleText = mTitleBar->CreateChild<Text>("TitleText");
    mTitleText->SetTransient(true);
    mTitleText->SetText(mTitle);
    mTitleText->SetTextSize(mTitleFontSize);
    mTitleText->SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    // Alignment is set in UpdateLayout
#if EDITOR
    mTitleText->mHiddenInTree = true;
#endif

    // Create close button
    mCloseButton = mTitleBar->CreateChild<Button>("CloseButton");
    mCloseButton->SetTransient(true);
    mCloseButton->SetAnchorMode(AnchorMode::TopRight);
    float btnSize = mTitleBarHeight - 4.0f;
    mCloseButton->SetDimensions(btnSize, btnSize);
    mCloseButton->SetPosition(-btnSize - 2.0f, 2.0f);
    mCloseButton->SetTextString("X");
    mCloseButton->SetNormalColor(mCloseButtonNormalColor);
    mCloseButton->SetHoveredColor(mCloseButtonHoveredColor);
    mCloseButton->SetPressedColor(mCloseButtonPressedColor);
    mCloseButton->SetVisible(mShowCloseButton);
#if EDITOR
    mCloseButton->mHiddenInTree = true;
#endif

    // ContentContainer is created lazily in EnsureContentContainer()
    // This avoids conflicts during Clone operations
    mContentContainer = nullptr;

    // Force initial layout
    MarkDirty();

    // Create resize handle
    mResizeHandle = CreateChild<Quad>("ResizeHandle");
    mResizeHandle->SetTransient(true);
    mResizeHandle->SetAnchorMode(AnchorMode::BottomRight);
    mResizeHandle->SetDimensions(mResizeHandleSize, mResizeHandleSize);
    mResizeHandle->SetPosition(0.0f, 0.0f);
    mResizeHandle->SetColor(glm::vec4(0.4f, 0.4f, 0.4f, 0.8f));
    mResizeHandle->SetVisible(mResizable);
#if EDITOR
    mResizeHandle->mHiddenInTree = true;
#endif

    MarkDirty();
}

void Window::Start()
{
    Widget::Start();

    // Register with WindowManager
    if (!mWindowId.empty() && WindowManager::Get() != nullptr)
    {
        WindowManager::Get()->RegisterWindow(mWindowId, this);
    }

    // Set initial visibility based on Start Hidden property
    if (mStartHidden)
    {
        Hide();
    }
    else
    {
        Show();
    }
}

void Window::Stop()
{
    // Unregister from WindowManager
    if (WindowManager::Get() != nullptr)
    {
        WindowManager::Get()->UnregisterWindow(this);
    }

    Widget::Stop();
}

void Window::GatherProperties(std::vector<Property>& props)
{
    Widget::GatherProperties(props);

    SCOPED_CATEGORY("Window");

    props.push_back(Property(DatumType::String, "Window Id", this, &mWindowId, 1, HandlePropChange));
    props.push_back(Property(DatumType::String, "Title", this, &mTitle, 1, HandlePropChange));
    props.push_back(Property(DatumType::Byte, "Title Alignment", this, &mTitleAlignment, 1,
        HandlePropChange, NULL_DATUM, int32_t(TitleAlignment::Count), sTitleAlignmentStrings));
    props.push_back(Property(DatumType::Float, "Title Font Size", this, &mTitleFontSize, 1, HandlePropChange));
    props.push_back(Property(DatumType::Vector2D, "Title Offset", this, &mTitleOffset, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Title Bar Height", this, &mTitleBarHeight, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Content Padding Left", this, &mContentPaddingLeft, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Content Padding Top", this, &mContentPaddingTop, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Content Padding Right", this, &mContentPaddingRight, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Content Padding Bottom", this, &mContentPaddingBottom, 1, HandlePropChange));
    props.push_back(Property(DatumType::Bool, "Draggable", this, &mDraggable, 1, HandlePropChange));
    props.push_back(Property(DatumType::Bool, "Resizable", this, &mResizable, 1, HandlePropChange));
    props.push_back(Property(DatumType::Bool, "Show Close Button", this, &mShowCloseButton, 1, HandlePropChange));
    props.push_back(Property(DatumType::Bool, "Show Title Bar", this, &mShowTitleBar, 1, HandlePropChange));
    props.push_back(Property(DatumType::Bool, "Start Hidden", this, &mStartHidden, 1, HandlePropChange));
#if EDITOR
    static bool sFakeToggle = false;
    props.push_back(Property(DatumType::Bool, "Toggle Visibility", this, &sFakeToggle, 1, HandlePropChange));
#endif
    props.push_back(Property(DatumType::Vector2D, "Min Size", this, &mMinSize, 1, HandlePropChange));
    props.push_back(Property(DatumType::Vector2D, "Max Size", this, &mMaxSize, 1, HandlePropChange));
    props.push_back(Property(DatumType::Float, "Resize Handle Size", this, &mResizeHandleSize, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Title Bar Color", this, &mTitleBarColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Background Color", this, &mBackgroundColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Asset, "Background Texture", this, &mBackgroundTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    props.push_back(Property(DatumType::Asset, "Title Bar Texture", this, &mTitleBarTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    props.push_back(Property(DatumType::Asset, "Close Button Texture", this, &mCloseButtonTexture, 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    props.push_back(Property(DatumType::Color, "Close Button Normal Color", this, &mCloseButtonNormalColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Close Button Hovered Color", this, &mCloseButtonHoveredColor, 1, HandlePropChange));
    props.push_back(Property(DatumType::Color, "Close Button Pressed Color", this, &mCloseButtonPressedColor, 1, HandlePropChange));
}

void Window::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);

    // Reroute any non-transient children to the content container
    RerouteChildrenToContent();

    // Check for close button activation
    if (mCloseButton != nullptr && mCloseButton->IsVisible())
    {
        if (IsPointerJustUp(0) && mCloseButton->GetState() == ButtonState::Pressed && mCloseButton->ContainsMouse())
        {
            Close();
            return;
        }
    }

    HandleDrag();
    HandleResize();
}

void Window::RerouteChildrenToContent()
{
    if (mContentContainer == nullptr)
    {
        return;
    }

    // Move any non-transient direct children to the content container
    std::vector<Node*> childrenToMove;
    for (uint32_t i = 0; i < GetNumChildren(); ++i)
    {
        Node* child = GetChild(i);
        if (child != nullptr &&
            !child->IsTransient() &&
            child != mContentContainer)
        {
            childrenToMove.push_back(child);
        }
    }

    for (Node* child : childrenToMove)
    {
        child->Attach(mContentContainer);
    }
}

void Window::HandleDrag()
{
    if (!mDraggable || mTitleBar == nullptr)
    {
        return;
    }

    int32_t mouseX, mouseY;
    GetMousePosition(mouseX, mouseY);
    glm::vec2 mousePos((float)mouseX, (float)mouseY);

    if (IsPointerJustDown(0) && mTitleBar->ContainsMouse() && !mResizing)
    {
        // Don't start drag if clicking on close button
        if (mCloseButton != nullptr && mCloseButton->IsVisible() && mCloseButton->ContainsMouse())
        {
            return;
        }

        mDragging = true;
        mDragOffset = GetPosition() - mousePos;
        EmitSignal("DragStart", { this });
        CallFunction("OnDragStart", { this });
    }

    if (mDragging)
    {
        if (IsPointerDown(0))
        {
            SetPosition(mousePos + mDragOffset);
        }
        else
        {
            mDragging = false;
            EmitSignal("DragEnd", { this });
            CallFunction("OnDragEnd", { this });
        }
    }
}

void Window::HandleResize()
{
    if (!mResizable || mResizeHandle == nullptr)
    {
        return;
    }

    int32_t mouseX, mouseY;
    GetMousePosition(mouseX, mouseY);
    glm::vec2 mousePos((float)mouseX, (float)mouseY);

    if (IsPointerJustDown(0) && mResizeHandle->ContainsMouse() && !mDragging)
    {
        mResizing = true;
        mResizeStartSize = GetDimensions();
        mResizeStartMouse = mousePos;
        EmitSignal("ResizeStart", { this });
        CallFunction("OnResizeStart", { this });
    }

    if (mResizing)
    {
        if (IsPointerDown(0))
        {
            glm::vec2 delta = mousePos - mResizeStartMouse;
            glm::vec2 newSize = mResizeStartSize + delta;

            // Clamp to min size
            newSize = glm::max(newSize, mMinSize);

            // Clamp to max size if set
            if (mMaxSize.x > 0.0f)
            {
                newSize.x = glm::min(newSize.x, mMaxSize.x);
            }
            if (mMaxSize.y > 0.0f)
            {
                newSize.y = glm::min(newSize.y, mMaxSize.y);
            }

            SetDimensions(newSize);
            MarkDirty();

            EmitSignal("Resized", { this });
            CallFunction("OnResized", { this });
        }
        else
        {
            mResizing = false;
            EmitSignal("ResizeEnd", { this });
            CallFunction("OnResizeEnd", { this });
        }
    }
}

void Window::EnsureContentContainer()
{
    if (mContentContainer != nullptr)
    {
        return;
    }

    // Find existing or create new content container
    mContentContainer = static_cast<ScrollContainer*>(FindChild("ContentContainer", false));
    if (mContentContainer == nullptr)
    {
        mContentContainer = CreateChild<ScrollContainer>("ContentContainer");
        mContentContainer->SetScrollSizeMode(ScrollSizeMode::FitWidth);
    }
}

void Window::PreRender()
{
    Widget::PreRender();

    // Ensure content container exists
    EnsureContentContainer();

    // Reroute any non-transient direct children to content container
    RerouteChildrenToContent();

    // Update layout - always run to ensure proper positioning
    UpdateLayout();
}

void Window::UpdateLayout()
{
    float effectiveTitleBarHeight = mShowTitleBar ? mTitleBarHeight : 0.0f;

    if (mTitleBar != nullptr)
    {
        mTitleBar->SetHeight(mTitleBarHeight);
        mTitleBar->SetVisible(mShowTitleBar);
    }

    if (mContentContainer != nullptr)
    {
        float contentX = mContentPaddingLeft;
        float contentY = effectiveTitleBarHeight + mContentPaddingTop;
        float contentW = glm::max(1.0f, GetWidth() - mContentPaddingLeft - mContentPaddingRight);
        float contentH = glm::max(1.0f, GetHeight() - effectiveTitleBarHeight - mContentPaddingTop - mContentPaddingBottom);

        // Ensure anchor mode is TopLeft for explicit positioning
        mContentContainer->SetAnchorMode(AnchorMode::TopLeft);
        mContentContainer->SetPosition(contentX, contentY);
        mContentContainer->SetDimensions(contentW, contentH);
        mContentContainer->MarkDirty();
    }

    if (mResizeHandle != nullptr)
    {
        mResizeHandle->SetDimensions(mResizeHandleSize, mResizeHandleSize);
        mResizeHandle->SetVisible(mResizable);
    }

    if (mCloseButton != nullptr)
    {
        mCloseButton->SetVisible(mShowCloseButton);
        mCloseButton->SetDimensions(mTitleBarHeight - 4.0f, mTitleBarHeight - 4.0f);
        mCloseButton->SetNormalColor(mCloseButtonNormalColor);
        mCloseButton->SetHoveredColor(mCloseButtonHoveredColor);
        mCloseButton->SetPressedColor(mCloseButtonPressedColor);
        if (mCloseButtonTexture.Get<Texture>() != nullptr)
        {
            mCloseButton->SetNormalTexture(mCloseButtonTexture.Get<Texture>());
            mCloseButton->SetTextString("");
        }
    }

    if (mBackground != nullptr)
    {
        mBackground->SetColor(mBackgroundColor);
        mBackground->SetTexture(mBackgroundTexture.Get<Texture>());
    }

    // Update title text alignment and size
    if (mTitleText != nullptr)
    {
        mTitleText->SetTextSize(mTitleFontSize);

        float closeButtonWidth = (mShowCloseButton && mCloseButton != nullptr) ? (mTitleBarHeight) : 0.0f;

        switch (mTitleAlignment)
        {
            case TitleAlignment::Left:
                mTitleText->SetAnchorMode(AnchorMode::MidLeft);
                mTitleText->SetPosition(8.0f + mTitleOffset.x, mTitleOffset.y);
                break;
            case TitleAlignment::Center:
                mTitleText->SetAnchorMode(AnchorMode::Mid);
                mTitleText->SetPosition(-closeButtonWidth / 2.0f + mTitleOffset.x, mTitleOffset.y);
                break;
            case TitleAlignment::Right:
                mTitleText->SetAnchorMode(AnchorMode::MidRight);
                mTitleText->SetPosition(-closeButtonWidth - 8.0f + mTitleOffset.x, mTitleOffset.y);
                break;
            default:
                break;
        }
    }

    // Update title bar background color and texture
    if (mTitleBar != nullptr && mTitleBar->GetNumChildren() > 0)
    {
        Quad* titleBg = static_cast<Quad*>(mTitleBar->GetChild(0));
        if (titleBg != nullptr)
        {
            titleBg->SetColor(mTitleBarColor);
            titleBg->SetTexture(mTitleBarTexture.Get<Texture>());
        }
    }
}

void Window::SetContentWidget(Widget* widget)
{
    if (mContentContainer != nullptr && widget != nullptr)
    {
        mContentContainer->AddChild(widget);
    }
}

// Setters and getters

void Window::SetWindowId(const std::string& id)
{
    if (mWindowId != id)
    {
        // Unregister old ID
        if (!mWindowId.empty() && WindowManager::Get() != nullptr)
        {
            WindowManager::Get()->UnregisterWindow(mWindowId);
        }

        mWindowId = id;

        // Register new ID if we're started
        if (!mWindowId.empty() && HasStarted() && WindowManager::Get() != nullptr)
        {
            WindowManager::Get()->RegisterWindow(mWindowId, this);
        }
    }
}

const std::string& Window::GetWindowId() const
{
    return mWindowId;
}

void Window::SetTitle(const std::string& title)
{
    mTitle = title;
    if (mTitleText != nullptr)
    {
        mTitleText->SetText(mTitle);
    }
    MarkDirty();
}

const std::string& Window::GetTitle() const
{
    return mTitle;
}

void Window::SetTitleAlignment(TitleAlignment alignment)
{
    mTitleAlignment = alignment;
    MarkDirty();
}

TitleAlignment Window::GetTitleAlignment() const
{
    return mTitleAlignment;
}

void Window::SetTitleFontSize(float size)
{
    mTitleFontSize = size;
    if (mTitleText != nullptr)
    {
        mTitleText->SetTextSize(mTitleFontSize);
    }
    MarkDirty();
}

float Window::GetTitleFontSize() const
{
    return mTitleFontSize;
}

void Window::SetTitleOffset(glm::vec2 offset)
{
    mTitleOffset = offset;
    MarkDirty();
}

glm::vec2 Window::GetTitleOffset() const
{
    return mTitleOffset;
}

void Window::SetContentPadding(float left, float top, float right, float bottom)
{
    mContentPaddingLeft = left;
    mContentPaddingTop = top;
    mContentPaddingRight = right;
    mContentPaddingBottom = bottom;
    MarkDirty();
}

void Window::SetContentPaddingLeft(float left)
{
    mContentPaddingLeft = left;
    MarkDirty();
}

void Window::SetContentPaddingTop(float top)
{
    mContentPaddingTop = top;
    MarkDirty();
}

void Window::SetContentPaddingRight(float right)
{
    mContentPaddingRight = right;
    MarkDirty();
}

void Window::SetContentPaddingBottom(float bottom)
{
    mContentPaddingBottom = bottom;
    MarkDirty();
}

float Window::GetContentPaddingLeft() const
{
    return mContentPaddingLeft;
}

float Window::GetContentPaddingTop() const
{
    return mContentPaddingTop;
}

float Window::GetContentPaddingRight() const
{
    return mContentPaddingRight;
}

float Window::GetContentPaddingBottom() const
{
    return mContentPaddingBottom;
}

void Window::Show()
{
    SetVisible(true);
    EmitSignal("Show", { this });
    CallFunction("OnShow", { this });
}

void Window::Hide()
{
    SetVisible(false);
    EmitSignal("Hide", { this });
    CallFunction("OnHide", { this });
}

void Window::Close()
{
    EmitSignal("Close", { this });
    CallFunction("OnClose", { this });
    Hide();
}

void Window::SetStartHidden(bool hidden)
{
    mStartHidden = hidden;
}

bool Window::GetStartHidden() const
{
    return mStartHidden;
}

void Window::SetDraggable(bool draggable)
{
    mDraggable = draggable;
}

bool Window::IsDraggable() const
{
    return mDraggable;
}

void Window::SetResizable(bool resizable)
{
    mResizable = resizable;
    if (mResizeHandle != nullptr)
    {
        mResizeHandle->SetVisible(mResizable);
    }
    MarkDirty();
}

bool Window::IsResizable() const
{
    return mResizable;
}

void Window::SetShowCloseButton(bool show)
{
    mShowCloseButton = show;
    if (mCloseButton != nullptr)
    {
        mCloseButton->SetVisible(mShowCloseButton);
    }
    MarkDirty();
}

bool Window::GetShowCloseButton() const
{
    return mShowCloseButton;
}

void Window::SetShowTitleBar(bool show)
{
    mShowTitleBar = show;
    if (mTitleBar != nullptr)
    {
        mTitleBar->SetVisible(mShowTitleBar);
    }
    MarkDirty();
}

bool Window::GetShowTitleBar() const
{
    return mShowTitleBar;
}

void Window::SetTitleBarHeight(float height)
{
    mTitleBarHeight = height;
    MarkDirty();
}

float Window::GetTitleBarHeight() const
{
    return mTitleBarHeight;
}

void Window::SetMinSize(glm::vec2 minSize)
{
    mMinSize = minSize;
}

glm::vec2 Window::GetMinSize() const
{
    return mMinSize;
}

void Window::SetMaxSize(glm::vec2 maxSize)
{
    mMaxSize = maxSize;
}

glm::vec2 Window::GetMaxSize() const
{
    return mMaxSize;
}

void Window::SetResizeHandleSize(float size)
{
    mResizeHandleSize = size;
    MarkDirty();
}

float Window::GetResizeHandleSize() const
{
    return mResizeHandleSize;
}

ScrollContainer* Window::GetContentContainer()
{
    return mContentContainer;
}

Widget* Window::GetContentWidget()
{
    if (mContentContainer != nullptr)
    {
        return mContentContainer->GetContentWidget();
    }
    return nullptr;
}

void Window::SetTitleBarColor(glm::vec4 color)
{
    mTitleBarColor = color;
    MarkDirty();
}

glm::vec4 Window::GetTitleBarColor() const
{
    return mTitleBarColor;
}

void Window::SetBackgroundColor(glm::vec4 color)
{
    mBackgroundColor = color;
    if (mBackground != nullptr)
    {
        mBackground->SetColor(mBackgroundColor);
    }
    MarkDirty();
}

glm::vec4 Window::GetBackgroundColor() const
{
    return mBackgroundColor;
}

void Window::SetBackgroundTexture(Texture* texture)
{
    mBackgroundTexture = texture;
    MarkDirty();
}

Texture* Window::GetBackgroundTexture()
{
    return mBackgroundTexture.Get<Texture>();
}

void Window::SetTitleBarTexture(Texture* texture)
{
    mTitleBarTexture = texture;
    MarkDirty();
}

Texture* Window::GetTitleBarTexture()
{
    return mTitleBarTexture.Get<Texture>();
}

void Window::SetCloseButtonTexture(Texture* texture)
{
    mCloseButtonTexture = texture;
    MarkDirty();
}

Texture* Window::GetCloseButtonTexture()
{
    return mCloseButtonTexture.Get<Texture>();
}

void Window::SetCloseButtonNormalColor(glm::vec4 color)
{
    mCloseButtonNormalColor = color;
    MarkDirty();
}

glm::vec4 Window::GetCloseButtonNormalColor() const
{
    return mCloseButtonNormalColor;
}

void Window::SetCloseButtonHoveredColor(glm::vec4 color)
{
    mCloseButtonHoveredColor = color;
    MarkDirty();
}

glm::vec4 Window::GetCloseButtonHoveredColor() const
{
    return mCloseButtonHoveredColor;
}

void Window::SetCloseButtonPressedColor(glm::vec4 color)
{
    mCloseButtonPressedColor = color;
    MarkDirty();
}

glm::vec4 Window::GetCloseButtonPressedColor() const
{
    return mCloseButtonPressedColor;
}

Canvas* Window::GetTitleBar()
{
    return mTitleBar;
}

Text* Window::GetTitleText()
{
    return mTitleText;
}

Button* Window::GetCloseButton()
{
    return mCloseButton;
}

Quad* Window::GetResizeHandle()
{
    return mResizeHandle;
}

Quad* Window::GetBackground()
{
    return mBackground;
}
