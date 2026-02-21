# NodeGraphPlayer

`NodeGraphPlayer` is a scene node that plays a `NodeGraphAsset` at runtime. It creates an internal copy of the graph, evaluates it every frame while playing, and exposes input/output access so scripts can feed data into the graph and read computed results back out.

## Editor Properties

| Property | Type | Description |
|---|---|---|
| Node Graph | Asset (NodeGraphAsset) | The graph asset to play |
| Play On Start | Bool | When true, playback begins automatically on scene start |

## Lifecycle

1. **Scene starts** - If `PlayOnStart` is enabled, `Play()` is called automatically.
2. **Every tick** - While playing and not paused, the runtime graph is evaluated by the `GraphProcessor`. After evaluation, the graph's domain receives an `OnGraphEvaluated` callback.
3. **Scene stops** - Playback stops and the runtime graph is cleaned up on `Destroy()`.

The runtime graph is a copy of the asset's graph, created lazily on the first call to `Play()` or any `SetInput*` method. Changing the asset via `SetNodeGraphAsset()` discards the existing runtime graph so a fresh copy is created next time.

## Playback Control

| Method | Description |
|---|---|
| `Play()` | Begin evaluation. Creates the runtime graph if needed. |
| `Pause()` | Suspend evaluation. The runtime graph and its state are preserved. |
| `Stop()` | Stop evaluation and clear the playing/paused flags. State is preserved. |
| `Reset()` | Stop evaluation and destroy the runtime graph. All state is lost. |
| `IsPlaying()` | Returns `true` if the graph is actively evaluating (playing and not paused). |
| `IsPaused()` | Returns `true` if paused. |

### Playback State Diagram

```
           Play()         Pause()
 Stopped ---------> Playing ---------> Paused
    ^                  |                  |
    |    Stop()        |     Stop()       |
    +------------------+------------------+
    |                                     |
    |              Reset()                |
    +--------  (destroys runtime graph)---+
```

## Setting Inputs

Use the `SetInput*` methods to push values into the graph's input pins by name. The name must match an input pin defined on the graph. All setters return `true` on success, `false` if the named input was not found.

| Method | Value Type |
|---|---|
| `SetInputFloat(name, value)` | `float` |
| `SetInputInt(name, value)` | `int32` |
| `SetInputBool(name, value)` | `bool` |
| `SetInputString(name, value)` | `string` |
| `SetInputVector(name, value)` | `vec3` / `Vector` |
| `SetInputColor(name, value)` | `vec4` / `Vector` (RGBA) |
| `SetInputByte(name, value)` | `uint8` (C++ only) |
| `SetInputAsset(name, asset)` | `Asset*` (C++ only) |

Calling any setter before `Play()` is safe - it will create the runtime graph automatically.

## Reading Outputs

Use the `GetOutput*` methods to read values from the graph's output pins after evaluation. Outputs are accessed by pin index (0-based). If the graph has not been evaluated yet, default values are returned (0, false, zero vectors).

| Method | Return Type | Default |
|---|---|---|
| `GetOutputFloat(pinIndex)` | `float` | `0.0` |
| `GetOutputInt(pinIndex)` | `int32` | `0` |
| `GetOutputBool(pinIndex)` | `bool` | `false` |
| `GetOutputVector(pinIndex)` | `vec3` / `Vector` | `(0, 0, 0)` |
| `GetOutputColor(pinIndex)` | `vec4` / `Vector` | `(0, 0, 0, 0)` |

## Lua API

`NodeGraphPlayer` is fully accessible from Lua. It inherits all `Node` methods.

### Example: Basic Playback

```lua
local player = self:GetNode():FindChild("MyGraphPlayer", true)
player:Play()
```

### Example: Driving Inputs from Script

```lua
function OnTick(deltaTime)
    local player = self:GetNode():FindChild("MyGraphPlayer", true)

    -- Feed game state into the graph
    player:SetInputFloat("Speed", GetPlayerSpeed())
    player:SetInputBool("IsGrounded", IsPlayerGrounded())
    player:SetInputVector("Position", GetPlayerPosition())

    -- Read computed result
    local color = player:GetOutputColor(0)
    ApplyColor(color)
end
```

### Example: Swapping Graph Assets

```lua
local player = self:GetNode():FindChild("MyGraphPlayer", true)

-- Stop and clear current graph state
player:Reset()

-- Assign a new asset and play
local newAsset = Asset.Load("graphs/DamageFeedback")
player:SetNodeGraphAsset(newAsset)
player:Play()
```

### Example: Pause / Resume

```lua
function OnPauseToggle()
    local player = self:GetNode():FindChild("GraphPlayer", true)
    if player:IsPlaying() then
        player:Pause()
    elseif player:IsPaused() then
        player:Play()
    end
end
```

### Lua Method Reference

| Method | Parameters | Returns |
|---|---|---|
| `Play()` | - | - |
| `Pause()` | - | - |
| `Stop()` | - | - |
| `Reset()` | - | - |
| `IsPlaying()` | - | `boolean` |
| `IsPaused()` | - | `boolean` |
| `SetNodeGraphAsset(asset)` | `Asset?` | - |
| `GetNodeGraphAsset()` | - | `Asset` |
| `SetInputFloat(name, value)` | `string`, `number` | `boolean` |
| `SetInputInt(name, value)` | `string`, `number` | `boolean` |
| `SetInputBool(name, value)` | `string`, `boolean` | `boolean` |
| `SetInputString(name, value)` | `string`, `string` | `boolean` |
| `SetInputVector(name, value)` | `string`, `Vector` | `boolean` |
| `SetInputColor(name, value)` | `string`, `Vector` | `boolean` |
| `GetOutputFloat(pinIndex)` | `number` | `number` |
| `GetOutputInt(pinIndex)` | `number` | `integer` |
| `GetOutputBool(pinIndex)` | `number` | `boolean` |
| `GetOutputVector(pinIndex)` | `number` | `Vector` |
| `GetOutputColor(pinIndex)` | `number` | `Vector` |
| `SetPlayOnStart(v)` | `boolean` | - |
| `GetPlayOnStart()` | - | `boolean` |

## C++ Usage

```cpp
#include "Nodes/NodeGraphPlayer.h"
#include "Assets/NodeGraphAsset.h"

// Create and configure
NodeGraphPlayer* player = GetWorld()->SpawnNode<NodeGraphPlayer>();
player->SetNodeGraphAsset(myGraphAsset);
player->SetPlayOnStart(true);

// Or control manually
player->Play();
player->SetInputFloat("Intensity", 0.75f);

// After evaluation, read outputs
float result = player->GetOutputFloat(0);
glm::vec3 direction = player->GetOutputVector(1);
```
