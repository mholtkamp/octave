# Play Mode Hooks

## Overview

This example demonstrates how to react to Play-In-Editor (PIE) state changes using `RegisterOnPlayModeChanged`. This hook enables addons to perform setup/cleanup when entering or exiting play mode, and to handle the eject operation when detaching from the running game.

## Files

### package.json

```json
{
    "name": "Play Mode Monitor Addon",
    "author": "Octave Examples",
    "description": "Monitors play mode state transitions.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "playmodemonitoraddon",
        "apiVersion": 2
    }
}
```

### Source/PlayModeMonitorAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static bool sIsInPlayMode = false;
static int sPlayModeEnterCount = 0;

/**
 * @brief Called when play mode state changes.
 * @param state Play mode state (0=Enter, 1=Exit, 2=Eject).
 * @param userData User-provided data.
 *
 * State values:
 *   0 = Enter Play Mode - Simulation has started
 *   1 = Exit Play Mode  - Simulation has stopped, returning to edit mode
 *   2 = Eject           - Detaching editor from running game instance
 */
static void OnPlayModeChanged(int32_t state, void* userData)
{
    switch (state)
    {
    case 0: // Enter Play Mode
        sIsInPlayMode = true;
        sPlayModeEnterCount++;
        sEngineAPI->LogDebug("Entering play mode (session #%d)", sPlayModeEnterCount);

        // Example: Start profiling/metrics collection
        sEngineAPI->LogDebug("Starting performance metrics collection...");
        break;

    case 1: // Exit Play Mode
        sIsInPlayMode = false;
        sEngineAPI->LogDebug("Exiting play mode");

        // Example: Stop profiling and display results
        sEngineAPI->LogDebug("Stopping performance metrics...");

        // Example: Restore editor state
        sEngineAPI->LogDebug("Restoring editor state...");
        break;

    case 2: // Eject
        sIsInPlayMode = false;
        sEngineAPI->LogDebug("Ejecting from play mode");

        // Example: Detach debugging tools
        sEngineAPI->LogDebug("Game instance detached, running independently");
        break;

    default:
        sEngineAPI->LogDebug("Unknown play mode state: %d", state);
        break;
    }
}
#endif

/**
 * @brief Called when the plugin is loaded by the engine.
 * @param api Pointer to the engine API.
 * @return 0 on success, non-zero on failure.
 */
static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("Play Mode Monitor Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Play Mode Monitor Addon unloading.");
    sEngineAPI = nullptr;
}

#if EDITOR
/**
 * @brief Register editor UI hooks.
 * @param hooks Pointer to the EditorUIHooks function table.
 * @param hookId Unique identifier for this plugin's hooks.
 *
 * All hooks registered here are automatically cleaned up via
 * RemoveAllHooks(hookId) when the plugin unloads.
 */
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    hooks->RegisterOnPlayModeChanged(hookId, OnPlayModeChanged, nullptr);
    sEngineAPI->LogDebug("Play mode change handler registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Play Mode Monitor Addon";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;
    desc->TickEditor = nullptr;
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = nullptr;
#if EDITOR
    desc->RegisterEditorUI = RegisterEditorUI;
#else
    desc->RegisterEditorUI = nullptr;
#endif
    desc->OnEditorPreInit = nullptr;
    desc->OnEditorReady = nullptr;
    return 0;
}
```

## API Reference

### RegisterOnPlayModeChanged

```cpp
void (*RegisterOnPlayModeChanged)(HookId hookId, PlayModeCallback cb, void* userData);
```

Registers a callback invoked when play mode state changes.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when play mode changes
- `userData` - Optional user data passed to the callback

### PlayModeCallback

```cpp
typedef void (*PlayModeCallback)(int32_t state, void* userData);
```

Callback signature for play mode state changes.

**Parameters:**
- `state` - Play mode state:
  - `0` = Enter Play Mode (simulation started)
  - `1` = Exit Play Mode (returning to edit mode)
  - `2` = Eject (detaching from running game)
- `userData` - User-provided data from registration

## Play Mode States

| State | Value | Description |
|-------|-------|-------------|
| Enter | 0 | Simulation has started. Use for starting profiling, saving editor state, disabling edit-only UI. |
| Exit  | 1 | Returning to edit mode. Use for stopping profiling, restoring state, re-enabling editor UI. |
| Eject | 2 | Detaching from running game. The game continues running independently. |

## Best Practices

1. **State Tracking** - Maintain a boolean flag to track current play mode state
2. **Resource Management** - Clean up play-mode-specific resources on exit
3. **Editor State** - Save/restore editor configuration when entering/exiting play mode
4. **Performance** - Use play mode transitions for starting/stopping profiling
5. **UI Updates** - Show/hide or enable/disable UI based on play mode state
6. **Eject Support** - Handle the eject case for standalone debugging scenarios
7. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
