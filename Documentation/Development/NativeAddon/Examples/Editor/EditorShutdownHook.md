# Editor Shutdown Hook

## Overview

This example demonstrates how to perform cleanup before the editor closes using `RegisterOnEditorShutdown`. This hook enables addons to save state, close resources, and perform final operations while the editor is still running, before plugin unload occurs.

## Files

### package.json

```json
{
    "name": "Shutdown Handler Addon",
    "author": "Octave Examples",
    "description": "Handles cleanup before editor shutdown.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "shutdownhandleraddon",
        "apiVersion": 2
    }
}
```

### Source/ShutdownHandlerAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static bool sHasUnsavedData = false;

/**
 * @brief Called when the editor is about to shut down.
 * @param userData User-provided data (unused in this example).
 *
 * This is the last chance to save addon state, close resources,
 * and perform cleanup while the editor is still running.
 * Called before OnUnload.
 */
static void OnEditorShutdown(void* userData)
{
    sEngineAPI->LogDebug("Editor shutting down...");

    // Example: Save addon configuration
    if (sHasUnsavedData)
    {
        sEngineAPI->LogDebug("Saving addon configuration...");
        sHasUnsavedData = false;
    }

    // Example: Close external connections
    sEngineAPI->LogDebug("Closing external connections...");

    // Example: Save session statistics
    sEngineAPI->LogDebug("Saving session analytics...");

    // Example: Clean up temporary files
    sEngineAPI->LogDebug("Cleaning up temporary files...");

    // Example: Flush logs
    sEngineAPI->LogDebug("Flushing addon logs...");

    sEngineAPI->LogDebug("Addon shutdown preparation complete");
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
    api->LogDebug("Shutdown Handler Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 *
 * Note: OnEditorShutdown is called BEFORE OnUnload.
 * Final cleanup that does not require editor systems should go here.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Shutdown Handler Addon unloading.");
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
    hooks->RegisterOnEditorShutdown(hookId, OnEditorShutdown, nullptr);
    sEngineAPI->LogDebug("Editor shutdown hook registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Shutdown Handler Addon";
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

### RegisterOnEditorShutdown

```cpp
void (*RegisterOnEditorShutdown)(HookId hookId, EventCallback cb, void* userData);
```

Registers a callback invoked when the editor is about to shut down.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call before editor shutdown
- `userData` - Optional user data passed to the callback

**Triggered when:**
- User closes the editor
- Editor restart is initiated

### EventCallback

```cpp
typedef void (*EventCallback)(void* userData);
```

Callback signature for parameterless events.

**Parameters:**
- `userData` - User-provided data from registration

## Shutdown Sequence

The complete shutdown sequence is:

1. `OnEditorShutdown()` callbacks fire - Save state, close resources
2. Editor systems begin shutdown
3. `RemoveAllHooks(hookId)` is called automatically for each plugin
4. `OnUnload()` is called - Final cleanup
5. Plugin DLL is unloaded
6. Editor process terminates

## Best Practices

1. **Save First** - Save critical data before doing other cleanup
2. **Fast Execution** - Keep shutdown callback fast to avoid delaying editor close
3. **Error Handling** - Handle errors gracefully; do not prevent editor shutdown
4. **No UI** - Do not show modal dialogs or blocking UI during shutdown
5. **Cleanup Order** - Clean up in reverse order of initialization
6. **Crash Safety** - Do not rely solely on this hook; auto-save periodically during the session
7. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
8. **Log Everything** - Log shutdown steps for debugging shutdown issues

## Important Notes

1. **Not Guaranteed** - In case of crashes, this callback may not be called
2. **Editor Still Running** - Called while the editor is still running, before `OnUnload`
3. **No Blocking** - Do not block or show dialogs that require user input
4. **Auto-Save** - Implement periodic auto-save rather than relying solely on shutdown
