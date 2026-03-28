# ComboBox

A dropdown selection widget that displays a list of options when clicked.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### AddOption
Add an option to the dropdown list.

Sig: `ComboBox:AddOption(option)`
 - Arg: `string option` Option text to add
---
### RemoveOption
Remove an option by index.

Sig: `ComboBox:RemoveOption(index)`
 - Arg: `integer index` Index of option to remove
---
### ClearOptions
Remove all options.

Sig: `ComboBox:ClearOptions()`

---
### SetOptions
Set all options at once from a table.

Sig: `ComboBox:SetOptions(options)`
 - Arg: `table options` Array of option strings
---
### GetOptions
Get all options as a table.

Sig: `options = ComboBox:GetOptions()`
 - Ret: `table options` Array of option strings
---
### GetOptionCount
Get the number of options.

Sig: `count = ComboBox:GetOptionCount()`
 - Ret: `integer count` Number of options
---
### SetSelectedIndex
Set the selected option index.

Sig: `ComboBox:SetSelectedIndex(index)`
 - Arg: `integer index` Index to select (-1 for none)
---
### GetSelectedIndex
Get the selected option index.

Sig: `index = ComboBox:GetSelectedIndex()`
 - Ret: `integer index` Selected index (-1 if none)
---
### GetSelectedOption
Get the selected option text.

Sig: `option = ComboBox:GetSelectedOption()`
 - Ret: `string option` Selected option text (empty if none)
---
### IsOpen
Check if the dropdown is currently open.

Sig: `open = ComboBox:IsOpen()`
 - Ret: `boolean open` True if dropdown is open
---
### Open
Open the dropdown.

Sig: `ComboBox:Open()`

---
### Close
Close the dropdown.

Sig: `ComboBox:Close()`

---
### Toggle
Toggle the dropdown open/closed state.

Sig: `ComboBox:Toggle()`

---
### SetBackgroundColor
Set the background color.

Sig: `ComboBox:SetBackgroundColor(color)`
 - Arg: `Vector color` Background color
---
### GetBackgroundColor
Get the background color.

Sig: `color = ComboBox:GetBackgroundColor()`
 - Ret: `Vector color` Background color
---
### SetTextColor
Set the text color.

Sig: `ComboBox:SetTextColor(color)`
 - Arg: `Vector color` Text color
---
### GetTextColor
Get the text color.

Sig: `color = ComboBox:GetTextColor()`
 - Ret: `Vector color` Text color
---
### SetDropdownColor
Set the dropdown background color.

Sig: `ComboBox:SetDropdownColor(color)`
 - Arg: `Vector color` Dropdown color
---
### GetDropdownColor
Get the dropdown background color.

Sig: `color = ComboBox:GetDropdownColor()`
 - Ret: `Vector color` Dropdown color
---
### SetHoveredColor
Set the hovered item color.

Sig: `ComboBox:SetHoveredColor(color)`
 - Arg: `Vector color` Hovered color
---
### GetHoveredColor
Get the hovered item color.

Sig: `color = ComboBox:GetHoveredColor()`
 - Ret: `Vector color` Hovered color
---
### SetMaxVisibleItems
Set the maximum number of visible items in the dropdown.

Sig: `ComboBox:SetMaxVisibleItems(count)`
 - Arg: `integer count` Maximum visible items
---
### GetMaxVisibleItems
Get the maximum number of visible items.

Sig: `count = ComboBox:GetMaxVisibleItems()`
 - Ret: `integer count` Maximum visible items
---
### GetBackground
Get the internal background Quad widget.

Sig: `quad = ComboBox:GetBackground()`
 - Ret: `Quad quad` Background quad
---
### GetTextWidget
Get the internal Text widget showing the selection.

Sig: `text = ComboBox:GetTextWidget()`
 - Ret: `Text text` Text widget
---
