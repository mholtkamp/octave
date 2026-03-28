# InputField

A text input widget that allows users to type and edit text. Supports selection, clipboard operations (copy/paste/cut), password mode, and caret navigation.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetText
Set the text content.

Sig: `InputField:SetText(text)`
 - Arg: `string text` Text content
---
### GetText
Get the text content.

Sig: `text = InputField:GetText()`
 - Ret: `string text` Text content
---
### SetPlaceholder
Set the placeholder text shown when the field is empty.

Sig: `InputField:SetPlaceholder(placeholder)`
 - Arg: `string placeholder` Placeholder text
---
### GetPlaceholder
Get the placeholder text.

Sig: `placeholder = InputField:GetPlaceholder()`
 - Ret: `string placeholder` Placeholder text
---
### SetCaretPosition
Set the caret (cursor) position.

Sig: `InputField:SetCaretPosition(pos)`
 - Arg: `integer pos` Caret position (0 = before first character)
---
### GetCaretPosition
Get the caret position.

Sig: `pos = InputField:GetCaretPosition()`
 - Ret: `integer pos` Caret position
---
### SelectAll
Select all text in the field.

Sig: `InputField:SelectAll()`

---
### ClearSelection
Clear the current selection.

Sig: `InputField:ClearSelection()`

---
### HasSelection
Check if there is a text selection.

Sig: `hasSelection = InputField:HasSelection()`
 - Ret: `boolean hasSelection` True if text is selected
---
### GetSelectedText
Get the currently selected text.

Sig: `text = InputField:GetSelectedText()`
 - Ret: `string text` Selected text
---
### DeleteSelection
Delete the currently selected text.

Sig: `InputField:DeleteSelection()`

---
### Select
Select a range of text.

Sig: `InputField:Select(start, end)`
 - Arg: `integer start` Selection start position
 - Arg: `integer end` Selection end position
---
### GetSelectionStart
Get the selection start position.

Sig: `start = InputField:GetSelectionStart()`
 - Ret: `integer start` Selection start (-1 if no selection)
---
### GetSelectionEnd
Get the selection end position.

Sig: `end = InputField:GetSelectionEnd()`
 - Ret: `integer end` Selection end (-1 if no selection)
---
### SetFocused
Set whether the input field is focused.

Sig: `InputField:SetFocused(focused)`
 - Arg: `boolean focused` Focus state
---
### IsFocused
Check if the input field is focused.

Sig: `focused = InputField:IsFocused()`
 - Ret: `boolean focused` True if focused
---
### SetPasswordMode
Enable or disable password mode (text is displayed as asterisks).

Sig: `InputField:SetPasswordMode(enabled)`
 - Arg: `boolean enabled` Enable password mode
---
### IsPasswordMode
Check if password mode is enabled.

Sig: `enabled = InputField:IsPasswordMode()`
 - Ret: `boolean enabled` True if password mode is enabled
---
### SetMaxLength
Set the maximum text length (0 = no limit).

Sig: `InputField:SetMaxLength(maxLen)`
 - Arg: `integer maxLen` Maximum length
---
### GetMaxLength
Get the maximum text length.

Sig: `maxLen = InputField:GetMaxLength()`
 - Ret: `integer maxLen` Maximum length
---
### SetEditable
Set whether the field is editable.

Sig: `InputField:SetEditable(editable)`
 - Arg: `boolean editable` Editable state
---
### IsEditable
Check if the field is editable.

Sig: `editable = InputField:IsEditable()`
 - Ret: `boolean editable` True if editable
---
### SetBackgroundColor
Set the background color.

Sig: `InputField:SetBackgroundColor(color)`
 - Arg: `Vector color` Background color
---
### GetBackgroundColor
Get the background color.

Sig: `color = InputField:GetBackgroundColor()`
 - Ret: `Vector color` Background color
---
### SetTextColor
Set the text color.

Sig: `InputField:SetTextColor(color)`
 - Arg: `Vector color` Text color
---
### GetTextColor
Get the text color.

Sig: `color = InputField:GetTextColor()`
 - Ret: `Vector color` Text color
---
### GetBackground
Get the internal background Quad widget.

Sig: `quad = InputField:GetBackground()`
 - Ret: `Quad quad` Background quad
---
### GetTextWidget
Get the internal Text widget.

Sig: `text = InputField:GetTextWidget()`
 - Ret: `Text text` Text widget
---
### GetCaret
Get the internal caret Quad widget.

Sig: `quad = InputField:GetCaret()`
 - Ret: `Quad quad` Caret quad
---
