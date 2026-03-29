# Window

A window widget with title bar, close button, content area, and optional resize handle. Add child widgets to populate the content area. Windows can be registered with WindowManager by setting a Window Id.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetWindowId
Set the window's unique identifier for WindowManager registration.

Sig: `Window:SetWindowId(id)`
 - Arg: `string id` Unique window identifier
---
### GetWindowId
Get the window's identifier.

Sig: `id = Window:GetWindowId()`
 - Ret: `string id` Window identifier
---
### SetTitle
Set the title bar text.

Sig: `Window:SetTitle(title)`
 - Arg: `string title` Title text
---
### GetTitle
Get the title bar text.

Sig: `title = Window:GetTitle()`
 - Ret: `string title` Title text
---
### SetTitleAlignment
Set the title text alignment.

TitleAlignment values:
- 0 = Left
- 1 = Center
- 2 = Right

Sig: `Window:SetTitleAlignment(alignment)`
 - Arg: `integer alignment` Title alignment (0-2)
---
### GetTitleAlignment
Get the title text alignment.

Sig: `alignment = Window:GetTitleAlignment()`
 - Ret: `integer alignment` Title alignment
---
### SetTitleFontSize
Set the title text font size.

Sig: `Window:SetTitleFontSize(size)`
 - Arg: `number size` Font size in pixels
---
### GetTitleFontSize
Get the title text font size.

Sig: `size = Window:GetTitleFontSize()`
 - Ret: `number size` Font size
---
### SetTitleOffset
Set the title text offset from its default position.

Sig: `Window:SetTitleOffset(offset)`
 - Arg: `Vector offset` X and Y offset in pixels
---
### GetTitleOffset
Get the title text offset.

Sig: `offset = Window:GetTitleOffset()`
 - Ret: `Vector offset` X and Y offset
---
### SetContentPadding
Set all content padding values at once.

Sig: `Window:SetContentPadding(left, top, right, bottom)`
 - Arg: `number left` Left padding in pixels
 - Arg: `number top` Top padding in pixels
 - Arg: `number right` Right padding in pixels
 - Arg: `number bottom` Bottom padding in pixels
---
### SetContentPaddingLeft
Set the left content padding.

Sig: `Window:SetContentPaddingLeft(padding)`
 - Arg: `number padding` Padding in pixels
---
### SetContentPaddingTop
Set the top content padding.

Sig: `Window:SetContentPaddingTop(padding)`
 - Arg: `number padding` Padding in pixels
---
### SetContentPaddingRight
Set the right content padding.

Sig: `Window:SetContentPaddingRight(padding)`
 - Arg: `number padding` Padding in pixels
---
### SetContentPaddingBottom
Set the bottom content padding.

Sig: `Window:SetContentPaddingBottom(padding)`
 - Arg: `number padding` Padding in pixels
---
### GetContentPaddingLeft
Get the left content padding.

Sig: `padding = Window:GetContentPaddingLeft()`
 - Ret: `number padding` Padding in pixels
---
### GetContentPaddingTop
Get the top content padding.

Sig: `padding = Window:GetContentPaddingTop()`
 - Ret: `number padding` Padding in pixels
---
### GetContentPaddingRight
Get the right content padding.

Sig: `padding = Window:GetContentPaddingRight()`
 - Ret: `number padding` Padding in pixels
---
### GetContentPaddingBottom
Get the bottom content padding.

Sig: `padding = Window:GetContentPaddingBottom()`
 - Ret: `number padding` Padding in pixels
---
### Show
Show the window and emit Show signal.

Sig: `Window:Show()`
---
### Hide
Hide the window and emit Hide signal.

Sig: `Window:Hide()`
---
### Close
Close the window (emits Close signal then hides).

Sig: `Window:Close()`
---
### SetStartHidden
Set whether the window should be hidden when play mode starts.

Sig: `Window:SetStartHidden(hidden)`
 - Arg: `boolean hidden` Hide on start
