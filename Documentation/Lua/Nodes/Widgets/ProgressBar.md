# ProgressBar

A visual progress indicator widget showing a fill bar and optional percentage text.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetValue
Set the current value.

Sig: `ProgressBar:SetValue(value)`
 - Arg: `number value` Current value
---
### GetValue
Get the current value.

Sig: `value = ProgressBar:GetValue()`
 - Ret: `number value` Current value
---
### SetMinValue
Set the minimum value.

Sig: `ProgressBar:SetMinValue(min)`
 - Arg: `number min` Minimum value
---
### GetMinValue
Get the minimum value.

Sig: `min = ProgressBar:GetMinValue()`
 - Ret: `number min` Minimum value
---
### SetMaxValue
Set the maximum value.

Sig: `ProgressBar:SetMaxValue(max)`
 - Arg: `number max` Maximum value
---
### GetMaxValue
Get the maximum value.

Sig: `max = ProgressBar:GetMaxValue()`
 - Ret: `number max` Maximum value
---
### GetRatio
Get the current value as a ratio between 0 and 1.

Sig: `ratio = ProgressBar:GetRatio()`
 - Ret: `number ratio` Value ratio (0 to 1)
---
### SetShowPercentage
Set whether to display percentage text.

Sig: `ProgressBar:SetShowPercentage(show)`
 - Arg: `boolean show` Show percentage
---
### IsShowingPercentage
Check if percentage text is being displayed.

Sig: `showing = ProgressBar:IsShowingPercentage()`
 - Ret: `boolean showing` True if showing percentage
---
### SetBackgroundColor
Set the background color.

Sig: `ProgressBar:SetBackgroundColor(color)`
 - Arg: `Vector color` Background color
---
### GetBackgroundColor
Get the background color.

Sig: `color = ProgressBar:GetBackgroundColor()`
 - Ret: `Vector color` Background color
---
### SetFillColor
Set the fill bar color.

Sig: `ProgressBar:SetFillColor(color)`
 - Arg: `Vector color` Fill color
---
### GetFillColor
Get the fill bar color.

Sig: `color = ProgressBar:GetFillColor()`
 - Ret: `Vector color` Fill color
---
### SetTextColor
Set the percentage text color.

Sig: `ProgressBar:SetTextColor(color)`
 - Arg: `Vector color` Text color
---
### GetTextColor
Get the percentage text color.

Sig: `color = ProgressBar:GetTextColor()`
 - Ret: `Vector color` Text color
---
### GetBackgroundQuad
Get the internal background Quad widget.

Sig: `quad = ProgressBar:GetBackgroundQuad()`
 - Ret: `Quad quad` Background quad
---
### GetFillQuad
Get the internal fill Quad widget.

Sig: `quad = ProgressBar:GetFillQuad()`
 - Ret: `Quad quad` Fill quad
---
### GetTextWidget
Get the internal percentage Text widget.

Sig: `text = ProgressBar:GetTextWidget()`
 - Ret: `Text text` Text widget
---
