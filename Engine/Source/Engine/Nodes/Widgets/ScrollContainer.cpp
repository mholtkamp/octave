
#include "Nodes/Widgets/ScrollContainer.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Button.h"
#include "Assets/Texture.h"
#include "InputDevices.h"
#include "Engine.h"
#include "Renderer.h"

FORCE_LINK_DEF(ScrollContainer);
DEFINE_NODE(ScrollContainer, Widget);

static const char* sScrollSizeModeStrings[] = {
    "Fit Width",
    "Fit Height",
    "Fit Both",
    "None"
};

static const char* sScrollbarModeStrings[] = {
    "Hidden",
    "Auto",
    "Always Visible"
};

bool ScrollContainer::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    ScrollContainer* scrollContainer = static_cast<ScrollContainer*>(prop->mOwner);
    bool success = false;

    scrollContainer->MarkDirty();

    return success;
}

void ScrollContainer::Create()
{
    Super::Create();

    SetName("ScrollContainer");
    mUseScissor = true;

    // Create scrollbar widgets (rendered on top of content)
    mVScrollbarTrack = CreateChild<Quad>("VScrollbarTrack");
    mVScrollbarGrabber = CreateChild<Quad>("VScrollbarGrabber");
    mHScrollbarTrack = CreateChild<Quad>("HScrollbarTrack");
    mHScrollbarGrabber = CreateChild<Quad>("HScrollbarGrabber");

    // Create scroll buttons
    mUpButton = CreateChild<Button>("UpButton");
    mDownButton = CreateChild<Button>("DownButton");
    mLeftButton = CreateChild<Button>("LeftButton");
    mRightButton = CreateChild<Button>("RightButton");

    // Mark all as transient so they don't serialize
    mVScrollbarTrack->SetTransient(true);
    mVScrollbarGrabber->SetTransient(true);
    mHScrollbarTrack->SetTransient(true);
    mHScrollbarGrabber->SetTransient(true);
    mUpButton->SetTransient(true);
    mDownButton->SetTransient(true);
    mLeftButton->SetTransient(true);
    mRightButton->SetTransient(true);

#if EDITOR
    mVScrollbarTrack->mHiddenInTree = true;
    mVScrollbarGrabber->mHiddenInTree = true;
    mHScrollbarTrack->mHiddenInTree = true;
    mHScrollbarGrabber->mHiddenInTree = true;
    mUpButton->mHiddenInTree = true;
    mDownButton->mHiddenInTree = true;
    mLeftButton->mHiddenInTree = true;
    mRightButton->mHiddenInTree = true;
#endif

    // Configure scrollbar anchor modes
    mVScrollbarTrack->SetAnchorMode(AnchorMode::TopLeft);
    mVScrollbarGrabber->SetAnchorMode(AnchorMode::TopLeft);
    mHScrollbarTrack->SetAnchorMode(AnchorMode::TopLeft);
    mHScrollbarGrabber->SetAnchorMode(AnchorMode::TopLeft);

    // Configure button anchor modes
    mUpButton->SetAnchorMode(AnchorMode::TopLeft);
    mDownButton->SetAnchorMode(AnchorMode::TopLeft);
    mLeftButton->SetAnchorMode(AnchorMode::TopLeft);
    mRightButton->SetAnchorMode(AnchorMode::TopLeft);

    // Set button text
    mUpButton->SetTextString("^");
    mDownButton->SetTextString("v");
    mLeftButton->SetTextString("<");
    mRightButton->SetTextString(">");

    // Hide buttons by default
    mUpButton->SetVisible(false);
    mDownButton->SetVisible(false);
    mLeftButton->SetVisible(false);
    mRightButton->SetVisible(false);

    // Default dimensions
    SetDimensions(200, 150);
}

