# InstancedMesh3D

A node that will render multiple instances of a static mesh. This rendering is performed efficiently when using the Vulkan API. On consoles, instanced meshes are baked into multiple static meshes at packaging time. Consoles cannot create instanced mesh nodes on-the-fly during gameplay.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)
* [Mesh3D](Mesh3D.md)
* [StaticMesh3D](StaticMesh3D.md)

---

### GetNumInstances
Get the number of mesh instances.

Sig: `numInstances = InstancedMesh3D:GetNumInstances()`
 - Ret: `integer numInstances` Number of mesh instances
---
### GetInstanceData
Get instance data of a particular instance.

Sig: `instData = InstancedMesh3D:GetInstanceData(index)`
 - Arg: `integer index` Instance index
 - Ret: `table instData` Table containing instance data
   - `Vector position`
   - `Vector rotation`
   - `Vector scale`
---
### SetInstanceData
Set the instance data of a particular instance.

Sig: `InstancedMesh3D:SetInstanceData(index, instData)`
 - Arg: `integer index` Instance index
 - Arg: `table instData` Table containing instance data
   - `Vector position`
   - `Vector rotation`
   - `Vector scale`
---
### AddInstanceData
Add a new instance.

Sig: `InstancedMesh3D:AddInstanceData(instData)`
 - Arg: `table instData` Table containing instance data
   - `Vector position`
   - `Vector rotation`
   - `Vector scale`
---
### RemoveInstanceData
Remove an instance.

Sig: `InstancedMesh3D:RemoveInstanceData(index)`
 - Arg: `integer index` Instance index
---