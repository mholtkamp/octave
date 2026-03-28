# Slider

A draggable slider widget for selecting a value within a range. Composed internally of a background Quad and a draggable grabber Quad.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetValue
Set the slider's current value. The value will be clamped between min and max.

Sig: `Slider:SetValue(value)`
 - Arg: `number value` New value
---
### GetValue
Get the slider's current value.

Sig: `value = Slider:GetValue()`
 - Ret: `number value` Current value
---
### SetMinValue
Set the minimum value.

Sig: `Slider:SetMinValue(min)`
 - Arg: `number min` Minimum value
---
### GetMinValue
Get the minimum value.

Sig: `min = Slider:GetMinValue()`
 - Ret: `number min` Minimum value
---
### SetMaxValue
Set the maximum value.

Sig: `Slider:SetMaxValue(max)`
 - Arg: `number max` Maximum value
---
### GetMaxValue
Get the maximum value.

Sig: `max = Slider:GetMaxValue()`
 - Ret: `number max` Maximum value
---
### SetStep
Set the step amount. If step is 0, the slider is continuous.

Sig: `Slider:SetStep(step)`
 - Arg: `number step` Step amount (0 for continuous)
---
### GetStep
Get the step amount.

Sig: `step = Slider:GetStep()`
 - Ret: `number step` Step amount
---
### GetRatio
Get the current value as a ratio between 0 and 1.

Sig: `ratio = Slider:GetRatio()`
 - Ret: `number ratio` Value ratio (0 to 1)
---
### SetBackgroundColor
Set the background/track color.

Sig: `Slider:SetBackgroundColor(color)`
 - Arg: `Vector color` Background color
---
### GetBackgroundColor
Get the background/track color.

Sig: `color = Slider:GetBackgroundColor()`
 - Ret: `Vector color` Background color
---
### SetGrabberColor
Set the grabber color.

Sig: `Slider:SetGrabberColor(color)`
 - Arg: `Vector color` Grabber color
---
### GetGrabberColor
Get the grabber color.

Sig: `color = Slider:GetGrabberColor()`
 - Ret: `Vector color` Grabber color
---
### GetBackground
Get the internal background Quad widget.

Sig: `quad = Slider:GetBackground()`
 - Ret: `Quad quad` Background quad
---
### GetGrabber
Get the internal grabber Quad widget.

Sig: `quad = Slider:GetGrabber()`
 - Ret: `Quad quad` Grabber quad
---