void ScrollContainer::GatherProperties(std::vector<Property>& props)
{
    Super::GatherProperties(props);

    {
        SCOPED_CATEGORY("ScrollContainer");

        props.push_back(Property(DatumType::Byte, "Size Mode", this, &mScrollSizeMode, 1,
            HandlePropChange, NULL_DATUM, int32_t(ScrollSizeMode::Count), sScrollSizeModeStrings));
        props.push_back(Property(DatumType::Vector2D, "Scroll Offset", this, &mScrollOffset, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Float, "Scroll Speed", this, &mScrollSpeed, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Momentum Enabled", this, &mMomentumEnabled, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Float, "Momentum Friction", this, &mMomentumFriction, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Bool, "Child Input Priority", this, &mChildInputPriority, 1,
            HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Scrollbars");

        props.push_back(Property(DatumType::Byte, "Horizontal Scrollbar", this, &mHScrollbarMode, 1,
            HandlePropChange, NULL_DATUM, int32_t(ScrollbarMode::Count), sScrollbarModeStrings));
        props.push_back(Property(DatumType::Byte, "Vertical Scrollbar", this, &mVScrollbarMode, 1,
            HandlePropChange, NULL_DATUM, int32_t(ScrollbarMode::Count), sScrollbarModeStrings));
        props.push_back(Property(DatumType::Float, "Scrollbar Width", this, &mScrollbarWidth, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Asset, "Scrollbar Texture", this, &mScrollbarTexture, 1,
            HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Scrollbar Color", this, &mScrollbarColor, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Color, "Scrollbar Hovered Color", this, &mScrollbarHoveredColor, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Asset, "Track Texture", this, &mTrackTexture, 1,
            HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Track Color", this, &mScrollbarTrackColor, 1,
            HandlePropChange));
    }

    {
        SCOPED_CATEGORY("Scroll Buttons");

        props.push_back(Property(DatumType::Bool, "Show Scroll Buttons", this, &mShowScrollButtons, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Float, "Button Size", this, &mButtonSize, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Asset, "Up Button Texture", this, &mUpButtonTexture, 1,
            HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Asset, "Down Button Texture", this, &mDownButtonTexture, 1,
            HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Asset, "Left Button Texture", this, &mLeftButtonTexture, 1,
            HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Asset, "Right Button Texture", this, &mRightButtonTexture, 1,
            HandlePropChange, int32_t(Texture::GetStaticType())));
        props.push_back(Property(DatumType::Color, "Button Color", this, &mButtonColor, 1,
            HandlePropChange));
    }
}

void ScrollContainer::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (!ShouldHandleInput())
    {
        return;
    }

    // Ensure rect is up to date
    if (IsDirty())
    {
        UpdateRect();
    }

    HandleInput(deltaTime);
    HandleButtonInput();
    HandleMomentum(deltaTime);
}

void ScrollContainer::HandleInput(float deltaTime)
{
    bool containsMouse = ContainsMouse();
    int32_t mouseX, mouseY;
    GetMousePosition(mouseX, mouseY);
    glm::vec2 mousePos((float)mouseX, (float)mouseY);


    // Check scrollbar hover states
    if (mVScrollbarGrabber->IsVisible())
    {
        Rect vRect = mVScrollbarGrabber->GetRect();
        bool wasHovered = mVScrollbarHovered;
        mVScrollbarHovered = vRect.ContainsPoint(mousePos.x, mousePos.y);
        if (wasHovered != mVScrollbarHovered)
        {
            MarkDirty();
        }
    }
    else
    {
        mVScrollbarHovered = false;
    }

    if (mHScrollbarGrabber->IsVisible())
    {
        Rect hRect = mHScrollbarGrabber->GetRect();
        bool wasHovered = mHScrollbarHovered;
        mHScrollbarHovered = hRect.ContainsPoint(mousePos.x, mousePos.y);
        if (wasHovered != mHScrollbarHovered)
        {
            MarkDirty();
        }
    }
    else
    {
        mHScrollbarHovered = false;
    }

    // Check if mouse is over any scroll button
    bool overButton = false;
    if (mShowScrollButtons)
    {
        if (mUpButton->IsVisible() && mUpButton->ContainsMouse())
            overButton = true;
        else if (mDownButton->IsVisible() && mDownButton->ContainsMouse())
            overButton = true;
        else if (mLeftButton->IsVisible() && mLeftButton->ContainsMouse())
            overButton = true;
        else if (mRightButton->IsVisible() && mRightButton->ContainsMouse())
            overButton = true;
    }

    // Handle pointer down
    if (IsPointerJustDown(0) && containsMouse)
    {
        if (mVScrollbarHovered)
        {
            mDraggingVScrollbar = true;
            mDragStartMouse = mousePos;
            mDragStartOffset = mScrollOffset;
            mScrollVelocity = glm::vec2(0.0f);
        }
        else if (mHScrollbarHovered)
        {
            mDraggingHScrollbar = true;
            mDragStartMouse = mousePos;
            mDragStartOffset = mScrollOffset;
            mScrollVelocity = glm::vec2(0.0f);
        }
        else if (!overButton)
        {
            // Check if child widget should receive input first
            bool overChild = mChildInputPriority && IsPointerOverChildWidget(mousePos.x, mousePos.y);

            if (!overChild)
            {
                // Content drag (only if not clicking on a button or prioritized child)
                mDragging = true;
                mDragStartMouse = mousePos;
                mDragStartOffset = mScrollOffset;
                mScrollVelocity = glm::vec2(0.0f);

                EmitSignal("ScrollDragStarted", { this });
                CallFunction("OnScrollDragStarted", { this });
            }
        }
        mLastMousePos = mousePos;
    }

    // Handle drag updates
    if (mDragging)
    {
        if (IsPointerDown(0))
        {
            glm::vec2 delta = mLastMousePos - mousePos;

            // Track velocity for momentum (using instantaneous velocity)
            if (deltaTime > 0.0f)
            {
                mScrollVelocity = delta / deltaTime;
            }

            SetScrollOffset(mScrollOffset + delta);
            mLastMousePos = mousePos;
        }
        else
        {
            // Pointer released - end drag
            mDragging = false;

            EmitSignal("ScrollDragEnded", { this });
            CallFunction("OnScrollDragEnded", { this });
        }
    }
    else if (mDraggingVScrollbar)
    {
        if (IsPointerDown(0))
        {
            float containerHeight = GetHeight();
            float buttonReserve = mShowScrollButtons ? mButtonSize * 2.0f : 0.0f;
            float scrollbarReserve = ShouldShowHScrollbar() ? mScrollbarWidth : 0.0f;
            float trackHeight = containerHeight - scrollbarReserve - buttonReserve;
            float contentHeight = mCachedContentSize.y;
            float viewportHeight = containerHeight - scrollbarReserve;
            float maxScroll = glm::max(0.0f, contentHeight - viewportHeight);

            if (trackHeight > 0.0f && maxScroll > 0.0f)
            {
                float buttonOffset = mShowScrollButtons ? mButtonSize : 0.0f;
                float scrollRatio = (mousePos.y - mDragStartMouse.y) / trackHeight;
                float scrollDelta = scrollRatio * maxScroll;
                SetScrollOffsetY(mDragStartOffset.y + scrollDelta);
            }
        }
        else
        {
            mDraggingVScrollbar = false;
        }
    }
    else if (mDraggingHScrollbar)
    {
        if (IsPointerDown(0))
        {
            float containerWidth = GetWidth();
            float buttonReserve = mShowScrollButtons ? mButtonSize * 2.0f : 0.0f;
            float scrollbarReserve = ShouldShowVScrollbar() ? mScrollbarWidth : 0.0f;
            float trackWidth = containerWidth - scrollbarReserve - buttonReserve;
            float contentWidth = mCachedContentSize.x;
            float viewportWidth = containerWidth - scrollbarReserve;
            float maxScroll = glm::max(0.0f, contentWidth - viewportWidth);

            if (trackWidth > 0.0f && maxScroll > 0.0f)
            {
                float scrollRatio = (mousePos.x - mDragStartMouse.x) / trackWidth;
                float scrollDelta = scrollRatio * maxScroll;
                SetScrollOffsetX(mDragStartOffset.x + scrollDelta);
            }
        }
        else
        {
            mDraggingHScrollbar = false;
        }
    }

    // Handle scroll wheel
    if (containsMouse && !mDragging && !mDraggingVScrollbar && !mDraggingHScrollbar)
    {
        int32_t scrollDelta = GetScrollWheelDelta();
        if (scrollDelta != 0)
        {
            // Prioritize vertical scroll, use horizontal if can't scroll vertically
            if (CanScrollVertically())
            {
                SetScrollOffsetY(mScrollOffset.y - scrollDelta * mScrollSpeed);
            }
            else if (CanScrollHorizontally())
            {
                SetScrollOffsetX(mScrollOffset.x - scrollDelta * mScrollSpeed);
            }
            mScrollVelocity = glm::vec2(0.0f);  // Stop momentum on wheel
        }
    }
}

void ScrollContainer::HandleButtonInput()
{
    if (!mShowScrollButtons)
    {
        return;
    }

    // Check if buttons were activated (pointer released while over pressed button)
    // Button activation happens when pointer is released while button is in Pressed state
    if (IsPointerJustUp(0))
    {
        if (mUpButton->IsVisible() && mUpButton->GetState() == ButtonState::Pressed && mUpButton->ContainsMouse())
        {
            SetScrollOffsetY(mScrollOffset.y - mScrollSpeed);
        }
        if (mDownButton->IsVisible() && mDownButton->GetState() == ButtonState::Pressed && mDownButton->ContainsMouse())
        {
            SetScrollOffsetY(mScrollOffset.y + mScrollSpeed);
        }
        if (mLeftButton->IsVisible() && mLeftButton->GetState() == ButtonState::Pressed && mLeftButton->ContainsMouse())
        {
            SetScrollOffsetX(mScrollOffset.x - mScrollSpeed);
        }
        if (mRightButton->IsVisible() && mRightButton->GetState() == ButtonState::Pressed && mRightButton->ContainsMouse())
        {
            SetScrollOffsetX(mScrollOffset.x + mScrollSpeed);
        }
    }
}

void ScrollContainer::HandleMomentum(float deltaTime)
{
    if (!mMomentumEnabled || mDragging || mDraggingVScrollbar || mDraggingHScrollbar)
    {
        return;
    }

    float velocityMagnitude = glm::length(mScrollVelocity);
    if (velocityMagnitude > 1.0f)
    {
        // Apply friction (exponential decay)
        mScrollVelocity *= glm::exp(-mMomentumFriction * deltaTime);

        // Apply velocity
        SetScrollOffset(mScrollOffset + mScrollVelocity * deltaTime);
    }
    else
    {
        mScrollVelocity = glm::vec2(0.0f);
    }
}

void ScrollContainer::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        // Find content widget (first non-transient child)
        Widget* content = GetContentWidget();
        if (content != nullptr)
        {
            float containerWidth = GetWidth();
            float containerHeight = GetHeight();

            // Reserve space for visible scrollbars
            float hReserve = ShouldShowVScrollbar() ? mScrollbarWidth : 0.0f;
            float vReserve = ShouldShowHScrollbar() ? mScrollbarWidth : 0.0f;

            float availableWidth = containerWidth - hReserve;
            float availableHeight = containerHeight - vReserve;

            // Apply size mode to content
            switch (mScrollSizeMode)
            {
                case ScrollSizeMode::FitWidth:
                    content->SetWidth(availableWidth);
                    break;
                case ScrollSizeMode::FitHeight:
                    content->SetHeight(availableHeight);
                    break;
                case ScrollSizeMode::FitBoth:
                    content->SetDimensions(availableWidth, availableHeight);
                    break;
                case ScrollSizeMode::None:
                    // Content keeps its own size
                    break;
                default:
                    break;
            }

            // Cache content size
            mCachedContentSize = content->GetDimensions();

            // Update content position based on scroll offset
            UpdateContentPosition();
        }
        else
        {
            mCachedContentSize = glm::vec2(0.0f);
        }

        // Update scrollbar visuals
        UpdateScrollbars();
        UpdateScrollButtons();
    }
}

