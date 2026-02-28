# Selection Change Handler

## Overview

This example demonstrates how to react to selection changes in the editor using `RegisterOnSelectionChanged`. This hook enables addons to update their UI, load relevant data, or perform actions when the user selects different objects in the scene or asset browser.

## Files

### package.json

```json
{
    "name": "Selection Monitor Addon",
    "author": "Octave Examples",
    "description": "Monitors and reacts to selection changes.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "selectionmonitoraddon",
        "apiVersion": 2
    }
}
```

### Source/SelectionMonitorAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static int sSelectionChangeCount = 0;

/**
 * @brief Called when the editor selection changes.
 * @param userData User-provided data (unused in this example).
 *
 * This callback is invoked whenever:
 * - The user selects/deselects objects in the scene
 * - The user selects assets in the content browser
 * - Selection is changed programmatically
 * - Selection is cleared
 */
static void OnSelectionChanged(void* userData)
{
    sSelectionChangeCount++;
    sEngineAPI->LogDebug("Selection changed (#%d)", sSelectionChangeCount);

    // Example: Query the new selection via the editor API
    sEngineAPI->LogDebug("Loading properties for selected object...");

    // Example: Update addon UI to reflect new selection
    sEngineAPI->LogDebug("Refreshing custom inspector...");
}

/**
 * @brief Draw callback for a custom top-level menu showing selection stats.
 * @param userData User-provided data (unused in this example).
 */
static void DrawSelectionMenu(void* userData)
{
    char label[64];
    snprintf(label, sizeof(label), "Changes: %d", sSelectionChangeCount);
    ImGui::MenuItem(label, nullptr, false, false);

    ImGui::Separator();

    if (ImGui::MenuItem("Reset Counter"))
    {
        sSelectionChangeCount = 0;
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
    api->LogDebug("Selection Monitor Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Selection Monitor Addon unloading.");
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
    hooks->RegisterOnSelectionChanged(hookId, OnSelectionChanged, nullptr);
    hooks->AddTopLevelMenuItem(hookId, "Selection", DrawSelectionMenu, nullptr);
    sEngineAPI->LogDebug("Selection change handler registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Selection Monitor Addon";
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

### RegisterOnSelectionChanged

```cpp
void (*RegisterOnSelectionChanged)(HookId hookId, EventCallback cb, void* userData);
```

Registers a callback invoked when the editor selection changes.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when selection changes
- `userData` - Optional user data passed to the callback

**Triggered when:**
- Objects are selected/deselected in the scene
- Assets are selected in the content browser
- Selection is modified programmatically
- Selection is cleared

### EventCallback

```cpp
typedef void (*EventCallback)(void* userData);
```

Callback signature for parameterless events.

**Parameters:**
- `userData` - User-provided data from registration

## Best Practices

1. **Query Selection** - Use the editor API to get details about what is actually selected
2. **Performance** - Keep callbacks fast, as they fire frequently during multi-select operations
3. **Deferred Updates** - For expensive operations, defer work until the next frame
4. **Multi-Selection** - Handle both single and multi-selection cases
5. **Empty Selection** - Always handle the case where selection is cleared
6. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
7. **State Sync** - Keep addon state synchronized with editor selection
