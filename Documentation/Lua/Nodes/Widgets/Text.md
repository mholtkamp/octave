# Text 

A widget that can render a text string.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetFont
Set the font asset.

Sig: `Text:SetFont(font)`
 - Arg: `Font font` Font asset
---
### GetFont
Get the font asset.

Sig: `font = Text:GetFont()`
 - Ret: `Font font` Font asset
---
### SetTextSize
Set the text size.

Sig: `Text:SetTextSize(size)`
 - Arg: `number size` Text size
---
### GetTextSize
Get the text size.

Sig: `size = Text:GetTextSize()`
 - Ret: `number size` Text size
---
### GetScaledTextSize
Get the scaled text size (as a result of the Widget scale).

Sig: `size = Text:GetScaledTextSize()`
 - Ret: `number size` Scaled text size
---
### SetText
Set the text string.

Sig: `Text:SetText(text)`
 - Arg: `string text` Text string
---
### GetText
Get the text string.

Sig: `text = Text:GetText()`
 - Ret: `string text` Text string
---
### GetTextWidth
Get the width in screen pixels of the constructed font vertices. Useful for alignment and checking the actual area of the text (versus the widget Rect).

Sig: `width = Text:GetTextWidth()`
 - Ret: `number` Text width in pixels
---
### GetTextHeight
Get the height in screen pixels of the constructed font vertices. Useful for alignment and checking the actual area of the text (versus the widget Rect).

Sig: `height = Text:GetTextHeight()`
 - Ret: `number` Text height in pixels
---
### GetScaledMinExtent
Get the scaled minimum extent of text vertices. NOTE: Does not handle text justification correctly yet.

Sig: `minExt = Text:GetScaledMinExtent()`
 - Ret: `Vector minExt` Minimum extent in screen space
---
### GetScaledMaxExtent
Get the scaled maximum extent of text vertices. NOTE: Does not handle text justification correctly yet.

Sig: `maxExt = Text:GetScaledMaxExtent()`
 - Ret: `Vector maxExt` Maximum extent in screen space
---
### SetHorizontalJustification
Set the horizontal justification.

See [Justification](../../Misc/Enums.md#justification)

Sig: `Text:SetHorizontalJustification(just)`
 - Arg: `Justification(integer) just` Text justification
---
### GetHorizontalJustification
Get the horizontal justification.

See [Justification](../../Misc/Enums.md#justification)

Sig: `just = Text:GetHorizontalJustification()`
 - Ret: `Justification(integer) just` Text justification
---
### SetVerticalJustification
Set the vertical justification.

See [Justification](../../Misc/Enums.md#justification)

Sig: `Text:SetVerticalJustification(just)`
 - Arg: `Justification(integer) just` Text justification
---
### GetVerticalJustification
Get the vertical justification.

See [Justification](../../Misc/Enums.md#justification)

Sig: `just = Text:GetVerticalJustification()`
 - Ret: `Justification(integer) just` Text justification
---
### EnableWordWrap
Enable word wrap. Word wrap will put characters on a new line after they pass the dimensions of this widget's rect.

Sig: `Text:EnableWordWrap(wrap)`
 - Arg: `boolean wrap` Word wrap
---
### IsWordWrapEnabled
Check if word wrap is enabled. Word wrap will put characters on a new line after they pass the dimensions of this widget's rect.

Sig: `wrap = Text:IsWordWrapEnabled()`
 - Ret: `boolean wrap` Word wrap
---
