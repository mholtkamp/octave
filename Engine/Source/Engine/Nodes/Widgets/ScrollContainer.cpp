
#include "Nodes/Widgets/ScrollContainer.h"
#include "Nodes/Widgets/Quad.h"
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

    // Mark as transient so they don't serialize
    mVScrollbarTrack->SetTransient(true);
    mVScrollbarGrabber->SetTransient(true);
    mHScrollbarTrack->SetTransient(true);
    mHScrollbarGrabber->SetTransient(true);

#if EDITOR
    mVScrollbarTrack->mHiddenInTree = true;
    mVScrollbarGrabber->mHiddenInTree = true;
    mHScrollbarTrack->mHiddenInTree = true;
    mHScrollbarGrabber->mHiddenInTree = true;
#endif

    // Configure scrollbar anchor modes
    mVScrollbarTrack->SetAnchorMode(AnchorMode::TopLeft);
    mVScrollbarGrabber->SetAnchorMode(AnchorMode::TopLeft);
    mHScrollbarTrack->SetAnchorMode(AnchorMode::TopLeft);
    mHScrollbarGrabber->SetAnchorMode(AnchorMode::TopLeft);

    // Set initial colors
    mVScrollbarTrack->SetColor(mScrollbarTrackColor);
    mVScrollbarGrabber->SetColor(mScrollbarColor);
    mHScrollbarTrack->SetColor(mScrollbarTrackColor);
    mHScrollbarGrabber->SetColor(mScrollbarColor);

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
    }

    {
        SCOPED_CATEGORY("Scrollbars");

        props.push_back(Property(DatumType::Byte, "Horizontal Scrollbar", this, &mHScrollbarMode, 1,
            HandlePropChange, NULL_DATUM, int32_t(ScrollbarMode::Count), sScrollbarModeStrings));
        props.push_back(Property(DatumType::Byte, "Vertical Scrollbar", this, &mVScrollbarMode, 1,
            HandlePropChange, NULL_DATUM, int32_t(ScrollbarMode::Count), sScrollbarModeStrings));
        props.push_back(Property(DatumType::Float, "Scrollbar Width", this, &mScrollbarWidth, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Color, "Scrollbar Color", this, &mScrollbarColor, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Color, "Scrollbar Hovered Color", this, &mScrollbarHoveredColor, 1,
            HandlePropChange));
        props.push_back(Property(DatumType::Color, "Track Color", this, &mScrollbarTrackColor, 1,
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
        mVScrollbarHovered = vRect.ContainsPoint(mousePos.x, mousePos.y);
    }
    else
    {
        mVScrollbarHovered = false;
    }

    if (mHScrollbarGrabber->IsVisible())
    {
        Rect hRect = mHScrollbarGrabber->GetRect();
        mHScrollbarHovered = hRect.ContainsPoint(mousePos.x, mousePos.y);
    }
    else
    {
        mHScrollbarHovered = false;
    }

    // Update scrollbar colors based on hover/drag state
    if (mVScrollbarHovered || mDraggingVScrollbar)
    {
        mVScrollbarGrabber->SetColor(mScrollbarHoveredColor);
    }
    else
    {
        mVScrollbarGrabber->SetColor(mScrollbarColor);
    }

    if (mHScrollbarHovered || mDraggingHScrollbar)
    {
        mHScrollbarGrabber->SetColor(mScrollbarHoveredColor);
    }
    else
    {
        mHScrollbarGrabber->SetColor(mScrollbarColor);
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
        else
        {
            // Content drag
            mDragging = true;
            mDragStartMouse = mousePos;
            mDragStartOffset = mScrollOffset;
            mScrollVelocity = glm::vec2(0.0f);

            EmitSignal("ScrollDragStarted", { this });
            CallFunction("OnScrollDragStarted", { this });
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
            float scrollbarReserve = ShouldShowHScrollbar() ? mScrollbarWidth : 0.0f;
            float trackHeight = containerHeight - scrollbarReserve;
            float contentHeight = mCachedContentSize.y;
            float viewportHeight = containerHeight - scrollbarReserve;
            float maxScroll = glm::max(0.0f, contentHeight - viewportHeight);

            if (trackHeight > 0.0f && maxScroll > 0.0f)
            {
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
            float scrollbarReserve = ShouldShowVScrollbar() ? mScrollbarWidth : 0.0f;
            float trackWidth = containerWidth - scrollbarReserve;
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

    // Calculate viewport and content sizes
    float viewportWidth = containerWidth - hReserve;
    float viewportHeight = containerHeight - vReserve;

    float contentWidth = mCachedContentSize.x;
    float contentHeight = mCachedContentSize.y;

    // Update vertical scrollbar
    mVScrollbarTrack->SetVisible(showVScrollbar);
    mVScrollbarGrabber->SetVisible(showVScrollbar);

    if (showVScrollbar)
    {
        // Position track on right side
        mVScrollbarTrack->SetPosition(containerWidth - mScrollbarWidth, 0.0f);
        mVScrollbarTrack->SetDimensions(mScrollbarWidth, containerHeight - vReserve);
        mVScrollbarTrack->SetColor(mScrollbarTrackColor);

        // Calculate grabber size and position
        float trackHeight = containerHeight - vReserve;
        float grabberRatio = glm::clamp(viewportHeight / contentHeight, 0.1f, 1.0f);
        float grabberHeight = trackHeight * grabberRatio;

        float maxScroll = glm::max(0.0f, contentHeight - viewportHeight);
        float scrollRatio = (maxScroll > 0.0f) ? (mScrollOffset.y / maxScroll) : 0.0f;
        scrollRatio = glm::clamp(scrollRatio, 0.0f, 1.0f);

        float grabberY = scrollRatio * (trackHeight - grabberHeight);

        mVScrollbarGrabber->SetPosition(containerWidth - mScrollbarWidth, grabberY);
        mVScrollbarGrabber->SetDimensions(mScrollbarWidth, grabberHeight);
    }

    // Update horizontal scrollbar
    mHScrollbarTrack->SetVisible(showHScrollbar);
    mHScrollbarGrabber->SetVisible(showHScrollbar);

    if (showHScrollbar)
    {
        // Position track on bottom
        mHScrollbarTrack->SetPosition(0.0f, containerHeight - mScrollbarWidth);
        mHScrollbarTrack->SetDimensions(containerWidth - hReserve, mScrollbarWidth);
        mHScrollbarTrack->SetColor(mScrollbarTrackColor);

        // Calculate grabber size and position
        float trackWidth = containerWidth - hReserve;
        float grabberRatio = glm::clamp(viewportWidth / contentWidth, 0.1f, 1.0f);
        float grabberWidth = trackWidth * grabberRatio;

        float maxScroll = glm::max(0.0f, contentWidth - viewportWidth);
        float scrollRatio = (maxScroll > 0.0f) ? (mScrollOffset.x / maxScroll) : 0.0f;
        scrollRatio = glm::clamp(scrollRatio, 0.0f, 1.0f);

        float grabberX = scrollRatio * (trackWidth - grabberWidth);

        mHScrollbarGrabber->SetPosition(grabberX, containerHeight - mScrollbarWidth);
        mHScrollbarGrabber->SetDimensions(grabberWidth, mScrollbarWidth);
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

Quad* ScrollContainer::GetHScrollbar()
{
    return mHScrollbarGrabber;
}

Quad* ScrollContainer::GetVScrollbar()
{
    return mVScrollbarGrabber;
}
