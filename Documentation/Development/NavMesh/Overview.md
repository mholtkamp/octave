# NavMesh System Overview

## Introduction

The Octave Engine NavMesh system provides runtime navigation mesh generation and pathfinding using the industry-standard [Recast](https://github.com/recastnavigation/recastnavigation) library for mesh generation and [Detour](https://github.com/recastnavigation/recastnavigation) for pathfinding queries.

## System Architecture

The NavMesh system consists of three main components:

### 1. NavMesh3D Volume Nodes

`NavMesh3D` nodes (inheriting from `Box3D`) are placed in the scene to define the walkable area. They act as bounding volumes that control which geometry is included in the navigation mesh build.

**Properties:**

| Property | Default | Description |
|----------|---------|-------------|
| Nav Bounds | true | When enabled, this volume's extents define the area to include in the nav mesh build |
| Nav Overlay | true | Renders a wireframe overlay of the generated nav mesh for debugging |
| Nav Negator | false | When enabled, this volume *subtracts* from the walkable area (useful for doors, obstacles) |
| Cull Walls | false | Removes near-vertical triangles from the nav mesh input |
| Wall Cull Threshold | 0.2 | Dot-product threshold for wall culling (lower = more aggressive culling) |

### 2. Recast Build Pipeline

When `BuildNavigationData()` is called, the system:

1. **Gathers triangles** from all `StaticMesh3D` nodes within NavMesh3D bounds
2. **Applies negators** — removes triangles within negator volumes
3. **Applies wall culling** — optionally removes near-vertical faces
4. **Rasterizes** — Recast converts triangles to a voxel heightfield
5. **Builds regions** — identifies walkable areas
6. **Generates polygons** — creates the navigation polygon mesh
7. **Creates Detour mesh** — builds the Detour `dtNavMesh` for runtime queries
8. **Creates query object** — initializes `dtNavMeshQuery` for pathfinding

### 3. World Query API

The `World` class exposes pathfinding queries that operate on the built navigation data:

- **FindNavPath** — A* pathfinding between two points, returns a list of waypoints
- **FindRandomNavPoint** — Returns a random walkable point on the nav mesh
- **FindClosestNavPoint** — Snaps an arbitrary point to the nearest walkable surface
- **BuildNavigationData** — Triggers a full nav mesh rebuild
- **EnableAutoNavRebuild** — Enables automatic rebuilds when geometry changes
- **InvalidateNavMesh** — Marks the nav mesh as dirty (triggers rebuild if auto-rebuild is on)

## Build Parameters

These constants control the Recast voxelization and are defined in `World.cpp`:

| Parameter | Value | Description |
|-----------|-------|-------------|
| Cell Size | 0.3 | XZ resolution of the voxel grid. Smaller = more detail but slower build |
| Cell Height | 0.2 | Y resolution of the voxel grid |
| Walkable Slope | 55 deg | Maximum slope angle considered walkable |
| Walkable Height | 2.0 | Minimum ceiling clearance for the agent |
| Walkable Climb | 0.9 | Maximum step height the agent can climb |
| Walkable Radius | 0.4 | Agent collision radius (erodes nav mesh edges) |
| Query Extents | 2.0, 4.0, 2.0 | Search radius for nearest-polygon lookups |

## Auto-Rebuild

When auto-rebuild is enabled via `EnableAutoNavRebuild(true)`, the nav mesh is automatically rebuilt when `InvalidateNavMesh()` is called. This is useful for dynamic environments where obstacles move at runtime.

## File Locations

| File | Description |
|------|-------------|
| `Engine/Source/Engine/Nodes/3D/NavMesh3d.h/.cpp` | NavMesh3D volume node |
| `Engine/Source/Engine/World.h/.cpp` | World nav API and Recast build logic |
| `Engine/Source/LuaBindings/World_Lua.h/.cpp` | Lua bindings |
| `Engine/Source/Engine/NodeGraph/Nodes/NavMeshNodes.h/.cpp` | NodeGraph nodes |
| `Engine/External/Recast/` | Recast library source |
| `Engine/External/Detour/` | Detour library source |