Widget* ScrollContainer::GetContentWidget()
{
    uint32_t numChildren = GetNumChildren();
    for (uint32_t i = 0; i < numChildren; i++)
    {
        Widget* child = GetChildWidget(i);
        if (child != nullptr && !child->IsTransient())
        {
            return child;
        }
    }
    return nullptr;
}

void ScrollContainer::UpdateContentPosition()
{
    Widget* content = GetContentWidget();
    if (content != nullptr)
    {
        // Override content position with scroll offset
        content->SetAnchorMode(AnchorMode::TopLeft);
        content->SetPosition(-mScrollOffset.x, -mScrollOffset.y);
    }
}

void ScrollContainer::UpdateScrollbars()
{
    float containerWidth = GetWidth();
    float containerHeight = GetHeight();

    bool showVScrollbar = ShouldShowVScrollbar();
    bool showHScrollbar = ShouldShowHScrollbar();

    float hReserve = showVScrollbar ? mScrollbarWidth : 0.0f;
    float vReserve = showHScrollbar ? mScrollbarWidth : 0.0f;

    // Button reserve for scrollbar positioning
    float vButtonReserve = (mShowScrollButtons && showVScrollbar) ? mButtonSize : 0.0f;
    float hButtonReserve = (mShowScrollButtons && showHScrollbar) ? mButtonSize : 0.0f;

    // Calculate viewport and content sizes
    float viewportWidth = containerWidth - hReserve;
    float viewportHeight = containerHeight - vReserve;

    float contentWidth = mCachedContentSize.x;
    float contentHeight = mCachedContentSize.y;

    // Get textures
    Texture* scrollbarTex = mScrollbarTexture.Get<Texture>();
    Texture* trackTex = mTrackTexture.Get<Texture>();

    // Update vertical scrollbar
    mVScrollbarTrack->SetVisible(showVScrollbar);
    mVScrollbarGrabber->SetVisible(showVScrollbar);

    if (showVScrollbar)
    {
        float trackStartY = vButtonReserve;
        float trackHeight = containerHeight - vReserve - (vButtonReserve * 2.0f);

        // Position and style track
        mVScrollbarTrack->SetPosition(containerWidth - mScrollbarWidth, trackStartY);
        mVScrollbarTrack->SetDimensions(mScrollbarWidth, trackHeight);
        mVScrollbarTrack->SetTexture(trackTex);
        mVScrollbarTrack->SetColor(mScrollbarTrackColor);

        // Calculate grabber size and position
        float grabberRatio = glm::clamp(viewportHeight / contentHeight, 0.1f, 1.0f);
        float grabberHeight = trackHeight * grabberRatio;

        float maxScroll = glm::max(0.0f, contentHeight - viewportHeight);
        float scrollRatio = (maxScroll > 0.0f) ? (mScrollOffset.y / maxScroll) : 0.0f;
        scrollRatio = glm::clamp(scrollRatio, 0.0f, 1.0f);

        float grabberY = trackStartY + scrollRatio * (trackHeight - grabberHeight);

        mVScrollbarGrabber->SetPosition(containerWidth - mScrollbarWidth, grabberY);
        mVScrollbarGrabber->SetDimensions(mScrollbarWidth, grabberHeight);
        mVScrollbarGrabber->SetTexture(scrollbarTex);

        // Apply color based on hover state
        if (mVScrollbarHovered || mDraggingVScrollbar)
        {
            mVScrollbarGrabber->SetColor(mScrollbarHoveredColor);
        }
        else
        {
            mVScrollbarGrabber->SetColor(mScrollbarColor);
        }
    }

    // Update horizontal scrollbar
    mHScrollbarTrack->SetVisible(showHScrollbar);
    mHScrollbarGrabber->SetVisible(showHScrollbar);

    if (showHScrollbar)
    {
        float trackStartX = hButtonReserve;
        float trackWidth = containerWidth - hReserve - (hButtonReserve * 2.0f);

        // Position and style track
        mHScrollbarTrack->SetPosition(trackStartX, containerHeight - mScrollbarWidth);
        mHScrollbarTrack->SetDimensions(trackWidth, mScrollbarWidth);
        mHScrollbarTrack->SetTexture(trackTex);
        mHScrollbarTrack->SetColor(mScrollbarTrackColor);

        // Calculate grabber size and position
        float grabberRatio = glm::clamp(viewportWidth / contentWidth, 0.1f, 1.0f);
        float grabberWidth = trackWidth * grabberRatio;

        float maxScroll = glm::max(0.0f, contentWidth - viewportWidth);
        float scrollRatio = (maxScroll > 0.0f) ? (mScrollOffset.x / maxScroll) : 0.0f;
        scrollRatio = glm::clamp(scrollRatio, 0.0f, 1.0f);

        float grabberX = trackStartX + scrollRatio * (trackWidth - grabberWidth);

        mHScrollbarGrabber->SetPosition(grabberX, containerHeight - mScrollbarWidth);
        mHScrollbarGrabber->SetDimensions(grabberWidth, mScrollbarWidth);
        mHScrollbarGrabber->SetTexture(scrollbarTex);

        // Apply color based on hover state
        if (mHScrollbarHovered || mDraggingHScrollbar)
        {
            mHScrollbarGrabber->SetColor(mScrollbarHoveredColor);
        }
        else
        {
            mHScrollbarGrabber->SetColor(mScrollbarColor);
        }
    }
}

