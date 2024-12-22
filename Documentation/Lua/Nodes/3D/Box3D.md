# Box3D

Defines a box in 3D space. Can be used for collision and overlap detection.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)

---
### GetExtents
Gets the extents of the box. Note: These are the full extents, not half-extents.

Sig: `extents = Box3D:GetExtents()`
 - Ret: `Vector extents` Extents of box
---
### SetExtents
Sets the extents of the box. Note: These are the full extents, not half-extents.

Sig: `Box3D:SetExtents(extents)`
 - Arg: `Vector extents` Extents of box
---