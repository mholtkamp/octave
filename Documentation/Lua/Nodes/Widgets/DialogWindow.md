# DialogWindow

A dialog window widget that extends Window with confirm and reject buttons at the bottom. Useful for confirmation dialogs, input prompts, and other user interactions requiring explicit user decisions.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)
* [Window](Window.md)

---
### SetConfirmText
Set the confirm button text.

Sig: `DialogWindow:SetConfirmText(text)`
 - Arg: `string text` Button text
---
### GetConfirmText
Get the confirm button text.

Sig: `text = DialogWindow:GetConfirmText()`
 - Ret: `string text` Button text
---
### SetConfirmNormalColor
Set the confirm button normal color.

Sig: `DialogWindow:SetConfirmNormalColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetConfirmNormalColor
Get the confirm button normal color.

Sig: `color = DialogWindow:GetConfirmNormalColor()`
 - Ret: `Vector color` RGBA color
---
### SetConfirmHoveredColor
Set the confirm button hovered color.

Sig: `DialogWindow:SetConfirmHoveredColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetConfirmHoveredColor
Get the confirm button hovered color.

Sig: `color = DialogWindow:GetConfirmHoveredColor()`
 - Ret: `Vector color` RGBA color
---
### SetConfirmPressedColor
Set the confirm button pressed color.

Sig: `DialogWindow:SetConfirmPressedColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetConfirmPressedColor
Get the confirm button pressed color.

Sig: `color = DialogWindow:GetConfirmPressedColor()`
 - Ret: `Vector color` RGBA color
---
### SetConfirmTexture
Set the confirm button texture.

Sig: `DialogWindow:SetConfirmTexture(texture)`
 - Arg: `Texture texture` Button texture (or nil to clear)
---
### GetConfirmTexture
Get the confirm button texture.

Sig: `texture = DialogWindow:GetConfirmTexture()`
 - Ret: `Texture texture` Button texture or nil
---
### SetShowConfirmButton
Show or hide the confirm button.

Sig: `DialogWindow:SetShowConfirmButton(show)`
 - Arg: `boolean show` Show confirm button
---
### GetShowConfirmButton
Check if confirm button is shown.

Sig: `show = DialogWindow:GetShowConfirmButton()`
 - Ret: `boolean show` Confirm button visible
---
### SetRejectText
Set the reject button text.

Sig: `DialogWindow:SetRejectText(text)`
 - Arg: `string text` Button text
---
### GetRejectText
Get the reject button text.

Sig: `text = DialogWindow:GetRejectText()`
 - Ret: `string text` Button text
---
### SetRejectNormalColor
Set the reject button normal color.

Sig: `DialogWindow:SetRejectNormalColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetRejectNormalColor
Get the reject button normal color.

Sig: `color = DialogWindow:GetRejectNormalColor()`
 - Ret: `Vector color` RGBA color
---
### SetRejectHoveredColor
Set the reject button hovered color.

Sig: `DialogWindow:SetRejectHoveredColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetRejectHoveredColor
Get the reject button hovered color.

Sig: `color = DialogWindow:GetRejectHoveredColor()`
 - Ret: `Vector color` RGBA color
---
### SetRejectPressedColor
Set the reject button pressed color.

Sig: `DialogWindow:SetRejectPressedColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetRejectPressedColor
Get the reject button pressed color.

Sig: `color = DialogWindow:GetRejectPressedColor()`
 - Ret: `Vector color` RGBA color
---
### SetRejectTexture
Set the reject button texture.

Sig: `DialogWindow:SetRejectTexture(texture)`
 - Arg: `Texture texture` Button texture (or nil to clear)
---
### GetRejectTexture
Get the reject button texture.

Sig: `texture = DialogWindow:GetRejectTexture()`
 - Ret: `Texture texture` Button texture or nil
---
### SetShowRejectButton
Show or hide the reject button.

Sig: `DialogWindow:SetShowRejectButton(show)`
 - Arg: `boolean show` Show reject button
---
### GetShowRejectButton
Check if reject button is shown.

Sig: `show = DialogWindow:GetShowRejectButton()`
 - Ret: `boolean show` Reject button visible
---
### SetButtonBarHeight
Set the height of the button bar.

