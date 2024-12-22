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
