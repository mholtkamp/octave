#pragma once

#include "Nodes/Widgets/Widget.h"

class Quad;

enum class ScrollSizeMode : uint8_t
{
    FitWidth,       // Content fills parent width, scrolls vertically
    FitHeight,      // Content fills parent height, scrolls horizontally
    FitBoth,        // Content fills both dimensions
    None,           // Content uses explicit size
    Count
};

enum class ScrollbarMode : uint8_t
{
    Hidden,         // Never show scrollbars
    Auto,           // Show only when needed
    AlwaysVisible,  // Always show
    Count
};

class OCTAVE_API ScrollContainer : public Widget
{
public:

    DECLARE_NODE(ScrollContainer, Widget);

    virtual void Create() override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void Tick(float deltaTime) override;
    virtual void PreRender() override;

    // Scroll Position
    void SetScrollOffset(glm::vec2 offset);
    glm::vec2 GetScrollOffset() const;
    void SetScrollOffsetX(float x);
    void SetScrollOffsetY(float y);
    void ScrollToTop();
    void ScrollToBottom();
    void ScrollToLeft();
    void ScrollToRight();

    // Size Mode
    void SetScrollSizeMode(ScrollSizeMode mode);
    ScrollSizeMode GetScrollSizeMode() const;

    // Scrollbar Configuration
    void SetHorizontalScrollbarMode(ScrollbarMode mode);
    ScrollbarMode GetHorizontalScrollbarMode() const;
    void SetVerticalScrollbarMode(ScrollbarMode mode);
    ScrollbarMode GetVerticalScrollbarMode() const;
    void SetScrollbarWidth(float width);
    float GetScrollbarWidth() const;

    // Scroll Behavior
    void SetScrollSpeed(float speed);
    float GetScrollSpeed() const;
    void SetMomentumEnabled(bool enabled);
    bool IsMomentumEnabled() const;
    void SetMomentumFriction(float friction);
    float GetMomentumFriction() const;

    // Query
    bool CanScrollHorizontally() const;
    bool CanScrollVertically() const;
    glm::vec2 GetContentSize() const;
    glm::vec2 GetMaxScrollOffset() const;
    bool IsDragging() const;
    bool IsScrolling() const;

    // Content Access
    Widget* GetContentWidget();

    // Visual Customization
    void SetScrollbarColor(glm::vec4 color);
    glm::vec4 GetScrollbarColor() const;
    void SetScrollbarHoveredColor(glm::vec4 color);
    glm::vec4 GetScrollbarHoveredColor() const;
    void SetScrollbarTrackColor(glm::vec4 color);
    glm::vec4 GetScrollbarTrackColor() const;

    Quad* GetHScrollbar();
    Quad* GetVScrollbar();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void UpdateScrollbars();
    void UpdateContentPosition();
    void ClampScrollOffset();
    void HandleInput(float deltaTime);
    void HandleMomentum(float deltaTime);
    bool ShouldShowHScrollbar() const;
    bool ShouldShowVScrollbar() const;

    // Scroll State
    glm::vec2 mScrollOffset = glm::vec2(0.0f);
    glm::vec2 mScrollVelocity = glm::vec2(0.0f);
    glm::vec2 mDragStartOffset = glm::vec2(0.0f);
    glm::vec2 mDragStartMouse = glm::vec2(0.0f);
    glm::vec2 mLastMousePos = glm::vec2(0.0f);

    // Size & Mode
    ScrollSizeMode mScrollSizeMode = ScrollSizeMode::FitWidth;
    ScrollbarMode mHScrollbarMode = ScrollbarMode::Auto;
    ScrollbarMode mVScrollbarMode = ScrollbarMode::Auto;

    // Scroll Behavior
    float mScrollSpeed = 30.0f;          // Pixels per scroll wheel notch
    float mMomentumFriction = 5.0f;      // Deceleration factor
    bool mMomentumEnabled = true;

    // Visual
    float mScrollbarWidth = 8.0f;
    glm::vec4 mScrollbarColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.8f);
    glm::vec4 mScrollbarHoveredColor = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    glm::vec4 mScrollbarTrackColor = glm::vec4(0.2f, 0.2f, 0.2f, 0.5f);

    // State
    bool mDragging = false;
    bool mDraggingHScrollbar = false;
    bool mDraggingVScrollbar = false;
    bool mHScrollbarHovered = false;
    bool mVScrollbarHovered = false;

    // Cached content size (updated during PreRender)
    glm::vec2 mCachedContentSize = glm::vec2(0.0f);

    // Transient child widgets
    Quad* mHScrollbarTrack = nullptr;
    Quad* mHScrollbarGrabber = nullptr;
    Quad* mVScrollbarTrack = nullptr;
    Quad* mVScrollbarGrabber = nullptr;
};
