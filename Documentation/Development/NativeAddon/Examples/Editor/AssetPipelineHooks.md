# Asset Pipeline Hooks

## Overview

This example demonstrates how to hook into the asset pipeline using `RegisterOnAssetImported`, `RegisterOnAssetDeleted`, and `RegisterOnAssetSaved`. These hooks enable addons to automate asset workflows, perform post-processing, validate assets, and maintain asset databases.

## Files

### package.json

```json
{
    "name": "Asset Pipeline Addon",
    "author": "Octave Examples",
    "description": "Hooks into asset import/save/delete workflow.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "assetpipelineaddon",
        "apiVersion": 2
    }
}
```

### Source/AssetPipelineAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

#include <string.h>

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static int sImportCount = 0;
static int sSaveCount = 0;
static int sDeleteCount = 0;

/**
 * @brief Called when an asset is imported into the project.
 * @param assetPath Absolute path to the imported asset.
 * @param userData User-provided data.
 */
static void OnAssetImported(const char* assetPath, void* userData)
{
    sImportCount++;
    sEngineAPI->LogDebug("Asset imported (#%d): %s", sImportCount, assetPath);

    // Example: Determine asset type from extension
    const char* ext = strrchr(assetPath, '.');
    if (ext)
    {
        if (strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0)
        {
            sEngineAPI->LogDebug("Processing texture asset...");
        }
        else if (strcmp(ext, ".fbx") == 0 || strcmp(ext, ".obj") == 0)
        {
            sEngineAPI->LogDebug("Processing mesh asset...");
        }
    }

    // Example: Update asset database
    sEngineAPI->LogDebug("Updating asset registry...");
}

/**
 * @brief Called when an asset is saved.
 * @param assetPath Absolute path to the saved asset.
 * @param userData User-provided data.
 */
static void OnAssetSaved(const char* assetPath, void* userData)
{
    sSaveCount++;
    sEngineAPI->LogDebug("Asset saved (#%d): %s", sSaveCount, assetPath);

    // Example: Refresh dependent assets
    sEngineAPI->LogDebug("Refreshing dependent assets...");

    // Example: Invalidate cached data
    sEngineAPI->LogDebug("Invalidating asset cache...");
}

/**
 * @brief Called when an asset is deleted from the project.
 * @param assetPath Absolute path to the deleted asset.
 * @param userData User-provided data.
 */
static void OnAssetDeleted(const char* assetPath, void* userData)
{
    sDeleteCount++;
    sEngineAPI->LogDebug("Asset deleted (#%d): %s", sDeleteCount, assetPath);

    // Example: Remove from asset database
    sEngineAPI->LogDebug("Removing from asset registry...");

    // Example: Check for broken references
    sEngineAPI->LogDebug("Scanning for broken asset references...");
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
    api->LogDebug("Asset Pipeline Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Asset Pipeline Addon unloading.");
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
    hooks->RegisterOnAssetImported(hookId, OnAssetImported, nullptr);
    hooks->RegisterOnAssetSaved(hookId, OnAssetSaved, nullptr);
    hooks->RegisterOnAssetDeleted(hookId, OnAssetDeleted, nullptr);
    sEngineAPI->LogDebug("Asset pipeline hooks registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Asset Pipeline Addon";
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

### RegisterOnAssetImported

```cpp
void (*RegisterOnAssetImported)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when an asset is imported into the project.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when an asset is imported
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User drags files into the content browser
- Assets are imported via import dialog
- Assets are copied into the project folder

### RegisterOnAssetDeleted

```cpp
void (*RegisterOnAssetDeleted)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when an asset is deleted from the project.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when an asset is deleted
- `userData` - Optional user data passed to the callback

### RegisterOnAssetSaved

```cpp
void (*RegisterOnAssetSaved)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when an asset is saved.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when an asset is saved
- `userData` - Optional user data passed to the callback

### StringEventCallback

```cpp
typedef void (*StringEventCallback)(const char* str, void* userData);
```

Callback signature for events that provide a string parameter.

**Parameters:**
- `str` - String data (asset file path in these cases)
- `userData` - User-provided data from registration

## Best Practices

1. **Asset Type Detection** - Use file extensions to determine asset type and apply type-specific logic
2. **Performance** - Keep callbacks fast; defer heavy processing to background threads
3. **Error Handling** - Validate asset paths and handle missing files gracefully
4. **Reference Checking** - On delete, warn about or scan for broken references
5. **Metadata Cleanup** - Clean up all generated files when assets are deleted
6. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
7. **Batch Awareness** - Multiple assets may be imported in quick succession; keep callbacks lightweight
