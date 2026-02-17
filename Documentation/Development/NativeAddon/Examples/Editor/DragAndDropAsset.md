# Drag-and-Drop Asset Hooks

## Overview

This example demonstrates how to hook into asset drag-and-drop events using `RegisterOnAssetDropHierarchy` and `RegisterOnAssetDropViewport`. These hooks fire after a user drags a StaticMesh asset from the asset browser and drops it into the scene hierarchy or viewport, and after the spawned node has been created. The newly created node is the selected node, so addons can query `GetSelectedNode()` for post-processing.

## Files

### package.json

```json
{
    "name": "Drag and Drop Asset Addon",
    "author": "Octave Examples",
    "description": "Hooks into asset drag-and-drop events for post-processing.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "dragdropassetaddon",
        "apiVersion": 2
    }
}
```

### Source/DragDropAssetAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
/**
 * @brief Called when an asset is dropped onto the scene hierarchy.
 * @param assetName Name of the dropped asset.
 * @param userData User-provided data.
 */
static void OnAssetDropHierarchy(const char* assetName, void* userData)
{
    sEngineAPI->LogDebug("Asset dropped on hierarchy: %s", assetName);

    // Example: The spawned node is now the selected node.
    // You could query GetSelectedNode() to apply custom transforms,
    // attach components, or set default properties.
}

/**
 * @brief Called when an asset is dropped onto the viewport.
 * @param assetName Name of the dropped asset.
 * @param userData User-provided data.
 */
static void OnAssetDropViewport(const char* assetName, void* userData)
{
    sEngineAPI->LogDebug("Asset dropped on viewport: %s", assetName);

    // Example: Perform raycast to place the node at the drop position,
    // or apply viewport-specific default settings.
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
    api->LogDebug("Drag and Drop Asset Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Drag and Drop Asset Addon unloading.");
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
    hooks->RegisterOnAssetDropHierarchy(hookId, OnAssetDropHierarchy, nullptr);
    hooks->RegisterOnAssetDropViewport(hookId, OnAssetDropViewport, nullptr);
    sEngineAPI->LogDebug("Drag-and-drop asset hooks registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Drag and Drop Asset Addon";
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

### RegisterOnAssetDropHierarchy

```cpp
void (*RegisterOnAssetDropHierarchy)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a StaticMesh asset is dragged from the asset browser and dropped onto the scene hierarchy panel.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when an asset is dropped on the hierarchy
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User drags a StaticMesh asset onto a hierarchy tree node (spawns as child)
- User drags a StaticMesh asset onto empty hierarchy space (spawns at root)

### RegisterOnAssetDropViewport

```cpp
void (*RegisterOnAssetDropViewport)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a StaticMesh asset is dragged from the asset browser and dropped onto the 3D viewport.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when an asset is dropped on the viewport
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User drags a StaticMesh asset onto the 3D viewport

### StringEventCallback

```cpp
typedef void (*StringEventCallback)(const char* str, void* userData);
```

Callback signature for events that provide a string parameter.

**Parameters:**
- `str` - The asset name of the dropped StaticMesh
- `userData` - User-provided data from registration

## Best Practices

1. **Selected Node** - After the hook fires, the spawned node is the selected node; use `GetSelectedNode()` for post-processing
2. **Mesh Type** - The user chooses between Static Mesh and Instanced Mesh via a popup before the hook fires
3. **Performance** - Keep callbacks fast; the hook fires on the main thread during UI drawing
4. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
