# ScrollContainer Widget

A scrollable container widget that allows you to scroll through content that exceeds the container's visible area. The ScrollContainer clips its contents and provides both touch/drag scrolling and mouse wheel support.

## Overview

The ScrollContainer is designed to hold a single content widget as its child. When the content exceeds the container's dimensions, scrollbars appear (based on configuration) and the user can scroll through the content.

## Size Modes

The `Size Mode` property controls how the content widget is sized relative to the ScrollContainer:

| Mode | Description |
|------|-------------|
| **Fit Width** | Content fills the container's width and scrolls vertically. Best for vertical lists or text. |
| **Fit Height** | Content fills the container's height and scrolls horizontally. Best for horizontal galleries. |
| **Fit Both** | Content fills both dimensions. Only scrolls if content explicitly overflows. |
| **None** | Content uses its own explicit size. Scrolls in both directions as needed. |

## Scrollbar Modes

Both horizontal and vertical scrollbars can be configured independently:

| Mode | Description |
|------|-------------|
| **Hidden** | Never show the scrollbar |
| **Auto** | Show scrollbar only when content exceeds viewport (default) |
| **Always Visible** | Always show the scrollbar even when not needed |

## Properties

### ScrollContainer Category
- **Size Mode** - How content is sized (see above)
- **Scroll Offset** - Current scroll position (x, y)
- **Scroll Speed** - Pixels scrolled per mouse wheel notch (default: 30)
- **Momentum Enabled** - Enable momentum/inertia after drag release (default: true)
- **Momentum Friction** - How quickly momentum decays (default: 5.0)

### Scrollbars Category
- **Horizontal Scrollbar** - Visibility mode for horizontal scrollbar
- **Vertical Scrollbar** - Visibility mode for vertical scrollbar
- **Scrollbar Width** - Width of scrollbars in pixels (default: 8)
- **Scrollbar Color** - Color of scrollbar grabber
- **Scrollbar Hovered Color** - Color when hovering over scrollbar
- **Track Color** - Color of scrollbar track background

## Input Handling

### Mouse Wheel
When the mouse is over the ScrollContainer:
- Scroll wheel up/down scrolls vertically (if possible)
- If vertical scrolling is not possible, scrolls horizontally

### Touch/Drag
- Click and drag anywhere in the container to scroll
- Release to apply momentum scrolling (if enabled)

### Scrollbar Dragging
- Click and drag the scrollbar grabber for precise scrolling

## Signals

| Signal | Description |
|--------|-------------|
| `ScrollChanged` | Emitted when scroll position changes |
| `ScrollDragStarted` | Emitted when user starts dragging content |
| `ScrollDragEnded` | Emitted when user stops dragging content |

## Script Callbacks

| Callback | Description |
|----------|-------------|
| `OnScrollChanged` | Called when scroll position changes |
| `OnScrollDragStarted` | Called when drag starts |
| `OnScrollDragEnded` | Called when drag ends |

## Usage Example

1. Create a ScrollContainer widget in the editor
2. Set the desired Size Mode (e.g., "Fit Width" for a vertical scrolling list)
3. Add a child widget (Canvas, ArrayWidget, etc.) as the content
4. The content will be scrollable if it exceeds the container's bounds

### Typical Setup for a Scrollable List

1. Add ScrollContainer with Size Mode = "Fit Width"
2. Add an ArrayWidget as child with Orientation = "Vertical"
3. Add list items to the ArrayWidget
4. Items will scroll vertically when they exceed the container height

## Notes

- The ScrollContainer automatically enables scissoring to clip content outside its bounds
- The first non-transient child widget is treated as the content
- Internal scrollbar widgets are transient and hidden in the hierarchy view
- Content widget's anchor mode is overridden to TopLeft for scrolling to work correctly