---
### GetStartHidden
Check if window will be hidden on start.

Sig: `hidden = Window:GetStartHidden()`
 - Ret: `boolean hidden` Start hidden state
---
### SetDraggable
Enable or disable dragging by title bar.

Sig: `Window:SetDraggable(draggable)`
 - Arg: `boolean draggable` Enable dragging
---
### IsDraggable
Check if window is draggable.

Sig: `draggable = Window:IsDraggable()`
 - Ret: `boolean draggable` Draggable state
---
### SetResizable
Enable or disable the resize handle.

Sig: `Window:SetResizable(resizable)`
 - Arg: `boolean resizable` Enable resizing
---
### IsResizable
Check if window is resizable.

Sig: `resizable = Window:IsResizable()`
 - Ret: `boolean resizable` Resizable state
---
### SetShowCloseButton
Show or hide the close button.

Sig: `Window:SetShowCloseButton(show)`
 - Arg: `boolean show` Show close button
---
### GetShowCloseButton
Check if close button is shown.

Sig: `show = Window:GetShowCloseButton()`
 - Ret: `boolean show` Close button visible
---
### SetTitleBarHeight
Set the height of the title bar.

Sig: `Window:SetTitleBarHeight(height)`
 - Arg: `number height` Title bar height in pixels
---
### GetTitleBarHeight
Get the title bar height.

Sig: `height = Window:GetTitleBarHeight()`
 - Ret: `number height` Title bar height
---
### SetMinSize
Set the minimum window size.

Sig: `Window:SetMinSize(size)`
 - Arg: `Vector size` Minimum width and height
---
### GetMinSize
Get the minimum window size.

Sig: `size = Window:GetMinSize()`
 - Ret: `Vector size` Minimum size
---
### SetMaxSize
Set the maximum window size. Use (0, 0) for no limit.

Sig: `Window:SetMaxSize(size)`
 - Arg: `Vector size` Maximum width and height
---
### GetMaxSize
Get the maximum window size.

Sig: `size = Window:GetMaxSize()`
 - Ret: `Vector size` Maximum size
---
### SetResizeHandleSize
Set the size of the resize handle.

Sig: `Window:SetResizeHandleSize(size)`
 - Arg: `number size` Handle size in pixels
---
### GetResizeHandleSize
Get the resize handle size.

Sig: `size = Window:GetResizeHandleSize()`
 - Ret: `number size` Handle size
---
### SetContentWidget
Set the content widget for the window.

Sig: `Window:SetContentWidget(widget)`
 - Arg: `Widget widget` Widget to add as content
---
### GetContentContainer
Get the internal ScrollContainer that holds content.

Sig: `container = Window:GetContentContainer()`
 - Ret: `ScrollContainer container` Content container
---
### GetContentWidget
Get the first user widget in the content area.

Sig: `widget = Window:GetContentWidget()`
 - Ret: `Widget widget` Content widget or nil
---
### SetTitleBarColor
Set the title bar background color.

Sig: `Window:SetTitleBarColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetTitleBarColor
Get the title bar color.

Sig: `color = Window:GetTitleBarColor()`
 - Ret: `Vector color` RGBA color
---
### SetBackgroundColor
Set the window background color.

Sig: `Window:SetBackgroundColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetBackgroundColor
Get the background color.

Sig: `color = Window:GetBackgroundColor()`
 - Ret: `Vector color` RGBA color
---
### SetBackgroundTexture
Set the window background texture.

Sig: `Window:SetBackgroundTexture(texture)`
 - Arg: `Texture texture` Background texture (or nil to clear)
---
### GetBackgroundTexture
Get the background texture.

Sig: `texture = Window:GetBackgroundTexture()`
 - Ret: `Texture texture` Background texture or nil
---
### SetTitleBarTexture
Set the title bar background texture.

