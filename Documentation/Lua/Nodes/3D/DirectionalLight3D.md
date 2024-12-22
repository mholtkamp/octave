# DirectionalLight3D

A node that emits directional light, like a sun.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Light3D](Light3D.md)

---
### GetDirection
Get the direction the light is emitting from. Direction is derived from the node's rotation.

Sig: `direction = DirectionalLight3D:GetDirection()`
 - Ret: `Vector direction` Light direction
---
### SetDirection
Set the light direction. This will adjust the node's rotation (see Node3D).

Sig: `DirectionalLight3D:SetDirection(direction)`
 - Arg: `Vector direction` Light direction
---