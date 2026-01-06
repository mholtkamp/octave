# StaticMesh

An asset that contains a triangle mesh.

Inheritance:
* [Asset](Asset.md)

---
### GetMaterial
Get the default material.

Sig: `material = StaticMesh:GetMaterial()`
 - Ret: `Material material` Default material
---
### SetMaterial
Set the default material.

Sig: `StaticMesh:SetMaterial(material)`
 - Arg: `Material material` Default material
---
### GetNumIndices
Get the number of indices.

Sig: `num = StaticMesh:GetNumIndices()`
 - Ret: `integer num` Number of indices
---
### GetNumFaces
Get the number of faces.

Sig: `num = StaticMesh:GetNumFaces()`
 - Ret: `integer num` Number of faces
---
### GetNumVertices
Get the number of vertices.

Sig: `num = StaticMesh:GetNumVertices()`
 - Ret: `integer num` Number of vertices
---
### HasVertexColor
Check if the mesh has vertex color data.

Sig: `hasColor = StaticMesh:HasVertexColor()`
 - Ret: `boolean hasColor` Has vertex color
---
### GetVertices
Gets the vertex data array.

Sig: `vertices = StaticMesh:GetVertices()`
 - Ret: `table vertices` An array of vertices
   - `table vertex` Element of the array
     - `Vector position`
     - `Vector texcoord0`
     - `Vector texcoord1`
     - `Vector normal`
     - `integer color` (nil if mesh has no color, RGBA8 stored in 32 bits)
---
### GetIndices
Gets the index array. Each 3 indices represents a triangle. Use the indices to look up the vertex data from the array returned by GetVertices().

Note: Indices are zero-based, so you will need to add 1 when indexing the vertex array.

Example that iterates over the triangles of a mesh:
```lua
local mesh = self:GetStaticMesh()
local indices = mesh:GetIndices()
local verts = mesh:GetVertices()
local numFaces = mesh:GetNumFaces()

Log.Debug('NumFaces = ' .. numFaces .. "   NumVerts = " .. #verts)

for i = 0, numFaces - 1 do
    Log.Debug('Tri ' .. i + 1 .. ' <' .. indices[i * 3 + 1] .. ',' .. indices[i * 3 + 2] .. ',' .. indices[i * 3 + 3] .. '>')

    for j = 1, 3 do
        local index = indices[i * 3 + j]
        local vert = verts[index + 1]
        local position = vert.position
        local texCoord0 = vert.texcoord0
        local texCoord1 = vert.texcoord1
        local normal = vert.normal
        local color = vert.color

        Log.Debug(tostring(vert.position))
    end
end
```

Sig: `indices = StaticMesh:GetIndices()`
 - Ret: `table indices` An array of vertex indices
---
### HasTriangleMeshCollision
Check if the mesh has triangle collision data.

Sig: `triCollision = StaticMesh:HasTriangleMeshCollision()`
 - Ret: `boolean triCollision` Has triangle collision data
---
### EnableTriangleMeshCollision
Set whether this mesh should have triangle collision data generated.

Sig: `StaticMesh:EnableTriangleMeshCollision(enable)`
 - Arg: `boolean enable` Enable triangle collision
---
