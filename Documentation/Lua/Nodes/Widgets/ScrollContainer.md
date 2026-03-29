# ScrollContainer

A scrollable container widget. Add a child widget to provide scrollable content. The container clips content outside its bounds and supports scroll wheel, touch/drag scrolling, and optional scrollbars.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetScrollOffset
Set the scroll position.

Sig: `ScrollContainer:SetScrollOffset(x, y)`
 - Arg: `number x` Horizontal scroll offset in pixels
 - Arg: `number y` Vertical scroll offset in pixels
---
### GetScrollOffset
Get the current scroll position.

Sig: `x, y = ScrollContainer:GetScrollOffset()`
 - Ret: `number x` Horizontal scroll offset
 - Ret: `number y` Vertical scroll offset
---
### SetScrollOffsetX
Set only the horizontal scroll position.

Sig: `ScrollContainer:SetScrollOffsetX(x)`
 - Arg: `number x` Horizontal scroll offset
---
### SetScrollOffsetY
Set only the vertical scroll position.

Sig: `ScrollContainer:SetScrollOffsetY(y)`
 - Arg: `number y` Vertical scroll offset
---
### ScrollToTop
Scroll to the top of the content (y = 0).

Sig: `ScrollContainer:ScrollToTop()`
---
### ScrollToBottom
Scroll to the bottom of the content.

Sig: `ScrollContainer:ScrollToBottom()`
---
### ScrollToLeft
Scroll to the left of the content (x = 0).

Sig: `ScrollContainer:ScrollToLeft()`
---
### ScrollToRight
Scroll to the right of the content.

Sig: `ScrollContainer:ScrollToRight()`
---
### SetScrollSizeMode
Set how the content is sized relative to the container.

ScrollSizeMode values:
- 0 = FitWidth (content fills width, scrolls vertically)
- 1 = FitHeight (content fills height, scrolls horizontally)
- 2 = FitBoth (content fills both dimensions)
- 3 = None (content uses its own size)

Sig: `ScrollContainer:SetScrollSizeMode(mode)`
 - Arg: `integer mode` Size mode (0-3)
---
### GetScrollSizeMode
Get the current size mode.

Sig: `mode = ScrollContainer:GetScrollSizeMode()`
 - Ret: `integer mode` Size mode
---
### SetHorizontalScrollbarMode
Set horizontal scrollbar visibility mode.

ScrollbarMode values:
- 0 = Hidden (never show)
- 1 = Auto (show when needed)
- 2 = AlwaysVisible (always show)

Sig: `ScrollContainer:SetHorizontalScrollbarMode(mode)`
 - Arg: `integer mode` Scrollbar mode (0-2)
---
### GetHorizontalScrollbarMode
Get the horizontal scrollbar mode.

Sig: `mode = ScrollContainer:GetHorizontalScrollbarMode()`
 - Ret: `integer mode` Scrollbar mode
---
### SetVerticalScrollbarMode
Set vertical scrollbar visibility mode.

Sig: `ScrollContainer:SetVerticalScrollbarMode(mode)`
 - Arg: `integer mode` Scrollbar mode (0-2)
---
### GetVerticalScrollbarMode
Get the vertical scrollbar mode.

Sig: `mode = ScrollContainer:GetVerticalScrollbarMode()`
 - Ret: `integer mode` Scrollbar mode
---
### SetScrollbarWidth
Set the width of scrollbars in pixels.

Sig: `ScrollContainer:SetScrollbarWidth(width)`
 - Arg: `number width` Scrollbar width
---
### GetScrollbarWidth
Get the scrollbar width.

Sig: `width = ScrollContainer:GetScrollbarWidth()`
 - Ret: `number width` Scrollbar width
---
### SetScrollSpeed
Set pixels scrolled per mouse wheel notch.

Sig: `ScrollContainer:SetScrollSpeed(speed)`
 - Arg: `number speed` Scroll speed in pixels (default: 30)
---
### GetScrollSpeed
Get the scroll speed.

Sig: `speed = ScrollContainer:GetScrollSpeed()`
 - Ret: `number speed` Scroll speed
---
### SetMomentumEnabled
Enable or disable momentum scrolling after drag release.

Sig: `ScrollContainer:SetMomentumEnabled(enabled)`
 - Arg: `boolean enabled` Enable momentum
---
### IsMomentumEnabled
Check if momentum scrolling is enabled.

Sig: `enabled = ScrollContainer:IsMomentumEnabled()`
 - Ret: `boolean enabled` Momentum enabled
