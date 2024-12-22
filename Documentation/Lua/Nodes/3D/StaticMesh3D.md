# StaticMesh3D

A node that renders a static (no animation) mesh.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)
* [Mesh3D](Mesh3D.md)

---
### SetStaticMesh
Set the static mesh asset that is rendered.

Sig: `StaticMesh3D:SetStaticMesh(mesh)`
 - Arg: `StaticMesh mesh` Static mesh asset 
---
### GetStaticMesh
Get the static mesh asset that this node is set to render.

Sig: `mesh = StaticMesh3D:GetStaticMesh()`
 - Ret: `StaticMesh mesh` Static mesh asset
---
### SetUseTriangleCollision
Allow/disallow this node to use triangle collision. In addition to setting this flag, the StaticMesh asset assigned to this node must also have "GenerateTriangleCollision" enabled.

Triangle collision should only be used for objects that don't move. Triangle collision on a moving object can have a significant performance impact.

Sig: `StaticMesh3D:SetUseTriangleCollision(triCollision)`
 - Arg: `boolean triCollision` Enable triangle collision
---
### GetUseTriangleCollision
Check if triangle collision is enabled for this mesh.

Sig: `triCollision = StaticMesh3D:GetUseTriangleCollision()`
 - Ret: `boolean triCollision` Is triangle collision enabled
---
### GetBakeLighting
Check if this static mesh node should use baked lighting.

Sig: `bakeLighting = StaticMesh3D:GetBakeLighting()`
 - Ret: `boolean bakeLighting` True if lighting should be baked
---