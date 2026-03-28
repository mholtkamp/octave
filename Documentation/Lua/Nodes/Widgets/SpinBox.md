# SpinBox

A numeric input widget with increment/decrement buttons. Supports min/max values, step amount, and prefix/suffix display.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetValue
Set the current value.

Sig: `SpinBox:SetValue(value)`
 - Arg: `number value` Current value
---
### GetValue
Get the current value.

Sig: `value = SpinBox:GetValue()`
 - Ret: `number value` Current value
---
### SetMinValue
Set the minimum value.

Sig: `SpinBox:SetMinValue(min)`
 - Arg: `number min` Minimum value
---
### GetMinValue
Get the minimum value.

Sig: `min = SpinBox:GetMinValue()`
 - Ret: `number min` Minimum value
---
### SetMaxValue
Set the maximum value.

Sig: `SpinBox:SetMaxValue(max)`
 - Arg: `number max` Maximum value
---
### GetMaxValue
Get the maximum value.

Sig: `max = SpinBox:GetMaxValue()`
 - Ret: `number max` Maximum value
---
### SetStep
Set the step amount for increment/decrement.

Sig: `SpinBox:SetStep(step)`
 - Arg: `number step` Step amount
---
### GetStep
Get the step amount.

Sig: `step = SpinBox:GetStep()`
 - Ret: `number step` Step amount
---
### SetPrefix
Set the prefix text displayed before the value.

Sig: `SpinBox:SetPrefix(prefix)`
 - Arg: `string prefix` Prefix text
---
### GetPrefix
Get the prefix text.

Sig: `prefix = SpinBox:GetPrefix()`
 - Ret: `string prefix` Prefix text
---
### SetSuffix
Set the suffix text displayed after the value.

Sig: `SpinBox:SetSuffix(suffix)`
 - Arg: `string suffix` Suffix text
---
### GetSuffix
Get the suffix text.

Sig: `suffix = SpinBox:GetSuffix()`
 - Ret: `string suffix` Suffix text
---
### SetBackgroundColor
Set the background color.

Sig: `SpinBox:SetBackgroundColor(color)`
 - Arg: `Vector color` Background color
---
### GetBackgroundColor
Get the background color.

Sig: `color = SpinBox:GetBackgroundColor()`
 - Ret: `Vector color` Background color
---
### SetTextColor
Set the value text color.

Sig: `SpinBox:SetTextColor(color)`
 - Arg: `Vector color` Text color
---
### GetTextColor
Get the value text color.

Sig: `color = SpinBox:GetTextColor()`
 - Ret: `Vector color` Text color
---
### SetButtonColor
Set the button color.

Sig: `SpinBox:SetButtonColor(color)`
 - Arg: `Vector color` Button color
---
### GetButtonColor
Get the button color.

Sig: `color = SpinBox:GetButtonColor()`
 - Ret: `Vector color` Button color
---
### GetBackground
Get the internal background Quad widget.

Sig: `quad = SpinBox:GetBackground()`
 - Ret: `Quad quad` Background quad
---
### GetTextWidget
Get the internal value Text widget.

Sig: `text = SpinBox:GetTextWidget()`
 - Ret: `Text text` Text widget
---
### GetIncrementButton
Get the internal increment Button widget.

Sig: `button = SpinBox:GetIncrementButton()`
 - Ret: `Button button` Increment button
---
### GetDecrementButton
Get the internal decrement Button widget.

Sig: `button = SpinBox:GetDecrementButton()`
 - Ret: `Button button` Decrement button
---
