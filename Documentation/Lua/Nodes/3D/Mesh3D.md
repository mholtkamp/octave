# Mesh3D

Base class for a node that can be rendered using a triangle mesh.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)

---
### GetMaterial
Get the material that this mesh will use for rendering. If a material override has been assigned using SetMaterialOverride(), then this function will return that overridden material. Otherwise, the material assigned on the mesh asset will be returned.

Sig: `material = Mesh3D:GetMaterial()`
 - Ret: `Material material` Material used for rendering
---
### GetMaterialOverride
Get the overridden material for this node. A material override will be used instead of the mesh asset's material.

Sig: `material = Mesh3D:GetMaterialOverride()`
 - Ret: `Material material` Override material
---
### SetMaterialOverride
Set the override material for this node. A material override will be used instead of the mesh asset's material.

Sig: `Mesh3D:SetMaterialOverride(material)`
 - Arg: `Material material` Override material
---
### InstantiateMaterial
Create a new material instance based on the existing material (as retrieved from GetMaterial()) and assign it to this node.

Sig: `material = Mesh3D:InstantiateMaterial()`
 - Ret: `Material material` The newly created instance
---
### IsBillboard
Check if this node is set to billboard. Billboarded meshes will always face the camera.
TODO: Consider adding a BillboardNode.

Sig: `billboard = Mesh3D:IsBillboard()`
 - Ret: `boolean billboard` True if mesh is set to billboard
---
### SetBillboard
Set whether this node should billboard. Billboarded meshes will always face the camera.
TODO: Consider adding a BillboardNode.

Sig: `Mesh3D:SetBillboard(billboard)`
 - Arg: `boolean billboard` Whether to billboard
---