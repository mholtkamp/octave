# 3DS Dual-Screen System

## Overview

The Nintendo 3DS has two physical screens:

| Screen | Resolution | Features |
|--------|-----------|----------|
| **Top** | 400 x 240 | Stereoscopic 3D (left/right eye rendering) |
| **Bottom** | 320 x 240 | Touch input, mono only |

The hardware 3D slider on the side of the console controls stereoscopic depth. When the slider is at 0, the top screen renders a single mono view. When the slider is above 0, the top screen renders two views (left eye and right eye) to produce a stereoscopic 3D effect.

The New Nintendo 3DS variant is detected at startup via `APT_CheckNew3DS()`, which enables CPU speedup (`osSetSpeedupEnable(true)`).

## Architecture: World-per-Screen

On 3DS, Octave creates **two World objects** at startup -- one per screen:

```
World 0  -->  Top screen (screen index 0)
World 1  -->  Bottom screen (screen index 1)
```

Each world has its own independent scene graph, camera, physics simulation, and node hierarchy. During the render loop, the engine iterates over both worlds:

```cpp
for (int32_t i = 0; i < int32_t(sWorlds.size()); ++i)
{
    Renderer::Get()->Render(sWorlds[i], i);
}
```

The world index directly maps to the screen index. This mapping is fixed and cannot be changed at runtime.

The `SUPPORTS_SECOND_SCREEN` constant is set to `1` only for the Citro3D graphics backend (3DS). All other backends set it to `0`.

## Stereoscopic 3D

### How It Works

The 3DS graphics backend creates three render targets:

| Render Target | Size (W x H) | Output |
|---------------|--------------|--------|
| `mRenderTargetLeft` | 240 x 400 | `GFX_TOP`, `GFX_LEFT` |
| `mRenderTargetRight` | 240 x 400 | `GFX_TOP`, `GFX_RIGHT` |
| `mRenderTargetBottom` | 240 x 320 | `GFX_BOTTOM`, `GFX_LEFT` |

Note: Render target dimensions are swapped (height x width) because the 3DS screen is physically rotated.

### 3D Slider and IOD

Each frame, `SYS_Update()` reads the 3D slider position via `osGet3DSliderState()` and stores it in `SystemState::mSlider` (a float, roughly 0.0 to 1.0).

The **interocular distance (IOD)** is computed from the slider value:

```
IOD = slider / 3
```

- **Left eye** (view 0): IOD is negated (`IOD * -1`)
- **Right eye** (view 1): IOD is used as-is

The IOD feeds into `Mtx_PerspStereoTilt()` to produce offset perspective matrices for each eye, creating the stereoscopic depth effect.

### View Count

`GFX_GetNumViews()` returns:

- **2** for the top screen (screen 0) when the 3D slider is above 0
- **1** otherwise (slider at 0, or bottom screen)

The bottom screen always renders a single view.

## Screen Dimensions

Screen dimensions are stored in `EngineState`:

| Field | Value | Description |
|-------|-------|-------------|
| `mWindowWidth` | 400 | Top screen width |
| `mWindowHeight` | 240 | Top screen height |
| `mSecondWindowWidth` | 320 | Bottom screen width |
| `mSecondWindowHeight` | 240 | Bottom screen height |

These values are set during `SYS_Initialize()` and do not change at runtime. The scissor test in the graphics backend uses these widths (400 vs 320) to correctly clip rendering for each screen.

## Lua API

### Getting a World by Screen

Use `Engine.GetWorld(index)` to get the world associated with a screen. Lua uses **1-based indexing**:

```lua
local topWorld = Engine.GetWorld(1)      -- World for top screen
local bottomWorld = Engine.GetWorld(2)   -- World for bottom screen
```

### Loading Scenes to a Screen

Each world can load its own scene independently:

```lua
-- Load a gameplay scene on the top screen
Engine.GetWorld(1):LoadScene("Gameplay")

-- Load a HUD/map scene on the bottom screen
Engine.GetWorld(2):LoadScene("BottomHUD")
```

### Querying Screen State

```lua
-- Get the screen index currently being rendered (0 or 1)
local screenIdx = Renderer.GetScreenIndex()

-- Get resolution of a specific screen (1-indexed)
local topRes = Renderer.GetScreenResolution(1)    -- Vector(400, 240)
local botRes = Renderer.GetScreenResolution(2)    -- Vector(320, 240)

-- Get resolution of the screen currently being rendered
local activeRes = Renderer.GetActiveScreenResolution()
```

### Adaptive UI Example

Because the two screens have different widths (400 vs 320), you may need to adjust widget layouts:

