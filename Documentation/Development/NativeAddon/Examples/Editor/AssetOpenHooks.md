# Asset Open Hooks

## Overview

This example demonstrates how to react to asset open events using `RegisterOnAssetOpen` and `RegisterOnAssetOpened`. These hooks fire when an asset is double-clicked in the Asset Panel, allowing addons to perform pre-open validation, post-open setup, analytics tracking, or custom handling for specific asset types.

## Files

### package.json

```json
{
    "name": "Asset Open Watcher Addon",
    "author": "Octave Examples",
    "description": "Monitors asset open and opened events.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "assetopenwatcheraddon",
        "apiVersion": 2
    }
}
```

### Source/AssetOpenWatcherAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
/**
 * @brief Called just before an asset is opened (double-clicked in Asset Panel).
 * @param assetName Name of the asset about to be opened.
 * @param userData User-provided data.
 *
 * Fires before the asset is loaded or displayed. Use this for
 * pre-open validation, logging, or preparing state.
 */
static void OnAssetOpen(const char* assetName, void* userData)
{
    sEngineAPI->LogDebug("Asset opening: %s", assetName);

    // Example: Track which assets are being accessed
    sEngineAPI->LogDebug("Recording asset access for analytics...");

    // Example: Prepare addon state before asset loads
    sEngineAPI->LogDebug("Preparing workspace for asset...");
}

/**
 * @brief Called after an asset has been opened (loaded and displayed).
 * @param assetName Name of the asset that was opened.
 * @param userData User-provided data.
 *
 * Fires after the asset has been loaded and the appropriate editor
 * action has been taken (scene opened, timeline displayed, or
 * asset inspected in the Properties panel).
 */
static void OnAssetOpened(const char* assetName, void* userData)
{
    sEngineAPI->LogDebug("Asset opened: %s", assetName);

    // Example: Update addon UI to reflect the opened asset
    sEngineAPI->LogDebug("Updating addon panel for opened asset...");

    // Example: Run post-open validation
    sEngineAPI->LogDebug("Validating asset integrity...");
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
    api->LogDebug("Asset Open Watcher Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Asset Open Watcher Addon unloading.");
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
    hooks->RegisterOnAssetOpen(hookId, OnAssetOpen, nullptr);
    hooks->RegisterOnAssetOpened(hookId, OnAssetOpened, nullptr);
    sEngineAPI->LogDebug("Asset open hooks registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Asset Open Watcher Addon";
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

### RegisterOnAssetOpen

```cpp
void (*RegisterOnAssetOpen)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked just before an asset is opened in the editor.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call before the asset opens
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User double-clicks an asset in the Asset Panel
- Fires before the asset is loaded or any editor action is taken

### RegisterOnAssetOpened

```cpp
void (*RegisterOnAssetOpened)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked after an asset has been opened in the editor.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call after the asset opens
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User double-clicks an asset in the Asset Panel
- Fires after the asset has been loaded and the editor has acted on it (scene opened, timeline displayed, or properties inspected)

### StringEventCallback

```cpp
typedef void (*StringEventCallback)(const char* str, void* userData);
```

Callback signature for events that provide a string parameter.

**Parameters:**
- `str` - String data (asset name in these cases)
- `userData` - User-provided data from registration

## Best Practices

1. **Pre-Open Logic** - Use `OnAssetOpen` for validation or state preparation before the asset loads
2. **Post-Open Logic** - Use `OnAssetOpened` for UI updates or follow-up actions after the asset is ready
3. **Null Checks** - Validate the `assetName` parameter before using it
4. **Performance** - Keep callbacks fast to avoid delaying asset opening
5. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
6. **Asset Types** - The callback receives the asset name; use the engine API to query the asset type if needed
