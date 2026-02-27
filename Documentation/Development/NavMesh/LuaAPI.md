# NavMesh Lua API Reference

All navigation methods are called on the `World` object. Access the world from a script's node:

```lua
local world = self:GetWorld()
```

## World Navigation Methods

### FindNavPath

Finds a path between two points on the navigation mesh.

```lua
local path, success = world:FindNavPath(start, goal)
```

**Parameters:**
- `start` (Vec3) — Starting position
- `goal` (Vec3) — Goal position

**Returns:**
- `path` (table) — Array of Vec3 waypoints (empty table if not found)
- `success` (boolean) — Whether a valid path was found

**Example:**
```lua
local start = self:GetWorldPosition()
local goal = targetNode:GetWorldPosition()
local path, found = world:FindNavPath(start, goal)

if found then
    for i, waypoint in ipairs(path) do
        print("Waypoint " .. i .. ": " .. tostring(waypoint))
    end
end
```

### FindRandomNavPoint

Returns a random walkable point on the navigation mesh.

```lua
local point, success = world:FindRandomNavPoint()
```

**Returns:**
- `point` (Vec3) — Random point on the nav mesh (zero vector if not found)
- `success` (boolean) — Whether a valid point was found

**Example:**
```lua
local target, found = world:FindRandomNavPoint()
if found then
    -- Navigate AI to random patrol point
    self:SetWorldPosition(target)
end
```

### FindClosestNavPoint

Snaps a position to the nearest walkable point on the navigation mesh.

```lua
local point, success = world:FindClosestNavPoint(position)
```

**Parameters:**
- `position` (Vec3) — The query position

**Returns:**
- `point` (Vec3) — Nearest point on nav mesh (zero vector if not found)
- `success` (boolean) — Whether a nearest point was found

**Example:**
```lua
local spawnPos = Vec3.New(10, 5, 10)
local navPos, found = world:FindClosestNavPoint(spawnPos)
if found then
    enemy:SetWorldPosition(navPos)
end
```

### BuildNavData

Triggers a full navigation mesh rebuild.

```lua
world:BuildNavData()
```

**Note:** Synchronous operation. Call during loading or level setup.

### EnableAutoNavRebuild

Enables or disables automatic nav mesh rebuilding.

```lua
world:EnableAutoNavRebuild(true)   -- enable
world:EnableAutoNavRebuild(false)  -- disable
```

**Parameters:**
- `enable` (boolean) — Whether to enable auto-rebuild

## NavMesh3D Node

NavMesh3D volumes are placed in the scene via the editor. They can also be manipulated from Lua through the standard Node3D API:

```lua
local navVolume = world:FindNode("NavMeshVolume")
navVolume:SetWorldPosition(Vec3.New(0, 0, 0))
navVolume:SetWorldScale(Vec3.New(50, 10, 50))
```

### GetExtents / SetExtents

Since NavMesh3D inherits from Box3D, you can adjust its extents:

```lua
local extents = navVolume:GetExtents()
navVolume:SetExtents(Vec3.New(100, 20, 100))
```
