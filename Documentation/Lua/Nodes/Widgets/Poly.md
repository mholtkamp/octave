# Poly

A widget that can render polygons. Poly widgets are currently only supported on Vulkan.

Inheritance:
* [Node](../Node.md)
* [Widget](Widget.md)

---
### AddVertex
Add a vertex to the polygon.

Sig: `Poly:AddVertex(position, color, uv)`
 - Arg: `Vector position` X/Y position
 - Arg: `Vector color` Color
 - Arg: `Vector uv` Texture coordinate
---
### ClearVertices
Clear all vertices.

Sig: `Poly:ClearVertices()`

---
### GetNumVertices
Get the number of vertices.

Sig: `numVertices = Poly:GetNumVertices()`
 - Ret: `integer numVertices` Number of vertices
---
### GetVertex
Get a specific vertex.

Sig: `vertex = Poly:GetVertex(index)`
 - Arg: `integer index` Vertex index
 - Ret: `table vertex` Vertex data table
   - `Vector position`
   - `Vector color`
   - `Vector uv`
---
### SetTexture
Set the texture to use. Texture support not yet implemented.

Sig: `Poly:SetTexture(texture)`
 - Arg: `Texture texture` Texture to use
---
### GetTexture
Get the polygon's assigned texture.

Sig: `texture = Poly:GetTexture()`
 - Ret: `Texture texture` Assigned texture
---
### GetLineWidth
Get the line width to use when rendering.

Sig: `lineWidth = Poly:GetLineWidth()`
 - Ret: `number lineWidth` Line width
---
### SetLineWidth
Set the line width to use when rendering.

Sig: `Poly:SetLineWidth(lineWidth)`
 - Arg: `number lineWidth` Line width
---
