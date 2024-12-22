# SkeletalMesh

A mesh that can contain a bone hierarchy and animations.

Inheritance:
* [Asset](Asset.md)

---
### GetMaterial
Get the mesh's default material.

Sig: `material = SkeletalMesh:GetMaterial()`
 - Ret: `Material material` Default material
---
### SetMaterial
Set the mesh's default material.

Sig: `SkeletalMesh:SetMaterial(material)`
 - Arg: `Material material` Default material
---
### GetNumIndices
Get the number of indices.

Sig: `num = SkeletalMesh:GetNumIndices()`
 - Ret: `integer num` Num indices
---
### GetNumFaces
Get the number of faces.

Sig: `num = SkeletalMesh:GetNumFaces()`
 - Ret: `integer num` Num faces
---
### GetNumVertices
Get the number of vertices.

Sig: `num = SkeletalMesh:GetNumVertices()`
 - Ret: `integer num` Num vertices
---
### FindBoneIndex
Find a bone's index from its name.

Sig: `index = SkeletalMesh:FindBoneIndex(name)`
 - Arg: `string name` Bone name
 - Ret: `integer index` Bone index
---
### GetNumBones
Get the number of bones.

Sig: `num = SkeletalMesh:GetNumBones()`
 - Ret: `integer num` Num bones
---
### GetAnimationName
Get an animation name from its index.

Sig: `name = SkeletalMesh:GetAnimationName(index)`
 - Arg: `integer index` Animation index
 - Ret: `string name` Animation name
---
### GetNumAnimations
Get the number of animations.

Sig: `num = SkeletalMesh:GetNumAnimations()`
 - Ret: `integer num` Number of animations
---
### GetAnimationDuration
Get an animation's duration.

Sig: `duration = SkeletalMesh:GetAnimationDuration(name)`
 - Arg: `string name` Animation name
 - Ret: `number duration` Animation duration
---
