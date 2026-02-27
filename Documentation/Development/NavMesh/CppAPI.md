# NavMesh C++ API Reference

## World Navigation Methods

All navigation methods are on the `World` class. Access from a Node3D:

```cpp
World* world = myNode3D->GetWorld();
```

### FindNavPath

Finds a path between two points on the navigation mesh using A* pathfinding.

```cpp
bool FindNavPath(glm::vec3 start, glm::vec3 end, std::vector<glm::vec3>& outPath);
```

**Parameters:**
- `start` — Starting position (snapped to nearest nav poly)
- `end` — Goal position (snapped to nearest nav poly)
- `outPath` — Output vector of waypoints (cleared on entry)

**Returns:** `true` if a valid path was found, `false` otherwise.

**Example:**
```cpp
std::vector<glm::vec3> path;
if (world->FindNavPath(enemyPos, playerPos, path))
{
    // path[0] is near start, path[path.size()-1] is near end
    for (const auto& waypoint : path)
    {
        // Move toward each waypoint in sequence
    }
}
```

### FindRandomNavPoint

Returns a random walkable point on the navigation mesh.

```cpp
bool FindRandomNavPoint(glm::vec3& outPoint);
```

**Parameters:**
- `outPoint` — Output position on the nav mesh

**Returns:** `true` if a valid point was found, `false` if no nav mesh exists.

**Example:**
```cpp
glm::vec3 patrolTarget;
if (world->FindRandomNavPoint(patrolTarget))
{
    // Navigate to patrolTarget
}
```

### FindClosestNavPoint

Snaps a position to the nearest point on the navigation mesh.

```cpp
bool FindClosestNavPoint(glm::vec3 inPoint, glm::vec3& outPoint);
```

**Parameters:**
- `inPoint` — The query position
- `outPoint` — The nearest point on the nav mesh

**Returns:** `true` if a nearest point was found within the query extents.

**Example:**
```cpp
glm::vec3 spawnPos(10.0f, 5.0f, 10.0f);
glm::vec3 navPos;
if (world->FindClosestNavPoint(spawnPos, navPos))
{
    // navPos is on the walkable surface
    enemy->SetPosition(navPos);
}
```

### BuildNavigationData

Triggers a full navigation mesh rebuild. Gathers all StaticMesh3D triangles within NavMesh3D bounds and runs the Recast pipeline.

```cpp
void BuildNavigationData();
```

**Note:** This is a synchronous operation. For large scenes, call during loading or level transitions.

### EnableAutoNavRebuild

Enables or disables automatic nav mesh rebuilding when `InvalidateNavMesh()` is called.

```cpp
void EnableAutoNavRebuild(bool enable);
```

### IsAutoNavRebuildEnabled

Returns whether auto-rebuild is currently enabled.

```cpp
bool IsAutoNavRebuildEnabled() const;
```

### InvalidateNavMesh

Marks the navigation mesh as dirty. If auto-rebuild is enabled, this triggers a rebuild.

```cpp
void InvalidateNavMesh();
```

## NavMesh3D Class

`NavMesh3D` inherits from `Box3D` and defines navigation volume properties.

**Header:** `Engine/Source/Engine/Nodes/3D/NavMesh3d.h`

### Key Methods

```cpp
void SetNavBounds(bool navBounds);   // Include in nav build bounds
bool IsNavBounds() const;
bool IsNavOverlayEnabled() const;    // Draw debug wireframe
bool IsNavNegatorEnabled() const;    // Subtract from walkable area
bool IsCullWallsEnabled() const;     // Remove vertical faces
float GetWallCullThreshold() const;  // Wall cull dot-product threshold
```

### Properties

| Member | Type | Default | Description |
|--------|------|---------|-------------|
| `mNavBounds` | bool | true | Include volume in nav build |
| `mNavOverlay` | bool | true | Show debug overlay |
| `mNavNegator` | bool | false | Subtract from walkable area |
| `mCullWalls` | bool | false | Cull near-vertical faces |
| `mWallCullThreshold` | float | 0.2 | Wall cull sensitivity |

## StaticMesh3D NavMesh Integration

StaticMesh3D nodes automatically contribute their geometry to the nav mesh build if they are within a NavMesh3D bounds volume. No additional configuration is needed on the mesh node itself.