void ScrollContainer::UpdateScrollButtons()
{
    float containerWidth = GetWidth();
    float containerHeight = GetHeight();

    bool showVScrollbar = ShouldShowVScrollbar();
    bool showHScrollbar = ShouldShowHScrollbar();

    float vReserve = showHScrollbar ? mScrollbarWidth : 0.0f;
    float hReserve = showVScrollbar ? mScrollbarWidth : 0.0f;

    // Get textures for each button
    Texture* upTex = mUpButtonTexture.Get<Texture>();
    Texture* downTex = mDownButtonTexture.Get<Texture>();
    Texture* leftTex = mLeftButtonTexture.Get<Texture>();
    Texture* rightTex = mRightButtonTexture.Get<Texture>();

    // Calculate hovered/pressed colors from button color
    glm::vec4 hoveredColor = glm::min(mButtonColor * 1.3f, glm::vec4(1.0f));
    hoveredColor.a = mButtonColor.a;
    glm::vec4 pressedColor = mButtonColor * 0.8f;
    pressedColor.a = mButtonColor.a;

    // Vertical scroll buttons (up/down)
    bool showVButtons = mShowScrollButtons && showVScrollbar;
    mUpButton->SetVisible(showVButtons);
    mDownButton->SetVisible(showVButtons);

    if (showVButtons)
    {
        // Up button at top-right
        mUpButton->SetPosition(containerWidth - mScrollbarWidth, 0.0f);
        mUpButton->SetDimensions(mScrollbarWidth, mButtonSize);
        mUpButton->SetNormalColor(mButtonColor);
        mUpButton->SetHoveredColor(hoveredColor);
        mUpButton->SetPressedColor(pressedColor);
        if (upTex)
        {
            mUpButton->SetNormalTexture(upTex);
            mUpButton->SetHoveredTexture(upTex);
            mUpButton->SetPressedTexture(upTex);
            mUpButton->SetTextString("");  // Clear text when using texture
        }
        else
        {
            mUpButton->SetNormalTexture(nullptr);
            mUpButton->SetHoveredTexture(nullptr);
            mUpButton->SetPressedTexture(nullptr);
            mUpButton->SetTextString("^");
        }

        // Down button above horizontal scrollbar
        mDownButton->SetPosition(containerWidth - mScrollbarWidth, containerHeight - vReserve - mButtonSize);
        mDownButton->SetDimensions(mScrollbarWidth, mButtonSize);
        mDownButton->SetNormalColor(mButtonColor);
        mDownButton->SetHoveredColor(hoveredColor);
        mDownButton->SetPressedColor(pressedColor);
        if (downTex)
        {
            mDownButton->SetNormalTexture(downTex);
            mDownButton->SetHoveredTexture(downTex);
            mDownButton->SetPressedTexture(downTex);
            mDownButton->SetTextString("");
        }
        else
        {
            mDownButton->SetNormalTexture(nullptr);
            mDownButton->SetHoveredTexture(nullptr);
            mDownButton->SetPressedTexture(nullptr);
            mDownButton->SetTextString("v");
        }
    }

    // Horizontal scroll buttons (left/right)
    bool showHButtons = mShowScrollButtons && showHScrollbar;
    mLeftButton->SetVisible(showHButtons);
    mRightButton->SetVisible(showHButtons);

    if (showHButtons)
    {
        // Left button at bottom-left
        mLeftButton->SetPosition(0.0f, containerHeight - mScrollbarWidth);
        mLeftButton->SetDimensions(mButtonSize, mScrollbarWidth);
        mLeftButton->SetNormalColor(mButtonColor);
        mLeftButton->SetHoveredColor(hoveredColor);
        mLeftButton->SetPressedColor(pressedColor);
        if (leftTex)
        {
            mLeftButton->SetNormalTexture(leftTex);
            mLeftButton->SetHoveredTexture(leftTex);
            mLeftButton->SetPressedTexture(leftTex);
            mLeftButton->SetTextString("");
        }
        else
        {
            mLeftButton->SetNormalTexture(nullptr);
            mLeftButton->SetHoveredTexture(nullptr);
            mLeftButton->SetPressedTexture(nullptr);
            mLeftButton->SetTextString("<");
        }

        // Right button to the left of vertical scrollbar
        mRightButton->SetPosition(containerWidth - hReserve - mButtonSize, containerHeight - mScrollbarWidth);
        mRightButton->SetDimensions(mButtonSize, mScrollbarWidth);
        mRightButton->SetNormalColor(mButtonColor);
        mRightButton->SetHoveredColor(hoveredColor);
        mRightButton->SetPressedColor(pressedColor);
        if (rightTex)
        {
            mRightButton->SetNormalTexture(rightTex);
            mRightButton->SetHoveredTexture(rightTex);
            mRightButton->SetPressedTexture(rightTex);
            mRightButton->SetTextString("");
        }
        else
        {
            mRightButton->SetNormalTexture(nullptr);
            mRightButton->SetHoveredTexture(nullptr);
            mRightButton->SetPressedTexture(nullptr);
            mRightButton->SetTextString(">");
        }
    }
}

