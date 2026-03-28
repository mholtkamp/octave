# LineEdit

A compound widget combining a title label with an InputField. Useful for labeled form inputs.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetTitle
Set the title/label text.

Sig: `LineEdit:SetTitle(title)`
 - Arg: `string title` Title text
---
### GetTitle
Get the title/label text.

Sig: `title = LineEdit:GetTitle()`
 - Ret: `string title` Title text
---
### SetTitleWidth
Set the width allocated for the title.

Sig: `LineEdit:SetTitleWidth(width)`
 - Arg: `number width` Title width in pixels
---
### GetTitleWidth
Get the width allocated for the title.

Sig: `width = LineEdit:GetTitleWidth()`
 - Ret: `number width` Title width in pixels
---
### SetText
Set the input field's text content.

Sig: `LineEdit:SetText(text)`
 - Arg: `string text` Text content
---
### GetText
Get the input field's text content.

Sig: `text = LineEdit:GetText()`
 - Ret: `string text` Text content
---
### SetPlaceholder
Set the placeholder text for the input field.

Sig: `LineEdit:SetPlaceholder(placeholder)`
 - Arg: `string placeholder` Placeholder text
---
### GetPlaceholder
Get the placeholder text.

Sig: `placeholder = LineEdit:GetPlaceholder()`
 - Ret: `string placeholder` Placeholder text
---
### SetCaretPosition
Set the caret position in the input field.

Sig: `LineEdit:SetCaretPosition(pos)`
 - Arg: `integer pos` Caret position
---
### GetCaretPosition
Get the caret position.

Sig: `pos = LineEdit:GetCaretPosition()`
 - Ret: `integer pos` Caret position
---
### SelectAll
Select all text in the input field.

Sig: `LineEdit:SelectAll()`

---
### ClearSelection
Clear the current selection.

Sig: `LineEdit:ClearSelection()`

---
### HasSelection
Check if there is a text selection.

Sig: `hasSelection = LineEdit:HasSelection()`
 - Ret: `boolean hasSelection` True if text is selected
---
### GetSelectedText
Get the currently selected text.

Sig: `text = LineEdit:GetSelectedText()`
 - Ret: `string text` Selected text
---
### DeleteSelection
Delete the currently selected text.

Sig: `LineEdit:DeleteSelection()`

---
### SetFocused
Set whether the input field is focused.

Sig: `LineEdit:SetFocused(focused)`
 - Arg: `boolean focused` Focus state
---
### IsFocused
Check if the input field is focused.

Sig: `focused = LineEdit:IsFocused()`
 - Ret: `boolean focused` True if focused
---
### SetPasswordMode
Enable or disable password mode.

Sig: `LineEdit:SetPasswordMode(enabled)`
 - Arg: `boolean enabled` Enable password mode
---
### IsPasswordMode
Check if password mode is enabled.

Sig: `enabled = LineEdit:IsPasswordMode()`
 - Ret: `boolean enabled` True if password mode is enabled
---
### SetMaxLength
Set the maximum text length.

Sig: `LineEdit:SetMaxLength(maxLen)`
 - Arg: `integer maxLen` Maximum length (0 = no limit)
---
### GetMaxLength
Get the maximum text length.

Sig: `maxLen = LineEdit:GetMaxLength()`
 - Ret: `integer maxLen` Maximum length
---
### SetEditable
Set whether the field is editable.

Sig: `LineEdit:SetEditable(editable)`
 - Arg: `boolean editable` Editable state
---
### IsEditable
Check if the field is editable.

Sig: `editable = LineEdit:IsEditable()`
 - Ret: `boolean editable` True if editable
---
### GetTitleWidget
Get the internal title Text widget.

Sig: `text = LineEdit:GetTitleWidget()`
 - Ret: `Text text` Title widget
---
### GetInputField
Get the internal InputField widget.

Sig: `inputField = LineEdit:GetInputField()`
 - Ret: `InputField inputField` InputField widget
---