```lua
function OnStart(self)
    local res = Renderer.GetActiveScreenResolution()

    -- Center a widget horizontally regardless of screen width
    local widget = self:FindChild("StatusBar")
    widget:SetPosition(res.x / 2, widget:GetPosition().y)
end
```

## New 3DS Detection

The `SystemState::mNew3DS` boolean indicates whether the game is running on a New Nintendo 3DS. This is detected at startup via `APT_CheckNew3DS()`.

On a New 3DS, the engine enables CPU speedup (`osSetSpeedupEnable(true)`) automatically. The platform tier reported by `SYS_GetPlatformTier()` is:

- **1** on New 3DS
- **0** on original 3DS

## Limitations

The following aspects are **hardcoded and cannot be changed at runtime**:

- **Screen resolutions** -- 400x240 (top) and 320x240 (bottom) are fixed by hardware
- **Number of screens** -- always 2 on 3DS
- **World-to-screen mapping** -- world index always equals screen index
- **Stereoscopic 3D** -- always enabled on the top screen; the hardware slider is the only control
- **No wide mode** or screen layout configuration API

What developers **can** control:

- Which scene loads on which screen (via `Engine.GetWorld(index):LoadScene()`)
- Per-screen camera setup (each world has its own active camera)
- Querying screen dimensions and current screen index for adaptive UI
- Independent scene graphs, physics, and node hierarchies per world

## Editor 3DS Preview Filtering

### How It Works

During Play-In-Editor, all scenes live in a single shared game world. The 3DS Preview panel renders this world twice (once per screen), using `mTargetScreenFilter` on the Renderer to filter nodes by their `mTargetScreen` property at the scene-root level.

The filter is applied in `GatherDrawData()`: when `mTargetScreenFilter >= 0`, direct children of the world root whose `GetTargetScreen()` does not match the filter are skipped entirely (including their whole subtree of 3D nodes and widgets).

On actual 3DS hardware, the two worlds are already separate, so the filter acts as a safety net (set to `screenIndex` via `SUPPORTS_SECOND_SCREEN`).

### Scene Panel Screen Filter

The Scene Panel includes a **Screen Filter** combo dropdown with three options:

- **All Screens** (default) -- shows all nodes in both the hierarchy and viewport
- **Top Screen** -- only shows subtrees whose root has `mTargetScreen == 0`
- **Bottom Screen** -- only shows subtrees whose root has `mTargetScreen == 1`

This filter affects:
- The scene hierarchy tree (subtrees are hidden)
- The editor viewport (via `mTargetScreenFilter` in `GatherDrawData()`)

During PIE, the viewport filter is disabled (shows everything), but the 3DS Preview panel still applies per-screen filtering.

### Target Screen Property

Set `mTargetScreen` on scene root nodes to control which 3DS screen the subtree renders on. The convention matches `FindSceneForScreen()`: root children with `GetTargetScreen() == 0` are top-screen, `1` are bottom-screen.

### UIDocument Interaction

When a UIDocument is mounted to a widget via `UIDocument::Mount()`, the entire widget tree inherits the parent's `mTargetScreen` value. This ensures consistency even though the scene-root subtree filter already handles rendering.

## Key Source Files

| File | Contents |
|------|----------|
| `Engine/Source/System/3DS/System_3DS.cpp` | Platform init, 3D slider read, New 3DS detection |
| `Engine/Source/Graphics/C3D/Graphics_C3D.cpp` | Render targets, stereo rendering, scissor/viewport |
| `Engine/Source/Graphics/C3D/C3dTypes.h` | `C3dContext` struct (render targets, IOD, current screen) |
| `Engine/Source/Graphics/GraphicsConstants.h` | `SUPPORTS_SECOND_SCREEN` constant |
| `Engine/Source/Engine/Engine.cpp` | World creation and render loop |
| `Engine/Source/Engine/Renderer.cpp` | Screen index tracking, resolution queries, `mTargetScreenFilter` |
| `Engine/Source/Engine/EngineTypes.h` | `mWindowWidth/Height`, `mSecondWindowWidth/Height` |
| `Engine/Source/System/SystemTypes.h` | `SystemState` 3DS fields (`mSlider`, `mNew3DS`) |
| `Engine/Source/LuaBindings/Renderer_Lua.cpp` | `GetScreenIndex`, `GetScreenResolution` bindings |
| `Engine/Source/LuaBindings/Engine_Lua.cpp` | `Engine.GetWorld()` binding |
| `Engine/Source/Editor/SecondScreenPreview/SecondScreenPreview.cpp` | 3DS Preview panel, PIE screen filtering |
| `Engine/Source/Editor/EditorState.h` | `mSceneScreenFilter` for Scene Panel filter |