bool ScrollContainer::ShouldShowVScrollbar() const
{
    if (mVScrollbarMode == ScrollbarMode::Hidden)
    {
        return false;
    }

    if (mVScrollbarMode == ScrollbarMode::AlwaysVisible)
    {
        return true;
    }

    // Auto mode - show if content exceeds viewport
    float hReserve = (mHScrollbarMode == ScrollbarMode::AlwaysVisible) ? mScrollbarWidth : 0.0f;
    float viewportHeight = GetHeight() - hReserve;
    return mCachedContentSize.y > viewportHeight;
}

bool ScrollContainer::ShouldShowHScrollbar() const
{
    if (mHScrollbarMode == ScrollbarMode::Hidden)
    {
        return false;
    }

    if (mHScrollbarMode == ScrollbarMode::AlwaysVisible)
    {
        return true;
    }

    // Auto mode - show if content exceeds viewport
    float vReserve = (mVScrollbarMode == ScrollbarMode::AlwaysVisible) ? mScrollbarWidth : 0.0f;
    float viewportWidth = GetWidth() - vReserve;
    return mCachedContentSize.x > viewportWidth;
}

void ScrollContainer::ClampScrollOffset()
{
    glm::vec2 maxOffset = GetMaxScrollOffset();
    mScrollOffset.x = glm::clamp(mScrollOffset.x, 0.0f, maxOffset.x);
    mScrollOffset.y = glm::clamp(mScrollOffset.y, 0.0f, maxOffset.y);
}