Sig: `Window:SetTitleBarTexture(texture)`
 - Arg: `Texture texture` Title bar texture (or nil to clear)
---
### GetTitleBarTexture
Get the title bar texture.

Sig: `texture = Window:GetTitleBarTexture()`
 - Ret: `Texture texture` Title bar texture or nil
---
### SetCloseButtonTexture
Set the close button texture.

Sig: `Window:SetCloseButtonTexture(texture)`
 - Arg: `Texture texture` Close button texture (or nil to clear)
---
### GetCloseButtonTexture
Get the close button texture.

Sig: `texture = Window:GetCloseButtonTexture()`
 - Ret: `Texture texture` Close button texture or nil
---
### SetCloseButtonNormalColor
Set the close button normal color.

Sig: `Window:SetCloseButtonNormalColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetCloseButtonNormalColor
Get the close button normal color.

Sig: `color = Window:GetCloseButtonNormalColor()`
 - Ret: `Vector color` RGBA color
---
### SetCloseButtonHoveredColor
Set the close button hovered color.

Sig: `Window:SetCloseButtonHoveredColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetCloseButtonHoveredColor
Get the close button hovered color.

Sig: `color = Window:GetCloseButtonHoveredColor()`
 - Ret: `Vector color` RGBA color
---
### SetCloseButtonPressedColor
Set the close button pressed color.

Sig: `Window:SetCloseButtonPressedColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetCloseButtonPressedColor
Get the close button pressed color.

Sig: `color = Window:GetCloseButtonPressedColor()`
 - Ret: `Vector color` RGBA color
---
### GetTitleBar
Get the internal title bar Canvas widget.

Sig: `titleBar = Window:GetTitleBar()`
 - Ret: `Canvas titleBar` Title bar widget
---
### GetTitleText
Get the internal title Text widget.

Sig: `text = Window:GetTitleText()`
 - Ret: `Text text` Title text widget
---
### GetCloseButton
Get the internal close Button widget.

Sig: `button = Window:GetCloseButton()`
 - Ret: `Button button` Close button widget
---
### GetResizeHandle
Get the internal resize handle Quad widget.

Sig: `handle = Window:GetResizeHandle()`
 - Ret: `Quad handle` Resize handle widget
---
### GetBackground
Get the internal background Quad widget.

Sig: `bg = Window:GetBackground()`
 - Ret: `Quad bg` Background widget

## Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| Show | self | Emitted when window is shown |
| Hide | self | Emitted when window is hidden |
| Close | self | Emitted when close is requested |
| DragStart | self | Emitted when drag begins |
| DragEnd | self | Emitted when drag ends |
| ResizeStart | self | Emitted when resize begins |
| ResizeEnd | self | Emitted when resize ends |
| Resized | self | Emitted during resize |

## Script Callbacks

| Callback | Parameters | Description |
|----------|------------|-------------|
| OnShow | self | Called when window is shown |
| OnHide | self | Called when window is hidden |
| OnClose | self | Called when close is requested |
| OnDragStart | self | Called when drag starts |
| OnDragEnd | self | Called when drag ends |
| OnResizeStart | self | Called when resize starts |
| OnResizeEnd | self | Called when resize ends |
| OnResized | self | Called during resize |

## Example

```lua
-- Create a window
local window = Node.Create("Window")
window:SetWindowId("inventory")
window:SetTitle("Inventory")
window:SetDimensions(300, 400)
window:SetPosition(100, 100)

-- Add content using SetContentWidget
local content = Node.Create("ArrayWidget")
content:SetOrientation(0) -- Vertical
window:SetContentWidget(content)

for i = 1, 10 do
    local item = Node.Create("Button")
    item:SetDimensions(280, 40)
    item:SetTextString("Item " .. i)
    content:AddChild(item)
end

-- Handle close
window.OnClose = function(self)
    print("Closing inventory")
end

-- Control via WindowManager
WindowManager.ShowWindow("inventory")
```
