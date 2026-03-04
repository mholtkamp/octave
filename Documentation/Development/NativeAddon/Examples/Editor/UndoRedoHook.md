# Undo/Redo Hook

## Overview

This example demonstrates how to react to undo and redo operations using `RegisterOnUndoRedo`. This hook enables addons to synchronize their state with the editor's undo/redo system, ensuring addon UI and data remain consistent with editor operations.

## Files

### package.json

```json
{
    "name": "Undo Sync Addon",
    "author": "Octave Examples",
    "description": "Syncs addon state with undo/redo operations.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "undosyncaddon",
        "apiVersion": 2
    }
}
```

### Source/UndoSyncAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static int sTotalOperations = 0;

/**
 * @brief Called when an undo or redo operation occurs.
 * @param userData User-provided data (unused in this example).
 *
 * This callback is invoked for both undo (Ctrl+Z) and redo (Ctrl+Y)
 * operations. Use this to refresh addon state that depends on scene
 * or asset state.
 *
 * @note The callback does not indicate whether it was undo or redo.
 * If you need to distinguish, maintain your own history stack.
 */
static void OnUndoRedo(void* userData)
{
    sTotalOperations++;
    sEngineAPI->LogDebug("Undo/Redo operation (#%d)", sTotalOperations);

    // Example: Refresh cached data that may have changed
    sEngineAPI->LogDebug("Refreshing addon cache...");

    // Example: Update UI to reflect new state
    sEngineAPI->LogDebug("Updating addon UI...");

    // Example: Revalidate addon-specific data
    sEngineAPI->LogDebug("Revalidating addon state...");
}

/**
 * @brief Draw callback for an undo stats menu.
 * @param userData User-provided data (unused in this example).
 */
static void DrawUndoStatsMenu(void* userData)
{
    char label[64];
    snprintf(label, sizeof(label), "Operations: %d", sTotalOperations);
    ImGui::MenuItem(label, nullptr, false, false);

    ImGui::Separator();

    if (ImGui::MenuItem("Reset Counter"))
    {
        sTotalOperations = 0;
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
    api->LogDebug("Undo Sync Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Undo Sync Addon unloading.");
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
    hooks->RegisterOnUndoRedo(hookId, OnUndoRedo, nullptr);
    sEngineAPI->LogDebug("Undo/redo hook registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Undo Sync Addon";
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

### RegisterOnUndoRedo

```cpp
void (*RegisterOnUndoRedo)(HookId hookId, EventCallback cb, void* userData);
```

Registers a callback invoked when an undo or redo operation occurs.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when undo/redo happens
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User performs undo (Ctrl+Z)
- User performs redo (Ctrl+Y)
- Undo/redo is triggered programmatically

### EventCallback

```cpp
typedef void (*EventCallback)(void* userData);
```

Callback signature for parameterless events.

**Parameters:**
- `userData` - User-provided data from registration

## Important Notes

1. **No Direction Info** - The callback does not indicate whether it was undo or redo
2. **Frequent Calls** - May be called frequently during rapid undo/redo sequences
3. **State Refresh** - Always refresh addon state that depends on scene/asset data
4. **Performance** - Keep callbacks fast to avoid UI lag during undo/redo

## Best Practices

1. **Cache Invalidation** - Use this hook to invalidate caches that depend on editor state
2. **Lazy Refresh** - Defer expensive updates until the data is actually needed
3. **Selective Updates** - Only refresh what actually changed, if possible
4. **UI Responsiveness** - Keep the callback fast to maintain editor responsiveness
5. **Custom History** - If you need to track undo vs redo, maintain your own history stack
6. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
7. **Batch Updates** - If multiple operations occur rapidly, consider debouncing updates