---
### SetMomentumFriction
Set how quickly momentum decays. Higher values = faster decay.

Sig: `ScrollContainer:SetMomentumFriction(friction)`
 - Arg: `number friction` Friction factor (default: 5.0)
---
### GetMomentumFriction
Get the momentum friction value.

Sig: `friction = ScrollContainer:GetMomentumFriction()`
 - Ret: `number friction` Friction factor
---
### CanScrollHorizontally
Check if content exceeds container width (can scroll horizontally).

Sig: `canScroll = ScrollContainer:CanScrollHorizontally()`
 - Ret: `boolean canScroll` True if horizontal scrolling is possible
---
### CanScrollVertically
Check if content exceeds container height (can scroll vertically).

Sig: `canScroll = ScrollContainer:CanScrollVertically()`
 - Ret: `boolean canScroll` True if vertical scrolling is possible
---
### GetContentSize
Get the size of the content widget.

Sig: `width, height = ScrollContainer:GetContentSize()`
 - Ret: `number width` Content width
 - Ret: `number height` Content height
---
### GetMaxScrollOffset
Get the maximum scroll offset (content size - viewport size).

Sig: `maxX, maxY = ScrollContainer:GetMaxScrollOffset()`
 - Ret: `number maxX` Maximum horizontal scroll
 - Ret: `number maxY` Maximum vertical scroll
---
### IsDragging
Check if user is currently dragging the content.

Sig: `dragging = ScrollContainer:IsDragging()`
 - Ret: `boolean dragging` True if currently dragging
---
### IsScrolling
Check if the scroll container is currently scrolling (dragging or momentum).

Sig: `scrolling = ScrollContainer:IsScrolling()`
 - Ret: `boolean scrolling` True if scrolling
---
### GetContentWidget
Get the content widget (first non-transient child).

Sig: `content = ScrollContainer:GetContentWidget()`
 - Ret: `Widget content` Content widget or nil
---
### SetScrollbarColor
Set the scrollbar grabber color.

Sig: `ScrollContainer:SetScrollbarColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetScrollbarColor
Get the scrollbar grabber color.

Sig: `color = ScrollContainer:GetScrollbarColor()`
 - Ret: `Vector color` RGBA color
---
### SetScrollbarHoveredColor
Set the scrollbar color when hovered.

Sig: `ScrollContainer:SetScrollbarHoveredColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetScrollbarHoveredColor
Get the scrollbar hovered color.

Sig: `color = ScrollContainer:GetScrollbarHoveredColor()`
 - Ret: `Vector color` RGBA color
---
### SetScrollbarTrackColor
Set the scrollbar track background color.

Sig: `ScrollContainer:SetScrollbarTrackColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetScrollbarTrackColor
Get the scrollbar track color.

Sig: `color = ScrollContainer:GetScrollbarTrackColor()`
 - Ret: `Vector color` RGBA color
---
### GetHScrollbar
Get the internal horizontal scrollbar grabber Quad widget.

Sig: `quad = ScrollContainer:GetHScrollbar()`
 - Ret: `Quad quad` Horizontal scrollbar grabber
---
### GetVScrollbar
Get the internal vertical scrollbar grabber Quad widget.

Sig: `quad = ScrollContainer:GetVScrollbar()`
 - Ret: `Quad quad` Vertical scrollbar grabber
---

## Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| ScrollChanged | self | Emitted when scroll position changes |
| ScrollDragStarted | self | Emitted when drag starts |
| ScrollDragEnded | self | Emitted when drag ends |

## Script Callbacks

| Callback | Parameters | Description |
|----------|------------|-------------|
| OnScrollChanged | self | Called when scroll position changes |
| OnScrollDragStarted | self | Called when drag starts |
| OnScrollDragEnded | self | Called when drag ends |

## Example

```lua
-- Create a scrollable list
local scroll = Node.Create("ScrollContainer")
scroll:SetDimensions(200, 300)
scroll:SetScrollSizeMode(0) -- FitWidth
scroll:SetMomentumEnabled(true)

-- Add content
local content = Node.Create("ArrayWidget")
content:SetOrientation(0) -- Vertical
scroll:AddChild(content)

-- Add items to content
for i = 1, 20 do
    local item = Node.Create("Button")
    item:SetDimensions(180, 40)
    item:SetTextString("Item " .. i)
    content:AddChild(item)
end

-- Handle scroll events
scroll.OnScrollChanged = function(self)
    local x, y = self:GetScrollOffset()
    print("Scrolled to:", x, y)
end
```
