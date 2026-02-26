# Gizmos System

Unity-style editor-only debug drawing system for visualizing game objects, colliders, and spatial data in the scene view.

## Overview

The Gizmos system provides drawing primitives that are only rendered in the editor (not during play mode). It supports:

- **Solid draws** (cubes, spheres, meshes) rendered with the Forward pipeline
- **Wire draws** (wireframe cubes, spheres, meshes) rendered with the Wireframe pipeline
- **Lines** (lines, rays, frustums) rendered with the Line pipeline

Gizmos can be used from three access methods:
1. **Lua scripts** via `OnDrawGizmos` / `OnDrawGizmosSelected` callbacks
2. **NodeGraph** visual scripting nodes under the "Gizmo" category
3. **Native C++ addons** via the `OctaveEngineAPI` function pointers

## Node Callbacks

Any `Node3D` in the scene can implement two callbacks:

| Callback | When Called |
|---|---|
| `OnDrawGizmos` | Every frame for every Node3D in the scene |
| `OnDrawGizmosSelected` | Only when the node is selected in the editor |

Both callbacks are called during the `GatherDrawData` phase of rendering. `Gizmos.ResetState()` is called automatically before each callback, so color and matrix start at their defaults (white, identity).

### Lua Example

```lua
function OnDrawGizmos(self)
    -- Draw a green wireframe sphere around this node
    Gizmos.SetColor(Vec(0, 1, 0, 0.5))
    Gizmos.DrawWireSphere(self:GetWorldPosition(), 3.0)
end

function OnDrawGizmosSelected(self)
    -- Draw a yellow wireframe cube when selected
    Gizmos.SetColor(Vec(1, 1, 0, 1))
    Gizmos.DrawWireCube(self:GetWorldPosition(), Vec(2, 2, 2))
end
```

### C++ Override Example

```cpp
class MyNode : public Node3D
{
    // ...
#if EDITOR
    virtual void OnDrawGizmos() override
    {
        Gizmos::SetColor(glm::vec4(0, 1, 0, 0.5f));
        Gizmos::DrawWireSphere(GetWorldPosition(), 3.0f);
    }

    virtual void OnDrawGizmosSelected() override
    {
        Gizmos::SetColor(glm::vec4(1, 1, 0, 1));
        Gizmos::DrawWireCube(GetWorldPosition(), glm::vec3(2.0f));
    }
#endif
};
```

## C++ API Reference

All methods are static on the `Gizmos` class. Include `"Gizmos.h"`.

### State

| Method | Description |
|---|---|
| `Gizmos::SetColor(glm::vec4 color)` | Set the draw color (RGBA) |
| `Gizmos::GetColor()` | Get the current draw color |
| `Gizmos::SetMatrix(const glm::mat4& matrix)` | Set the transformation matrix |
| `Gizmos::GetMatrix()` | Get the current transformation matrix |
| `Gizmos::ResetState()` | Reset color to white, matrix to identity |
| `Gizmos::IsEnabled()` | Returns true if gizmos are active (editor, not playing) |

### Solid Draws

| Method | Description |
|---|---|
| `DrawCube(center, size)` | Draw a solid cube |
| `DrawSphere(center, radius)` | Draw a solid sphere |
| `DrawMesh(mesh, position, rotation?, scale?)` | Draw a solid StaticMesh |

### Wire Draws

| Method | Description |
|---|---|
| `DrawWireCube(center, size)` | Draw a wireframe cube |
| `DrawWireSphere(center, radius)` | Draw a wireframe sphere |
| `DrawWireMesh(mesh, position, rotation?, scale?)` | Draw a wireframe StaticMesh |

### Lines

| Method | Description |
|---|---|
| `DrawLine(from, to)` | Draw a line between two points |
| `DrawRay(origin, direction)` | Draw a ray from origin along direction |
| `DrawFrustum(viewProjectionMatrix)` | Draw a camera frustum (12 edges) |
| `DrawLineList(points, count)` | Draw pairs of points as lines |
| `DrawLineStrip(points, count)` | Draw connected line segments |

## Lua API Reference

Global `Gizmos` table, available in any Lua script.

### State

```lua
Gizmos.SetColor(color)        -- Vec(r, g, b, a)
Gizmos.GetColor()              -- returns Vec
Gizmos.SetMatrix(m0..m15)      -- 16 floats (column-major), or nil for identity
Gizmos.GetMatrix()              -- returns 16 numbers
Gizmos.ResetState()
```

### Drawing

