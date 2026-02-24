# NavMesh3D

Defines a nav volume node used to constrain/path-filter nav generation in 3D scenes.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)
* [Primitive3D](Primitive3D.md)
* [Box3D](Box3D.md)

---
### Notes
- `NavMesh3D` is a subclass of `Box3D`.
- It uses the same transform/extents foundation as `Box3D`.
- In Lua, you can use Box3D-style extent APIs on this node (`GetExtents` / `SetExtents`).
- Nav-specific controls (Nav Bounds, Nav Overlay, Nav Negator, Cull 90° Walls, Wall Cull Threshold) are editor properties on the node.

---
### GetExtents
Gets the extents of the nav volume. Note: These are full extents, not half-extents.

Sig: `extents = NavMesh3D:GetExtents()`
 - Ret: `Vector extents` Extents of nav volume
---
### SetExtents
Sets the extents of the nav volume. Note: These are full extents, not half-extents.

Sig: `NavMesh3D:SetExtents(extents)`
 - Arg: `Vector extents` Extents of nav volume
---