Sig: `DialogWindow:SetButtonBarHeight(height)`
 - Arg: `number height` Height in pixels
---
### GetButtonBarHeight
Get the button bar height.

Sig: `height = DialogWindow:GetButtonBarHeight()`
 - Ret: `number height` Height in pixels
---
### SetButtonSpacing
Set the spacing between buttons.

Sig: `DialogWindow:SetButtonSpacing(spacing)`
 - Arg: `number spacing` Spacing in pixels
---
### GetButtonSpacing
Get the button spacing.

Sig: `spacing = DialogWindow:GetButtonSpacing()`
 - Ret: `number spacing` Spacing in pixels
---
### SetButtonBarAlignment
Set the button alignment within the bar.

ButtonBarAlignment values:
- 0 = Left
- 1 = Center
- 2 = Right
- 3 = Spread

Sig: `DialogWindow:SetButtonBarAlignment(alignment)`
 - Arg: `integer alignment` Alignment (0-3)
---
### GetButtonBarAlignment
Get the button bar alignment.

Sig: `alignment = DialogWindow:GetButtonBarAlignment()`
 - Ret: `integer alignment` Alignment
---
### SetButtonBarColor
Set the button bar background color.

Sig: `DialogWindow:SetButtonBarColor(color)`
 - Arg: `Vector color` RGBA color
---
### GetButtonBarColor
Get the button bar color.

Sig: `color = DialogWindow:GetButtonBarColor()`
 - Ret: `Vector color` RGBA color
---
### SetButtonWidth
Set the width of each button.

Sig: `DialogWindow:SetButtonWidth(width)`
 - Arg: `number width` Width in pixels
---
### GetButtonWidth
Get the button width.

Sig: `width = DialogWindow:GetButtonWidth()`
 - Ret: `number width` Width in pixels
---
### SetButtonBarPadding
Set the padding inside the button bar.

Sig: `DialogWindow:SetButtonBarPadding(padding)`
 - Arg: `number padding` Padding in pixels
---
### GetButtonBarPadding
Get the button bar padding.

Sig: `padding = DialogWindow:GetButtonBarPadding()`
 - Ret: `number padding` Padding in pixels
---
### Confirm
Programmatically trigger confirm action (emits OnConfirm).

Sig: `DialogWindow:Confirm()`
---
### Reject
Programmatically trigger reject action (emits OnReject).

Sig: `DialogWindow:Reject()`
---
### GetButtonBar
Get the internal button bar Canvas widget.

Sig: `bar = DialogWindow:GetButtonBar()`
 - Ret: `Canvas bar` Button bar widget
---
### GetConfirmButton
Get the internal confirm Button widget.

Sig: `button = DialogWindow:GetConfirmButton()`
 - Ret: `Button button` Confirm button widget
---
### GetRejectButton
Get the internal reject Button widget.

Sig: `button = DialogWindow:GetRejectButton()`
 - Ret: `Button button` Reject button widget

## Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| Confirm | self | Emitted when confirm button clicked |
| Reject | self | Emitted when reject button clicked |

## Script Callbacks

| Callback | Parameters | Description |
|----------|------------|-------------|
| OnConfirm | self | Called when confirm is triggered |
| OnReject | self | Called when reject is triggered |

## Example

```lua
-- Create a confirmation dialog
local dialog = Node.Create("DialogWindow")
dialog:SetWindowId("confirm_delete")
dialog:SetTitle("Delete Item?")
dialog:SetDimensions(300, 200)
dialog:SetPosition(200, 150)

-- Customize buttons
dialog:SetConfirmText("Delete")
dialog:SetRejectText("Keep")
dialog:SetButtonBarAlignment(2) -- Right aligned
dialog:SetConfirmNormalColor(Vector.New(0.8, 0.2, 0.2, 1.0))

-- Add content
local message = Node.Create("Text")
message:SetTextString("Are you sure you want to delete this item?")
message:SetDimensions(280, 100)
dialog:SetContentWidget(message)

-- Handle responses
function dialog:OnConfirm()
    print("Item deleted!")
    self:Close()
end

function dialog:OnReject()
    print("Cancelled")
    self:Close()
end

-- Show via WindowManager
WindowManager.ShowWindow("confirm_delete")
```
