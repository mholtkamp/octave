# Timeline Lua Scripting API

Control Timeline playback from Lua scripts using the `TimelinePlayer` node and `Timeline` asset APIs.

**Source:** `Engine/Source/LuaBindings/TimelinePlayer_Lua.h/.cpp`, `Timeline_Lua.h/.cpp`

## TimelinePlayer Methods

`TimelinePlayer` inherits from `Node`, so all standard Node methods are available in addition to the ones listed here.

### Playback Control

```lua
player:Play()
```
Start or resume playback from the current time.

```lua
player:Pause()
```
Pause playback. The current time position is preserved.

```lua
player:Stop()
```
Stop playback and reset time to 0. Restores all target nodes to their pre-play state.

> **Note:** `Stop()` in Lua calls `StopPlayback()` in C++, not the Node `Stop()` method.

### Time

```lua
player:SetTime(time)    -- time: number (seconds)
```
Seek to a specific time position.

```lua
local t = player:GetTime()    -- returns: number
```
Get the current playback time in seconds.

```lua
local d = player:GetDuration()    -- returns: number
```
Get the duration of the assigned Timeline asset.

### State Queries

```lua
local playing = player:IsPlaying()    -- returns: boolean
```
Returns `true` if the timeline is playing and not paused.

```lua
local paused = player:IsPaused()    -- returns: boolean
```
Returns `true` if playback is paused.

```lua
local progress = player:GetProgress()    -- returns: number (0.0 to 1.0)
```
Returns the playback progress as a normalized value.

### Asset Management

```lua
player:SetTimeline(timelineAsset)    -- timelineAsset: Timeline or nil
```
Assign a Timeline asset. Pass `nil` to clear.

```lua
local tl = player:GetTimeline()    -- returns: Timeline or nil
```
Get the currently assigned Timeline asset.

### Settings

```lua
player:SetPlayOnStart(enabled)    -- enabled: boolean
```
Set whether the timeline should auto-play when the scene starts.

```lua
local auto = player:GetPlayOnStart()    -- returns: boolean
```
Get the auto-play setting.

### Signals and Callbacks

TimelinePlayer emits signals when playback state changes. Connect to them or define script callbacks:

**Using Signals:**
```lua
function Start()
    local player = self:FindChild("TimelinePlayer")
    player:ConnectSignal("OnFinished", self, OnTimelineFinished)
end

function OnTimelineFinished()
    print("Timeline completed!")
end
```

**Using Script Callbacks:**
```lua
-- Define these functions on the TimelinePlayer's script component
function OnStarted()
    print("Playback started")
end

function OnFinished()
    print("Playback finished")
end

function OnStopped()
    print("Playback stopped manually")
end

function OnStateChanged()
    -- Called on any state change
    local player = self
    print("Progress: " .. (player:GetProgress() * 100) .. "%")
end
```

| Signal | When Emitted |
|--------|--------------|
| `OnStarted` | `Play()` is called |
| `OnFinished` | Timeline reaches end (non-looping) |
| `OnStopped` | `Stop()` is called |
| `OnStateChanged` | Any state change |

## Timeline Asset Methods

The `Timeline` class inherits from `Asset`. These methods are available on Timeline asset references.

```lua
local duration = timeline:GetDuration()      -- returns: number
local looping  = timeline:IsLooping()        -- returns: boolean
local rate     = timeline:GetPlayRate()      -- returns: number
local count    = timeline:GetNumTracks()     -- returns: integer
```

## Usage Examples

### Basic Playback

```lua
function Start()
    -- Get the TimelinePlayer node
    self.player = self:GetParent():FindChild("CutscenePlayer")

    -- Start playing
    self.player:Play()
end

function Tick(deltaTime)
    -- Check if playback finished
    if not self.player:IsPlaying() then
        -- Timeline ended
    end
end
```

### Load and Assign a Timeline

```lua
function Start()
    -- Load a Timeline asset
    local tl = LoadAsset("Timelines/T_Intro")

    -- Assign it to the player and start
    local player = self:FindChild("TimelinePlayer")
    player:SetTimeline(tl)
    player:Play()
end
```

### Pause / Resume / Scrub

```lua
function TogglePause()
    if self.player:IsPaused() then
        self.player:Play()
    else
        self.player:Pause()
    end
end

function SkipToMiddle()
    local half = self.player:GetDuration() / 2.0
    self.player:SetTime(half)
end
```

### Query Timeline Info

```lua
function Start()
    local tl = self.player:GetTimeline()
    if tl ~= nil then
        local dur    = tl:GetDuration()
        local loops  = tl:IsLooping()
        local rate   = tl:GetPlayRate()
        local tracks = tl:GetNumTracks()
    end
end
```

### Using FunctionCallTrack to Call Lua Functions

A `FunctionCallTrack` targeting a node with a script component will call the named Lua function on that script at the keyframed time. Define the function in your script:

```lua
-- This function is called by a FunctionCallTrack keyframe
function OnExplosion()
    -- Spawn particle effect, play sound, etc.
    SpawnParticleEffect(self:GetPosition())
end

-- Another function triggered later in the timeline
function OnDoorOpen()
    self.door:SetActive(true)
end
```

In the editor, add a `FunctionCallTrack` targeting this node, then add keyframes with function names `"OnExplosion"` and `"OnDoorOpen"` at the desired times.

## Limitations

- **Track/clip/keyframe data is not accessible from Lua.** You cannot read or modify individual tracks, clips, or keyframes at runtime from scripts. The Lua API is limited to playback control and querying top-level timeline properties.
- **No Lua constants for enums.** Timeline-related enums (`InterpMode`, `AnimationWrapMode`, `AudioClipEndMode`) are not exposed to Lua.