void ScrollContainer::SetScrollOffset(glm::vec2 offset)
{
    glm::vec2 oldOffset = mScrollOffset;
    mScrollOffset = offset;
    ClampScrollOffset();

    if (mScrollOffset != oldOffset)
    {
        MarkDirty();

        if (!IsDestroyed())
        {
            EmitSignal("ScrollChanged", { this });
            CallFunction("OnScrollChanged", { this });
        }
    }
}

glm::vec2 ScrollContainer::GetScrollOffset() const
{
    return mScrollOffset;
}

void ScrollContainer::SetScrollOffsetX(float x)
{
    SetScrollOffset(glm::vec2(x, mScrollOffset.y));
}

void ScrollContainer::SetScrollOffsetY(float y)
{
    SetScrollOffset(glm::vec2(mScrollOffset.x, y));
}

void ScrollContainer::ScrollToTop()
{
    SetScrollOffsetY(0.0f);
}

void ScrollContainer::ScrollToBottom()
{
    glm::vec2 maxOffset = GetMaxScrollOffset();
    SetScrollOffsetY(maxOffset.y);
}

void ScrollContainer::ScrollToLeft()
{
    SetScrollOffsetX(0.0f);
}

void ScrollContainer::ScrollToRight()
{
    glm::vec2 maxOffset = GetMaxScrollOffset();
    SetScrollOffsetX(maxOffset.x);
}

