# Scene Lifecycle Hooks

## Overview

This example demonstrates how to react to scene lifecycle events using `RegisterOnSceneOpen` and `RegisterOnSceneClose`. These hooks enable addons to perform scene-specific setup, cache management, and cleanup when switching between scenes during development.

## Files

### package.json

```json
{
    "name": "Scene Watcher Addon",
    "author": "Octave Examples",
    "description": "Monitors scene lifecycle events.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "scenewatcheraddon",
        "apiVersion": 2
    }
}
```

### Source/SceneWatcherAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static char sCurrentScenePath[512] = {};
static int sSceneOpenCount = 0;

/**
 * @brief Called when a scene is opened.
 * @param scenePath Absolute path to the scene file.
 * @param userData User-provided data.
 */
static void OnSceneOpen(const char* scenePath, void* userData)
{
    sSceneOpenCount++;
    sEngineAPI->LogDebug("Scene opened (#%d): %s", sSceneOpenCount, scenePath);

    if (scenePath)
    {
        strncpy(sCurrentScenePath, scenePath, sizeof(sCurrentScenePath) - 1);
        sCurrentScenePath[sizeof(sCurrentScenePath) - 1] = '\0';
    }

    // Example: Build scene-specific caches
    sEngineAPI->LogDebug("Building scene cache...");

    // Example: Load scene-specific addon data
    sEngineAPI->LogDebug("Loading scene configuration...");

    // Example: Validate scene content
    sEngineAPI->LogDebug("Validating scene assets...");
}

/**
 * @brief Called when a scene is closed.
 * @param scenePath Absolute path to the scene file being closed.
 * @param userData User-provided data.
 */
static void OnSceneClose(const char* scenePath, void* userData)
{
    sEngineAPI->LogDebug("Scene closing: %s", scenePath);

    // Example: Save scene-specific addon data
    sEngineAPI->LogDebug("Saving scene-specific addon data...");

    // Example: Clear scene-specific caches
    sEngineAPI->LogDebug("Clearing scene cache...");

    sCurrentScenePath[0] = '\0';
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
    api->LogDebug("Scene Watcher Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Scene Watcher Addon unloading.");
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
    hooks->RegisterOnSceneOpen(hookId, OnSceneOpen, nullptr);
    hooks->RegisterOnSceneClose(hookId, OnSceneClose, nullptr);
    sEngineAPI->LogDebug("Scene lifecycle hooks registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Scene Watcher Addon";
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

### RegisterOnSceneOpen

```cpp
void (*RegisterOnSceneOpen)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a scene is opened in the editor.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when a scene opens
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User opens a scene from the File menu
- Scene is opened programmatically
- Project initially loads with a default scene

### RegisterOnSceneClose

```cpp
void (*RegisterOnSceneClose)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a scene is closed in the editor.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when a scene closes
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User closes the current scene
- User opens a different scene (old scene closes first)
- Project is closed

### StringEventCallback

```cpp
typedef void (*StringEventCallback)(const char* str, void* userData);
```

Callback signature for events that provide a string parameter.

**Parameters:**
- `str` - String data (scene file path in these cases)
- `userData` - User-provided data from registration

## Best Practices

1. **Path Storage** - Store the scene path if you need to reference it elsewhere
2. **Cache Management** - Build scene-specific caches on open, clear on close
3. **Cleanup** - Always clean up in `OnSceneClose` before switching scenes
4. **Null Checks** - Validate the `scenePath` parameter before using it
5. **Performance** - Keep callbacks fast to avoid delaying scene loading
6. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
7. **Multi-Scene** - Handle rapid scene switching gracefully