```lua
Gizmos.DrawCube(center, size)           -- center: Vec3, size: Vec3
Gizmos.DrawSphere(center, radius)       -- center: Vec3, radius: number
Gizmos.DrawMesh(mesh, pos, rot?, scale?) -- mesh: StaticMesh asset
Gizmos.DrawWireCube(center, size)
Gizmos.DrawWireSphere(center, radius)
Gizmos.DrawWireMesh(mesh, pos, rot?, scale?)
Gizmos.DrawLine(from, to)              -- from: Vec3, to: Vec3
Gizmos.DrawRay(origin, direction)       -- origin: Vec3, direction: Vec3
Gizmos.DrawFrustum(m0..m15)            -- 16 floats (view-projection matrix)
```

### Full Lua Example

```lua
function OnDrawGizmos(self)
    local pos = self:GetWorldPosition()

    -- Draw detection radius
    Gizmos.SetColor(Vec(1, 0, 0, 0.3))
    Gizmos.DrawWireSphere(pos, self.detectionRadius or 5.0)

    -- Draw forward ray
    Gizmos.SetColor(Vec(0, 0, 1, 1))
    Gizmos.DrawRay(pos, self:GetForwardVector() * 3.0)
end

function OnDrawGizmosSelected(self)
    local pos = self:GetWorldPosition()

    -- Draw interaction zone
    Gizmos.SetColor(Vec(0, 1, 0, 0.5))
    Gizmos.DrawWireCube(pos, Vec(4, 2, 4))

    -- Draw connection lines to children
    Gizmos.SetColor(Vec(1, 1, 0, 1))
    for i = 0, self:GetNumChildren() - 1 do
        local child = self:GetChild(i)
        if child:IsNode3D() then
            Gizmos.DrawLine(pos, child:GetWorldPosition())
        end
    end
end
```

## NodeGraph Nodes

All gizmo nodes are flow nodes (Exec in/out) under the **Gizmo** category (magenta/pink color).

| Node | Inputs | Description |
|---|---|---|
| Set Gizmo Color | Color (vec4) | Set the drawing color |
| Set Gizmo Matrix | Node3D (optional) | Set matrix from a node's transform |
| Reset Gizmo State | *(none)* | Reset to white color, identity matrix |
| Draw Cube | Center (vec3), Size (vec3) | Draw a solid cube |
| Draw Wire Cube | Center (vec3), Size (vec3) | Draw a wireframe cube |
| Draw Sphere | Center (vec3), Radius (float) | Draw a solid sphere |
| Draw Wire Sphere | Center (vec3), Radius (float) | Draw a wireframe sphere |
| Draw Line | From (vec3), To (vec3) | Draw a line |
| Draw Ray | Origin (vec3), Direction (vec3) | Draw a ray |

### NodeGraph Example

Connect a **Tick** event to **Set Gizmo Color** (green) then to **Draw Wire Sphere** (with Get Position feeding Center, and a Float constant for Radius).

## Native Addon API

The `OctaveEngineAPI` struct provides C-ABI function pointers (scalar floats, no glm types):

```c
// State
api->Gizmos_SetColor(r, g, b, a);
api->Gizmos_SetMatrix(float* matrix16);  // column-major 4x4
api->Gizmos_ResetState();

// Drawing
api->Gizmos_DrawCube(cx, cy, cz, sx, sy, sz);
api->Gizmos_DrawWireCube(cx, cy, cz, sx, sy, sz);
api->Gizmos_DrawSphere(cx, cy, cz, radius);
api->Gizmos_DrawWireSphere(cx, cy, cz, radius);
api->Gizmos_DrawLine(x0, y0, z0, x1, y1, z1);
api->Gizmos_DrawRay(ox, oy, oz, dx, dy, dz);
```

### Native Addon Example

```cpp
static OctaveEngineAPI* sAPI = nullptr;

int OnLoad(OctaveEngineAPI* api)
{
    sAPI = api;
    return 0;
}

void TickEditor(float deltaTime)
{
    // Draw a red wireframe sphere at the origin
    sAPI->Gizmos_SetColor(1.0f, 0.0f, 0.0f, 1.0f);
    sAPI->Gizmos_DrawWireSphere(0.0f, 0.0f, 0.0f, 5.0f);
}
```

## Implementation Details

- Gizmos are only rendered in `EDITOR` builds and only when **not** playing in the editor
- Each draw method checks `Gizmos::IsEnabled()` and returns immediately if false (zero overhead in play mode)
- Draw lists are cleared at the start of each frame via `Gizmos::BeginFrame()`
- Solid draws use `SM_Cube` / `SM_Sphere` built-in engine meshes
- The transformation matrix (`sMatrix`) is applied to all subsequent draws until changed
- `ResetState()` is called automatically before each node's `OnDrawGizmos` / `OnDrawGizmosSelected` callback