void ScrollContainer::SetScrollSizeMode(ScrollSizeMode mode)
{
    if (mScrollSizeMode != mode)
    {
        mScrollSizeMode = mode;
        MarkDirty();
    }
}

ScrollSizeMode ScrollContainer::GetScrollSizeMode() const
{
    return mScrollSizeMode;
}

void ScrollContainer::SetHorizontalScrollbarMode(ScrollbarMode mode)
{
    if (mHScrollbarMode != mode)
    {
        mHScrollbarMode = mode;
        MarkDirty();
    }
}

ScrollbarMode ScrollContainer::GetHorizontalScrollbarMode() const
{
    return mHScrollbarMode;
}

void ScrollContainer::SetVerticalScrollbarMode(ScrollbarMode mode)
{
    if (mVScrollbarMode != mode)
    {
        mVScrollbarMode = mode;
        MarkDirty();
    }
}

ScrollbarMode ScrollContainer::GetVerticalScrollbarMode() const
{
    return mVScrollbarMode;
}

void ScrollContainer::SetScrollbarWidth(float width)
{
    if (mScrollbarWidth != width)
    {
        mScrollbarWidth = width;
        MarkDirty();
    }
}

float ScrollContainer::GetScrollbarWidth() const
{
    return mScrollbarWidth;
}

void ScrollContainer::SetScrollSpeed(float speed)
{
    mScrollSpeed = speed;
}

float ScrollContainer::GetScrollSpeed() const
{
    return mScrollSpeed;
}

void ScrollContainer::SetMomentumEnabled(bool enabled)
{
    mMomentumEnabled = enabled;
    if (!enabled)
    {
        mScrollVelocity = glm::vec2(0.0f);
    }
}

bool ScrollContainer::IsMomentumEnabled() const
{
    return mMomentumEnabled;
}

void ScrollContainer::SetMomentumFriction(float friction)
{
    mMomentumFriction = friction;
}

float ScrollContainer::GetMomentumFriction() const
{
    return mMomentumFriction;
}

bool ScrollContainer::CanScrollHorizontally() const
{
    float vReserve = ShouldShowVScrollbar() ? mScrollbarWidth : 0.0f;
    float viewportWidth = GetWidth() - vReserve;
    return mCachedContentSize.x > viewportWidth;
}

bool ScrollContainer::CanScrollVertically() const
{
    float hReserve = ShouldShowHScrollbar() ? mScrollbarWidth : 0.0f;
    float viewportHeight = GetHeight() - hReserve;
    return mCachedContentSize.y > viewportHeight;
}

glm::vec2 ScrollContainer::GetContentSize() const
{
    return mCachedContentSize;
}

glm::vec2 ScrollContainer::GetMaxScrollOffset() const
{
    float hReserve = ShouldShowVScrollbar() ? mScrollbarWidth : 0.0f;
    float vReserve = ShouldShowHScrollbar() ? mScrollbarWidth : 0.0f;

    float viewportWidth = GetWidth() - hReserve;
    float viewportHeight = GetHeight() - vReserve;

    return glm::vec2(
        glm::max(0.0f, mCachedContentSize.x - viewportWidth),
        glm::max(0.0f, mCachedContentSize.y - viewportHeight)
    );
}

bool ScrollContainer::IsDragging() const
{
    return mDragging;
}

bool ScrollContainer::IsScrolling() const
{
    return mDragging || mDraggingVScrollbar || mDraggingHScrollbar ||
           glm::length(mScrollVelocity) > 1.0f;
}

void ScrollContainer::SetChildInputPriority(bool priority)
{
    mChildInputPriority = priority;
}

bool ScrollContainer::GetChildInputPriority() const
{
    return mChildInputPriority;
}

bool ScrollContainer::IsPointerOverChildWidget(float x, float y) const
{
    // Check if pointer is over any non-transient child widget (excluding scrollbar widgets)
    const std::vector<NodePtr>& children = GetChildren();

    for (const NodePtr& child : children)
    {
        if (child == nullptr || !child->IsWidget())
            continue;

        Widget* childWidget = static_cast<Widget*>(child.Get());

        // Skip transient widgets (scrollbars, buttons created by ScrollContainer)
        if (childWidget->IsTransient())
            continue;

        // Skip invisible widgets
        if (!childWidget->IsVisible())
            continue;

        // Check if point is inside this child
        if (childWidget->ContainsPoint(static_cast<int32_t>(x), static_cast<int32_t>(y)))
            return true;
    }
    return false;
}

