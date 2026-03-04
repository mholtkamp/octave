# NavMesh System

## Overview

The NavMesh system integrates Recast/Detour for runtime navigation mesh generation and pathfinding. NavMesh3D volume nodes define walkable bounds, and the World API provides pathfinding queries. Accessible from C++, Lua, and the NodeGraph visual scripting system.

## Architecture

```
NavMesh3D (Box3D)              — Volume node defining nav bounds/negators
  └── Properties: bounds, overlay, negator, wall culling

World                          — Owns nav cache, exposes query API
  └── RecastNavData            — Per-world Recast mesh + Detour query object
       ├── rcPolyMesh          — Recast polygon mesh
       ├── dtNavMesh           — Detour navigation mesh
       └── dtNavMeshQuery      — Detour query interface

Build Pipeline:
  1. Gather triangles from StaticMesh3D nodes (navmesh-ready)
  2. Apply NavMesh3D bounds/negators
  3. Recast rasterization → polygon mesh
  4. Detour nav mesh creation → query object
```

## Key Files

| File | Purpose |
|------|---------|
| `Engine/Nodes/3D/NavMesh3d.h/.cpp` | NavMesh3D volume node (bounds, negator, overlay) |
| `Engine/World.h/.cpp` | Nav API: FindNavPath, FindRandomNavPoint, FindClosestNavPoint, etc. |
| `LuaBindings/World_Lua.h/.cpp` | Lua bindings for World nav methods |
| `Engine/NodeGraph/Nodes/NavMeshNodes.h/.cpp` | 7 NodeGraph nodes for nav queries/actions |
| `Engine/NodeGraph/Domains/SceneGraphDomain.cpp` | Registers NavMesh nodes in SceneGraph domain |
| `External/Recast/` | Recast navigation mesh library |
| `External/Detour/` | Detour pathfinding library |

All paths relative to `Engine/Source/`.

## NavMesh3D Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `mNavBounds` | bool | true | Include this volume in nav build bounds |
| `mNavOverlay` | bool | true | Draw nav mesh wireframe overlay |
| `mNavNegator` | bool | false | Subtract this volume from walkable area |
| `mCullWalls` | bool | false | Remove near-vertical triangles |
| `mWallCullThreshold` | float | 0.2f | Dot-product threshold for wall culling |
| `mNavOverlayLineThickness` | float | 1.0f | Debug overlay line width |
| `mNavOverlayWireColor` | vec4 | (0.1, 1.0, 0.25, 1.0) | Debug overlay color |

## Build Parameters (World.cpp constants)

| Constant | Value | Description |
|----------|-------|-------------|
| `kNavCellSize` | 0.3 | Voxel cell size (XZ) |
| `kNavCellHeight` | 0.2 | Voxel cell height (Y) |
| `kNavWalkableSlopeDeg` | 55.0 | Max walkable slope in degrees |
| `kNavWalkableHeight` | 2.0 | Agent height |
| `kNavWalkableClimb` | 0.9 | Max step climb height |
| `kNavWalkableRadius` | 0.4 | Agent radius |
| `kNavQueryExtX/Y/Z` | 2.0/4.0/2.0 | Nearest-poly search extents |

## World C++ API

```cpp
bool FindNavPath(glm::vec3 start, glm::vec3 end, std::vector<glm::vec3>& outPath);
bool FindRandomNavPoint(glm::vec3& outPoint);
bool FindClosestNavPoint(glm::vec3 inPoint, glm::vec3& outPoint);
void BuildNavigationData();
void EnableAutoNavRebuild(bool enable);
bool IsAutoNavRebuildEnabled() const;
void InvalidateNavMesh();
```

## Lua API

```lua
local path, success = world:FindNavPath(startVec, goalVec)  -- returns table of Vec3, bool
local point, success = world:FindRandomNavPoint()            -- returns Vec3, bool
local point, success = world:FindClosestNavPoint(posVec)     -- returns Vec3, bool
world:BuildNavData()
world:EnableAutoNavRebuild(true)
```

## NodeGraph Nodes (Category: "NavMesh")

| Node | Type | Inputs | Outputs |
|------|------|--------|---------|
| Find Nav Path | Data | Start (Vec3), Goal (Vec3) | Path (PointCloud), Found (Bool), Waypoint Count (Int) |
| Find Random Nav Point | Data | — | Point (Vec3), Found (Bool) |
| Find Closest Nav Point | Data | Position (Vec3) | Point (Vec3), Found (Bool) |
| Is Auto Nav Rebuild | Data | — | Enabled (Bool) |
| Build Nav Data | Flow | Exec | Exec |
| Enable Auto Nav Rebuild | Flow | Exec, Enable (Bool) | Exec |
| Invalidate Nav Mesh | Flow | Exec | Exec |

Node color: teal-green `(0.2, 0.7, 0.5, 1.0)`.

## Runtime Workflow

1. Place NavMesh3D volumes in scene to define walkable area
2. Call `BuildNavigationData()` (or enable auto-rebuild)
3. Query paths with `FindNavPath()`, random points with `FindRandomNavPoint()`
4. Use `InvalidateNavMesh()` when geometry changes, then rebuild
