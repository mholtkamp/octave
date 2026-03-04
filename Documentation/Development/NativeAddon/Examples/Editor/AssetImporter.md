# Custom Asset Importers & Drag-Drop Handlers

## Overview

This example demonstrates:
- **Custom asset importers**: Handle file imports for custom extensions
- **Drag-drop handlers**: React to drag-and-drop events in specific editor areas
- **Pre/post import hooks**: Intercept the import pipeline

## Files

### package.json

```json
{
    "name": "Asset Pipeline Addon",
    "author": "Octave Examples",
    "description": "Custom asset importers and drag-drop handlers.",
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
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
/**
 * @brief Custom importer for .csv files.
 * @param filePath Full path to the file being imported.
 * @param extension File extension (e.g., ".csv").
 * @param userData User data (unused).
 * @return true if import was handled, false to fall back to default.
 */
static bool ImportCSV(const char* filePath, const char* extension, void* userData)
{
    sEngineAPI->LogDebug("Importing CSV file: %s", filePath);
    // Parse CSV, create data asset, etc.
    return true; // Handled
}

/**
 * @brief Custom importer for .json data files.
 */
static bool ImportJSON(const char* filePath, const char* extension, void* userData)
{
    sEngineAPI->LogDebug("Importing JSON file: %s", filePath);
    return true;
}

/**
 * @brief Custom drag-drop handler for the viewport.
 * @param payloadType ImGui payload type string.
 * @param payloadData Pointer to the payload data.
 * @param payloadSize Size of the payload in bytes.
 * @param userData User data (unused).
 * @return true if the drop was consumed, false to pass to next handler.
 */
static bool HandleViewportDrop(const char* payloadType, const void* payloadData,
                                int32_t payloadSize, void* userData)
{
    sEngineAPI->LogDebug("Viewport drop: type=%s, size=%d", payloadType, payloadSize);
    // Handle the drop (e.g., spawn a node from the dropped asset)
    return false; // Let other handlers process it too
}

/**
 * @brief Pre-import hook - called before any asset is imported.
 * @param filePath Path of the file about to be imported.
 * @param userData User data (unused).
 * @return true to allow import, false to cancel it.
 */
static bool OnPreImport(const char* filePath, void* userData)
{
    sEngineAPI->LogDebug("Pre-import: %s", filePath);
    // Return false to cancel the import
    return true;
}

/**
 * @brief Post-import hook - called after an asset is imported.
 * @param assetPath Path of the imported asset.
 * @param userData User data (unused).
 */
static void OnPostImport(const char* assetPath, void* userData)
{
    sEngineAPI->LogDebug("Post-import: %s", assetPath);
}
#endif

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    return 0;
}

static void OnUnload()
{
    sEngineAPI = nullptr;
}

#if EDITOR
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    // Register custom importers for file extensions
    hooks->RegisterAssetImporter(hookId, ".csv", ImportCSV, nullptr);
    hooks->RegisterAssetImporter(hookId, ".json", ImportJSON, nullptr);

    // Register drag-drop handler for viewport
    hooks->RegisterDragDropHandler(hookId, "Viewport", HandleViewportDrop, nullptr);

    // Pre/post import hooks
    hooks->RegisterOnPreAssetImport(hookId, OnPreImport, nullptr);
    hooks->RegisterOnPostAssetImport(hookId, OnPostImport, nullptr);
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

### RegisterAssetImporter

```cpp
void (*RegisterAssetImporter)(HookId hookId, const char* extension,
    AssetImportCallback importFunc, void* userData);
```

Register a handler for a file extension. Return `true` from callback if handled.

### RegisterDragDropHandler

```cpp
void (*RegisterDragDropHandler)(HookId hookId, const char* targetArea,
    DragDropHandlerCallback handler, void* userData);
```

**targetArea values:** `"Viewport"`, `"Hierarchy"`, `"AssetBrowser"`, `"Inspector"`

### RegisterOnPreAssetImport

```cpp
void (*RegisterOnPreAssetImport)(HookId hookId, PreImportCallback cb, void* userData);
```

Called before import. Return `false` to cancel the import.

### RegisterOnPostAssetImport

```cpp
void (*RegisterOnPostAssetImport)(HookId hookId, StringEventCallback cb, void* userData);
```

Called after import with the imported asset path.

## Best Practices

1. **Return Values Matter** - Importers and drag-drop handlers use return values to indicate handling
2. **Extension Format** - Include the dot (e.g., `".csv"` not `"csv"`)
3. **Pre-Import Validation** - Use `RegisterOnPreAssetImport` to validate or transform files before import
4. **Non-Destructive** - Don't modify the original source file during import
