# Project Lifecycle Hooks

## Overview

This example demonstrates how to react to project lifecycle events using `RegisterOnProjectOpen`, `RegisterOnProjectClose`, and `RegisterOnProjectSave`. These hooks enable addons to maintain project-specific state, validate project data, and perform cleanup when switching projects.

## Files

### package.json

```json
{
    "name": "Project Watcher Addon",
    "author": "Octave Examples",
    "description": "Monitors project lifecycle events.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "projectwatcheraddon",
        "apiVersion": 2
    }
}
```

### Source/ProjectWatcherAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static char sCurrentProjectPath[512] = {};

/**
 * @brief Called when a project is opened.
 * @param projectPath Absolute path to the project file.
 * @param userData User-provided data.
 */
static void OnProjectOpen(const char* projectPath, void* userData)
{
    sEngineAPI->LogDebug("Project opened: %s", projectPath);

    if (projectPath)
    {
        strncpy(sCurrentProjectPath, projectPath, sizeof(sCurrentProjectPath) - 1);
        sCurrentProjectPath[sizeof(sCurrentProjectPath) - 1] = '\0';
    }

    // Example: Load project-specific addon settings
    sEngineAPI->LogDebug("Loading project-specific addon settings...");

    // Example: Initialize project-level caches
    sEngineAPI->LogDebug("Initializing project caches...");
}

/**
 * @brief Called when the current project is closed.
 * @param projectPath Absolute path to the project file being closed.
 * @param userData User-provided data.
 */
static void OnProjectClose(const char* projectPath, void* userData)
{
    sEngineAPI->LogDebug("Project closing: %s", projectPath);

    // Example: Save addon data before closing
    sEngineAPI->LogDebug("Saving addon data for project...");

    // Example: Clear project-specific caches
    sEngineAPI->LogDebug("Clearing project caches...");

    sCurrentProjectPath[0] = '\0';
}

/**
 * @brief Called when the project is saved.
 * @param projectPath Absolute path to the project file being saved.
 * @param userData User-provided data.
 */
static void OnProjectSave(const char* projectPath, void* userData)
{
    sEngineAPI->LogDebug("Project saved: %s", projectPath);

    // Example: Persist addon configuration alongside the project
    sEngineAPI->LogDebug("Persisting addon settings...");
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
    api->LogDebug("Project Watcher Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Project Watcher Addon unloading.");
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
    hooks->RegisterOnProjectOpen(hookId, OnProjectOpen, nullptr);
    hooks->RegisterOnProjectClose(hookId, OnProjectClose, nullptr);
    hooks->RegisterOnProjectSave(hookId, OnProjectSave, nullptr);
    sEngineAPI->LogDebug("Project lifecycle hooks registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Project Watcher Addon";
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

### RegisterOnProjectOpen

```cpp
void (*RegisterOnProjectOpen)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a project is opened.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when a project opens
- `userData` - Optional user data passed to the callback

### RegisterOnProjectClose

```cpp
void (*RegisterOnProjectClose)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a project is closed.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when a project closes
- `userData` - Optional user data passed to the callback

### RegisterOnProjectSave

```cpp
void (*RegisterOnProjectSave)(HookId hookId, StringEventCallback cb, void* userData);
```

Registers a callback invoked when a project is saved.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when a project is saved
- `userData` - Optional user data passed to the callback

### StringEventCallback

```cpp
typedef void (*StringEventCallback)(const char* str, void* userData);
```

Callback signature for events that provide a string parameter.

**Parameters:**
- `str` - String data (project file path in these cases)
- `userData` - User-provided data from registration

## Best Practices

1. **State Management** - Use `OnProjectOpen` to initialize and `OnProjectClose` to clean up project-specific state
2. **Path Storage** - Store the project path if you need to reference it elsewhere in your addon
3. **Save Validation** - Use `OnProjectSave` to validate or persist addon data alongside the project
4. **Cleanup Order** - Always clean up in `OnProjectClose` before a new project opens
5. **Null Checks** - Always validate the path parameter before using it
6. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
7. **No Blocking** - Keep callbacks fast to avoid delaying project operations
