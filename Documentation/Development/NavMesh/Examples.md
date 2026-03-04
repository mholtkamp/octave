# NavMesh Examples

## Example 1: Basic Pathfinding (Lua)

Simple AI that finds and follows a path to the player.

```lua
function OnStart(self)
    local world = self:GetWorld()
    world:BuildNavData()
    self.pathIndex = 1
    self.path = {}
    self.speed = 5.0
end

function OnTick(self, deltaTime)
    local world = self:GetWorld()
    local myPos = self:GetWorldPosition()
    local player = world:FindNode("Player")
    if player == nil then return end

    local playerPos = player:GetWorldPosition()

    -- Repath periodically
    self.repathTimer = (self.repathTimer or 0) + deltaTime
    if self.repathTimer > 1.0 then
        self.repathTimer = 0
        self.path, self.pathFound = world:FindNavPath(myPos, playerPos)
        self.pathIndex = 1
    end

    -- Follow path
    if self.pathFound and self.pathIndex <= #self.path then
        local target = self.path[self.pathIndex]
        local dir = target - myPos
        dir.y = 0
        local dist = dir:Length()

        if dist < 0.5 then
            self.pathIndex = self.pathIndex + 1
        else
            dir = dir:GetNormalized()
            local newPos = myPos + dir * self.speed * deltaTime
            self:SetWorldPosition(newPos)
        end
    end
end
```

## Example 2: Auto-Rebuild with Moving Obstacles

Set up auto-rebuild so the nav mesh updates when obstacles move.

```lua
function OnStart(self)
    local world = self:GetWorld()

    -- Initial build
    world:BuildNavData()

    -- Enable auto-rebuild for dynamic environments
    world:EnableAutoNavRebuild(true)
end

function OnObstacleMoved(self)
    -- Call this when a nav-relevant object moves
    local world = self:GetWorld()
    world:InvalidateNavMesh()
    -- Nav mesh will automatically rebuild since auto-rebuild is enabled
end
```

## Example 3: NavMesh Negator for Doors

Use a NavMesh3D negator volume to block a doorway, then remove it when the door opens.

Place two NavMesh3D volumes in the scene:
- **"NavBounds"** — Large volume covering the entire level (Nav Bounds = true)
- **"DoorBlocker"** — Small volume covering the doorway (Nav Negator = true)

```lua
function OnStart(self)
    local world = self:GetWorld()
    world:BuildNavData()
    world:EnableAutoNavRebuild(true)
end

function OpenDoor(self)
    local world = self:GetWorld()
    local blocker = world:FindNode("DoorBlocker")

    -- Move the negator volume out of the way
    blocker:SetWorldPosition(Vec3.New(0, -100, 0))

    -- Rebuild nav mesh without the blocker
    world:InvalidateNavMesh()
end

function CloseDoor(self)
    local world = self:GetWorld()
    local blocker = world:FindNode("DoorBlocker")

    -- Restore the negator volume to the doorway
    blocker:SetWorldPosition(self.doorBlockerOriginalPos)

    -- Rebuild nav mesh with the blocker
    world:InvalidateNavMesh()
end
```

## Example 4: Node Graph AI Patrol

A visual scripting approach to AI patrol using NavMesh nodes.

### Graph Setup

1. **Start Event** → **Build Nav Data** → **Enable Auto Nav Rebuild (true)**

2. **Tick Event** → **Branch (needsNewTarget?)** :
   - True → **Find Random Nav Point** → store target, set needsNewTarget = false
   - False → move toward current target

3. When reaching the target (distance < threshold), set needsNewTarget = true

### Step-by-step Node Connections

```
[Start] ──Exec──► [Build Nav Data] ──Exec──► [Enable Auto Nav Rebuild]
                                                    │ Enable = true

[Tick] ──Exec──► [Branch]
                    │ True ──► [Find Random Nav Point] ──► [Set Variable: PatrolTarget]
                    │ False ──► [Move Toward PatrolTarget]
```

### Using Find Nav Path with Points

To follow an actual navmesh path instead of moving in a straight line:

```
[Find Nav Path] ── Path ──► [Get Point Count] ── Count ──► (loop index)
                   │
                   └── Path ──► [Get Point Position (index)] ── Position ──► (move target)
```

The **Find Nav Path** node outputs a PointCloud, which integrates seamlessly with the Point node system. Use **For Each Point** to iterate over waypoints, or **Get Point Position** to access specific waypoints by index.
