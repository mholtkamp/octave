# CheckBox

A toggle checkbox widget with a box indicator and text label.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetChecked
Set the checked state.

Sig: `CheckBox:SetChecked(checked)`
 - Arg: `boolean checked` Checked state
---
### IsChecked
Get the checked state.

Sig: `checked = CheckBox:IsChecked()`
 - Ret: `boolean checked` True if checked
---
### Toggle
Toggle the checked state.

Sig: `CheckBox:Toggle()`

---
### SetText
Set the label text.

Sig: `CheckBox:SetText(text)`
 - Arg: `string text` Label text
---
### GetText
Get the label text.

Sig: `text = CheckBox:GetText()`
 - Ret: `string text` Label text
---
### SetCheckedColor
Set the color when checked.

Sig: `CheckBox:SetCheckedColor(color)`
 - Arg: `Vector color` Checked color
---
### GetCheckedColor
Get the color when checked.

Sig: `color = CheckBox:GetCheckedColor()`
 - Ret: `Vector color` Checked color
---
### SetUncheckedColor
Set the color when unchecked.

Sig: `CheckBox:SetUncheckedColor(color)`
 - Arg: `Vector color` Unchecked color
---
### GetUncheckedColor
Get the color when unchecked.

Sig: `color = CheckBox:GetUncheckedColor()`
 - Ret: `Vector color` Unchecked color
---
### SetTextColor
Set the text color.

Sig: `CheckBox:SetTextColor(color)`
 - Arg: `Vector color` Text color
---
### GetTextColor
Get the text color.

Sig: `color = CheckBox:GetTextColor()`
 - Ret: `Vector color` Text color
---
### SetBoxSize
Set the size of the checkbox box.

Sig: `CheckBox:SetBoxSize(size)`
 - Arg: `number size` Box size in pixels
---
### GetBoxSize
Get the size of the checkbox box.

Sig: `size = CheckBox:GetBoxSize()`
 - Ret: `number size` Box size in pixels
---
### SetSpacing
Set the spacing between the box and text.

Sig: `CheckBox:SetSpacing(spacing)`
 - Arg: `number spacing` Spacing in pixels
---
### GetSpacing
Get the spacing between the box and text.

Sig: `spacing = CheckBox:GetSpacing()`
 - Ret: `number spacing` Spacing in pixels
---
### GetBoxQuad
Get the internal box Quad widget.

Sig: `quad = CheckBox:GetBoxQuad()`
 - Ret: `Quad quad` Box quad
---
### GetTextWidget
Get the internal Text widget.

Sig: `text = CheckBox:GetTextWidget()`
 - Ret: `Text text` Text widget
---
