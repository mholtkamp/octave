# Node3D

A node with a 3D transform associated with it. A Node3D's transform is determined by its relative position, rotation, scale and its parent's transform.

---

### GetPosition
Get this node's position relative to its parent.

Sig: `position = Node3D:GetPosition()`
 - Ret: `Vector position` Relative position
---
### GetRotation
Get this node's rotation relative to its parent as euler angles in degrees.

Sig: `rotEuler = Node3D:GetRotation()`
 - Ret: `Vector rotEuler` Relative rotation in degrees
---
### GetRotationQuat
Get this node's rotation relative to its parent as a quaternion.

Sig: `rotQuat = Node3D:GetRotationQuat()`
 - Ret: `Vector rotQuat` Relative rotation as a quaternion
---
### GetScale
Get this node's scale relative to its parent.

Sig: `scale = Node3D:GetScale()`
 - Ret: `Vector scale` Relative scale
---
### SetPosition
Set this node's relative position.

Sig: `Node3D:SetPosition(position)`
 - Arg: `Vector position` Relative position
---
### SetRotation
Set this node's relative rotation from euler angles in degrees.

Sig: `Node3D:SetRotation(rotation)`
 - Arg: `Vector rotation` Relative rotation as euler angles in degrees
---
### SetRotationQuat
Set this node's relative rotation from a quaternion.

Sig: `Node3D:SetRotationQuat(rotQuat)`
 - Arg: `Vector rotQuat` Relative rotation as a quaternion
---
### SetScale
Set this node's relative scale.

Sig: `Node3D:SetScale(scale)`
 - Arg: `Vector scale` Relative scale
---
### RotateAround
Rotate this node around a given pivot point in world space.

Sig: `Node3D:RotateAround(pivot, axis, degrees)`
 - Arg: `Vector pivot` Point in 3D space to rotate around
 - Arg: `Vector axis` Rotation axis in world space
 - Arg: `number degrees` Degrees to rotate this node around the pivot
---
### GetWorldPosition
Get this node's world space position.

Sig: `position = Node3D:GetWorldPosition()`
 - Ret: `Vector position` World space position
---
### GetWorldRotation
Get this node's world space rotation as euler angles in degrees.

Sig: `rotEuler = Node3D:GetWorldRotation()`
 - Ret: `Vector rotEuler` World space rotation in degrees
---
### GetWorldRotationQuat
Get this node's world space rotation as a quaternion.

Sig: `rotQuat = Node3D:GetWorldRotationQuat()`
 - Ret: `Vector rotQuat` World space rotation as a quaternion
---
### GetWorldScale
Get this node's world space scale.

Sig: `scale = Node3D:GetWorldScale()`
 - Ret: `Vector scale` World space scale
---
### SetWorldPosition
Set this node's world space position.

Sig: `Node3D:SetWorldPosition(position)`
 - Arg: `Vector position` World space position
---
### SetWorldRotation
Set this node's world space rotation from euler angles in degrees.

Sig: `Node3D:SetWorldRotation(rotEuler)`
 - Arg: `Vector rotEuler` World space rotation in degrees
---
### SetWorldRotationQuat
Set this node's world space rotation from a quaternion.

Sig: `Node3D:SetWorldRotationQuat(rotQuat)`
 - Arg: `Vector rotQuat` World space rotation as a quaternion
---
### SetWorldScale
Set this node's world space scale.

Sig: `Node3D:SetWorldScale(scale)`
 - Arg: `Vector scale` World space scale
---
### AddRotation
Add a relative rotation from euler angles in degrees.

Sig: `Node3D:AddRotation(rotEuler)`
 - Arg: `Vector rotEuler` Rotation in degrees.
---
### AddRotationQuat
Add a relative rotation from a quaternion.

Sig: `Node3D:AddRotationQuat(rotQuat)`
 - Arg: `Vector rotQuat` Rotation as a quaternion
---
### AddWorldRotation
Add a world space rotation from euler angles in degrees.

Sig: `Node3D:AddWorldRotation(rotEuler)`
 - Arg: `Vector rotEuler` World space rotation in degrees.
---
### AddWorldRotationQuat
Add a world space rotation from a quaternion.

Sig: `Node3D:AddWorldRotationQuat(rotQuat)`
 - Arg: `Vector rotQuat` World space rotation as a quaternion
---
### LookAt
Rotate this node so that it is looking at a given world-space position.

Sig: `Node3D:LookAt(worldPos, up=Vec(0,1,0))`
 - Arg: `Vector worldPos` World space position this node should look at
 - Arg: `Vector up` The direction that up should be considered
---
### GetForwardVector
Get this node's world space forward facing direction.

Sig: `forward = Node3D:GetForwardVector()`
 - Ret: `Vector forward` The forward vector in world space
---
### GetRightVector
Get this node's world space right vector.

Sig: `right = Node3D:GetRightVector()`
 - Ret: `Vector right` The right vector in world space
---
### GetUpVector
Get this node's world space up vector.

Sig: `up = Node3D:GetUpVector()`
 - Ret: `Vector up` The up vector in world space
---
### AttachToBone
Attach this node to a SkeletalMesh3D node at a specific bone.

Sig: `Node3D:AttachToBone(skelMeshNode, boneName, keepWorldTransform=false, childIndex=0)`
 - Arg: `SkeletalMesh3D skelMeshNode` Mesh node to attach to
 - Arg: `string boneName` Name of the bone to attach to
 - Arg: `boolean keepWorldTransform` Whether to maintain the same world-space transform after attaching to skelMeshNode
 - Arg: `integer childIndex` The specific child index this node should be placed at in the parent's child array. If 0, place this node at the end of the array.
---
### UpdateTransform
Update this node's transformation matrix. When a Node3D has its position, rotation, or scale changed, its transform matrix will remain unchanged until it is needed. Generally, game code will not need to call this. Functions like GetWorldPosition() will automatically update the transform if it is dirty.

Sig: `Node3D:UpdateTransform(updateChildren=false)`
 - Arg: `boolean updateChildren` Should children recursively be updated?
---