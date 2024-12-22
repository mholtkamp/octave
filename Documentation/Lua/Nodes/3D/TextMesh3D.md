# TextMesh3D

A dynamic mesh that can render text strings using a Font asset.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)
* [Mesh3D](Mesh3D.md)

---

### SetText
Set the text string.

Sig: `TextMesh3D:SetText(text)`
 - Arg: `string text` Text string to render
---
### GetText
Get the text string to be rendered.

Sig: `text = TextMesh3D:GetText()`
 - Ret: `string text` Text string to render
---
### SetFont
Set the Font asset to render.

Sig: `TextMesh3D:SetFont(font)`
 - Arg: `Font font` Font asset
---
### GetFont
Get the assigned Font asset.

Sig: `font = TextMesh3D:GetFont()`
 - Ret: `Font font` Font asset
---
### SetColor
Set the text color.

Sig: `TextMesh3D:SetColor(color)`
 - Arg: `Vector color` Text color
---
### GetColor
Get the text color.

Sig: `color = TextMesh3D:GetColor()`
 - Ret: `Vector color` Text color
---
### SetBlendMode
Set the blend mode.

See [BlendMode](../../Misc/Enums.md#blendmode)

Sig: `TextMesh3D:SetBlendMode(blendMode)`
 - Arg: `BlendMode(integer) blendMode` Blending mode
---
### GetBlendMode
Get the blend mode.

See [BlendMode](../../Misc/Enums.md#blendmode)

Sig: `blendMode = TextMesh3D:GetBlendMode()`
 - Ret: `BlendMode(integer) blendMode` Blending mode
---
### SetHorizontalJustification
Set the horizontal text justification. Justification should be between 0 and 1 (inclusive). 0.5 justification would be middle-justified.

Sig: `TextMesh3D:SetHorizontalJustification(justification)`
 - Arg: `number justification` Text justification

---
### GetHorizontalJustification
Get the horizontal text justification. Justification should be between 0 and 1 (inclusive). 0.5 justification would be middle-justified.

Sig: `justification = TextMesh3D:GetHorizontalJustification()`
 - Ret: `number justification` Text justification
---
### SetVerticalJustification
Set the vertical text justification. Justification should be between 0 and 1 (inclusive). 0.5 justification would be middle-justified.

Sig: `TextMesh3D:SetVerticalJustification(justification)`
 - Arg: `number justification` Text justification
---
### GetVerticalJustification
Get the vertical text justification. Justification should be between 0 and 1 (inclusive). 0.5 justification would be middle-justified.

Sig: `justification = TextMesh3D:GetVerticalJustification()`
 - Ret: `number justification` Text justification
---