void ScrollContainer::SetScrollbarColor(glm::vec4 color)
{
    if (mScrollbarColor != color)
    {
        mScrollbarColor = color;
        MarkDirty();
    }
}

glm::vec4 ScrollContainer::GetScrollbarColor() const
{
    return mScrollbarColor;
}

void ScrollContainer::SetScrollbarHoveredColor(glm::vec4 color)
{
    if (mScrollbarHoveredColor != color)
    {
        mScrollbarHoveredColor = color;
        MarkDirty();
    }
}

glm::vec4 ScrollContainer::GetScrollbarHoveredColor() const
{
    return mScrollbarHoveredColor;
}

void ScrollContainer::SetScrollbarTrackColor(glm::vec4 color)
{
    if (mScrollbarTrackColor != color)
    {
        mScrollbarTrackColor = color;
        MarkDirty();
    }
}

glm::vec4 ScrollContainer::GetScrollbarTrackColor() const
{
    return mScrollbarTrackColor;
}

void ScrollContainer::SetScrollbarTexture(Texture* texture)
{
    if (mScrollbarTexture != texture)
    {
        mScrollbarTexture = texture;
        MarkDirty();
    }
}

Texture* ScrollContainer::GetScrollbarTexture()
{
    return mScrollbarTexture.Get<Texture>();
}

void ScrollContainer::SetTrackTexture(Texture* texture)
{
    if (mTrackTexture != texture)
    {
        mTrackTexture = texture;
        MarkDirty();
    }
}

Texture* ScrollContainer::GetTrackTexture()
{
    return mTrackTexture.Get<Texture>();
}

void ScrollContainer::SetShowScrollButtons(bool show)
{
    if (mShowScrollButtons != show)
    {
        mShowScrollButtons = show;
        MarkDirty();
    }
}

bool ScrollContainer::GetShowScrollButtons() const
{
    return mShowScrollButtons;
}

void ScrollContainer::SetButtonSize(float size)
{
    if (mButtonSize != size)
    {
        mButtonSize = size;
        MarkDirty();
    }
}

float ScrollContainer::GetButtonSize() const
{
    return mButtonSize;
}

void ScrollContainer::SetUpButtonTexture(Texture* texture)
{
    if (mUpButtonTexture != texture)
    {
        mUpButtonTexture = texture;
        MarkDirty();
    }
}

Texture* ScrollContainer::GetUpButtonTexture()
{
    return mUpButtonTexture.Get<Texture>();
}

void ScrollContainer::SetDownButtonTexture(Texture* texture)
{
    if (mDownButtonTexture != texture)
    {
        mDownButtonTexture = texture;
        MarkDirty();
    }
}

Texture* ScrollContainer::GetDownButtonTexture()
{
    return mDownButtonTexture.Get<Texture>();
}

void ScrollContainer::SetLeftButtonTexture(Texture* texture)
{
    if (mLeftButtonTexture != texture)
    {
        mLeftButtonTexture = texture;
        MarkDirty();
    }
}

Texture* ScrollContainer::GetLeftButtonTexture()
{
    return mLeftButtonTexture.Get<Texture>();
}

void ScrollContainer::SetRightButtonTexture(Texture* texture)
{
    if (mRightButtonTexture != texture)
    {
        mRightButtonTexture = texture;
        MarkDirty();
    }
}

Texture* ScrollContainer::GetRightButtonTexture()
{
    return mRightButtonTexture.Get<Texture>();
}

void ScrollContainer::SetButtonColor(glm::vec4 color)
{
    if (mButtonColor != color)
    {
        mButtonColor = color;
        MarkDirty();
    }
}

glm::vec4 ScrollContainer::GetButtonColor() const
{
    return mButtonColor;
}

Quad* ScrollContainer::GetHScrollbar()
{
    return mHScrollbarGrabber;
}

Quad* ScrollContainer::GetVScrollbar()
{
    return mVScrollbarGrabber;
}

Quad* ScrollContainer::GetHTrack()
{
    return mHScrollbarTrack;
}

Quad* ScrollContainer::GetVTrack()
{
    return mVScrollbarTrack;
}

Button* ScrollContainer::GetUpButton()
{
    return mUpButton;
}

Button* ScrollContainer::GetDownButton()
{
    return mDownButton;
}

Button* ScrollContainer::GetLeftButton()
{
    return mLeftButton;
}

Button* ScrollContainer::GetRightButton()
{
    return mRightButton;
}
