# Quad

A widget that renders a textured rectangle.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### SetTexture
Set the texture asset to use.

Sig: `Quad:SetTexture(texture)`
 - Arg: `Texture texture` Texture asset
---
### GetTexture
Get the assigned texture asset.

Sig: `texture = Quad:GetTexture()`
 - Ret: `Texture texture` Texture asset
---
### SetVertexColors
Set the vertex colors to use when rendering. Currently, vertex colors are modulated with texture color, but in the future, more blending modes may be added (and also custom material support).

Sig: `Quad:SetVertexColors(topLeft, topRight, bottomLeft, bottomRight)`
 - Arg: `Vector topLeft` Top left vertex color
 - Arg: `Vector topRight` Top right vertex color
 - Arg: `Vector bottomLeft` Bottom left vertex color
 - Arg: `Vector bottomRight` Bottom right vertex color
---
### SetUvScale
Adjust the scale of the UVs. Useful for repeating patterns.

Sig: `Quad:SetUvScale(uvScale)`
 - Arg: `Vector uvScale` UV scale
---
### GetUvScale
Get the UV scale.

Sig: `uvScale = Quad:GetUvScale()`
 - Ret: `Vector uvScale` UV scale
---
### SetUvOffset
Adjust the offset of the UVs. Useful for scrolling backgrounds.

Sig: `Quad:SetUvOffset(uvOffset)`
 - Arg: `Vector uvOffset` UV offset
---
### GetUvOffset
Get the UV offset.

Sig: `uvOffset = Quad:GetUvOffset()`
 - Ret: `Vector uvOffset` UV offset
---
