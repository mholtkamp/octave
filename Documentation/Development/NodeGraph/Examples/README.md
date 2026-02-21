# NodeGraph Lua Examples

Lua scripts demonstrating how to interact with the Octave Engine's NodeGraph system at runtime via the `NodeGraphPlayer` API.

## Prerequisites

- A scene with at least one Node to attach scripts to
- One or more `NodeGraphAsset` files created in the editor (Material, SceneGraph, Procedural, etc.)
- Familiarity with the Octave Lua script lifecycle: `Create()` -> `GatherProperties()` -> `Start()` -> `Tick(dt)` -> `Stop()`

## How to Use

1. **Create a NodeGraphAsset** in the editor using the Node Graph panel
2. **Attach a script** to any Node in your scene (via the Script component property)
3. **Assign the NodeGraphAsset** to the script's exposed property in the Properties panel
4. **Press Play** in the editor to run

Each example file is self-contained. Copy it into your project's `Scripts/` directory or reference it directly.

## Examples by Difficulty

### Beginner

| # | File | Description |
|---|------|-------------|
| 01 | `01_BasicPlayback.lua` | Create a NodeGraphPlayer, assign an asset, and play/stop it |
| 02 | `02_SetInputs.lua` | Feed Float, Vector, and Bool values into graph InputNodes each frame |
| 03 | `03_ReadOutputs.lua` | Read Float, Vector, and Bool values from graph output pins |
| 04 | `04_PauseResume.lua` | Pause, resume, and reset a running graph |

### Intermediate

| # | File | Description |
|---|------|-------------|
| 05 | `05_SwapGraphAtRuntime.lua` | Hot-swap between different NodeGraphAssets at runtime |
| 06 | `06_InputOutputLoop.lua` | Feedback loop: read outputs and feed them back as inputs next frame |
| 07 | `07_AllInputTypes.lua` | Every `SetInput*` type: Float, Int, Bool, String, Vector, Color |
| 08 | `08_MultipleOutputs.lua` | Read multiple output types and apply them to scene nodes |

### Intermediate (continued)

| # | File | Description |
|---|------|-------------|
| 13 | `13_ComparisonAndLogic.lua` | Comparison and boolean logic nodes: Equal, Greater, And, Select |
| 14 | `14_StringProcessing.lua` | String manipulation: Concatenate, ToUpper, Length, Contains |

### Advanced

| # | File | Description |
|---|------|-------------|
| 09 | `09_ProceduralAnimation.lua` | Drive procedural animation from graph-computed position/rotation offsets |
| 10 | `10_MultiGraphOrchestrator.lua` | Run two graphs simultaneously and chain their outputs |
| 11 | `11_StateMachineDriver.lua` | Lua state machine that feeds state info into a graph for per-state behavior |
| 12 | `12_FunctionGraphDemo.lua` | Work with graphs that contain user-defined functions (FunctionCallNodes) |
| 15 | `15_FlowControlPatterns.lua` | Flow control nodes: FlipFlop, DoOnce, Gate, DoN |
| 16 | `16_NodeReferences.lua` | Node/Node3D inputs, conversion (NodeToNode3D), IsValid/IsNull checks |
| 17 | `17_InputDrivenGameplay.lua` | Input system nodes driving gameplay autonomously from the graph |

## NodeGraphPlayer API Reference

### Playback Control

| Method | Description |
|--------|-------------|
| `player:Play()` | Start or resume graph evaluation |
| `player:Pause()` | Pause evaluation (retains state) |
| `player:Stop()` | Stop evaluation |
| `player:Reset()` | Stop and destroy the runtime graph (next Play recreates it) |
| `player:IsPlaying()` | Returns `true` if playing and not paused |
| `player:IsPaused()` | Returns `true` if paused |

### Asset Management

| Method | Description |
|--------|-------------|
| `player:SetNodeGraphAsset(asset)` | Assign a NodeGraphAsset (destroys current runtime graph) |
| `player:GetNodeGraphAsset()` | Returns the currently assigned NodeGraphAsset |
| `player:SetPlayOnStart(bool)` | If true, graph auto-plays when the scene starts |
| `player:GetPlayOnStart()` | Returns the PlayOnStart setting |

### Setting Inputs

Feed values into the graph's InputNodes by name. Returns `true` if the named input was found.

| Method | Lua Types |
|--------|-----------|
| `player:SetInputFloat(name, value)` | `string, number` |
| `player:SetInputInt(name, value)` | `string, number` (truncated to integer) |
| `player:SetInputBool(name, value)` | `string, boolean` |
| `player:SetInputString(name, value)` | `string, string` |
| `player:SetInputVector(name, vec)` | `string, Vec(x,y,z)` |
| `player:SetInputColor(name, vec)` | `string, Vec(r,g,b,a)` |

### Reading Outputs

Read computed values from the graph's OutputNode by **0-based pin index**.

| Method | Returns |
|--------|---------|
| `player:GetOutputFloat(pinIndex)` | `number` |
| `player:GetOutputInt(pinIndex)` | `integer` |
| `player:GetOutputBool(pinIndex)` | `boolean` |
| `player:GetOutputVector(pinIndex)` | `Vec(x,y,z)` |
| `player:GetOutputColor(pinIndex)` | `Vec(r,g,b,a)` |

## Common Patterns

### Creating a player at runtime

```lua
function MyScript:Start()
    self.player = Node.Construct("NodeGraphPlayer")
    self:AddChild(self.player)
    self.player:SetNodeGraphAsset(self.myAsset)
    self.player:Play()
end
```

### Input/output each frame

```lua
function MyScript:Tick(deltaTime)
    self.player:SetInputFloat("Time", self.elapsed)
    local result = self.player:GetOutputFloat(0)
end
```

### Clean shutdown

```lua
function MyScript:Stop()
    self.player:Stop()
    self.player:Reset()
end
```

## Notes

- **InputNode names** must match exactly between your Lua `SetInput*` calls and the InputNode names in the graph editor. Names are case-sensitive.
- **Output pin indices** are 0-based and correspond to the order of pins on the graph's OutputNode (top to bottom in the editor).
- **Functions are transparent**: If your graph uses FunctionCallNodes (user-defined functions), they evaluate automatically during `GraphProcessor::Evaluate()`. No special Lua code is needed.
- **SetNodeGraphAsset** destroys the current runtime graph. Call it before `Play()`, not during playback. To swap graphs mid-play, call `Stop()` and `Reset()` first (see example 05).
- **Performance**: Each `NodeGraphPlayer` maintains its own runtime copy of the graph. For many instances of the same graph, each gets independent state.
