# MaterialInstance

A material instance can override default parameters on a base material. MaterialInstances do not have unique shaders compiled for them, instead they reference the MaterialBase compiled shader. MaterialInstance is only supported on Vulkan platforms.

Inheritance:
* [Asset](Asset.md)
* [Material](Material.md)

---
### MaterialInstance.Create(srcMat)
Create a new material instance from a given source material. The source material can be either a MaterialBase or another MaterialInstance.

Sig: `inst = MaterialInstance.Create(srcMat)`
 - Arg: `Material srcMat` MaterialBase or MaterialInstance source material
 - Ret: `MaterialInstance inst` Newly created MaterialInstance
---
### GetBaseMaterial
Get the base material used by this instance.

Sig: `base = MaterialInstance:GetBaseMaterial()`
 - Ret: `MaterialBase base` Referenced base material
---
### SetBaseMaterial
Set the base material used by this instance.

Sig: `MaterialInstance:SetBaseMaterial(base)`
 - Arg: `MaterialBase base` Base material to reference
---
