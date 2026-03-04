# Editor Initialization Hooks

## Overview

This example demonstrates the two editor initialization callbacks available on the `OctavePluginDesc` struct: `OnEditorPreInit` and `OnEditorReady`. These are set directly on the descriptor (not via `EditorUIHooks`) and provide control over when addon code executes during editor startup, allowing for custom font loading, early configuration, and post-initialization setup.

## Files

### package.json

```json
{
    "name": "Init Timing Addon",
    "author": "Octave Examples",
    "description": "Demonstrates editor initialization timing hooks.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "inittimingaddon",
        "apiVersion": 2
    }
}
```

### Source/InitTimingAddon.cpp

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
 * @brief Called before the editor initializes its systems.
 *
 * Use this for:
 * - Loading custom fonts before ImGui initializes
 * - Setting early configuration values
 * - Preparing resources needed during editor init
 *
 * @warning Most editor systems are not yet available at this point.
 * Do not attempt to register UI hooks or access editor state.
 */
static void OnEditorPreInit()
{
    sEngineAPI->LogDebug("OnEditorPreInit called");

    // Example: Load custom font configuration
    sEngineAPI->LogDebug("Preparing custom fonts...");

    // Example: Set up early configuration
    sEngineAPI->LogDebug("Applying early editor configuration...");

    // Do NOT register UI hooks here - editor is not ready
}

/**
 * @brief Called after the editor has fully initialized.
 *
 * Use this for:
 * - Accessing editor state and systems
 * - Performing one-time setup that requires the editor to be ready
 *
 * Note: For registering editor hooks (menus, toolbars, event callbacks),
 * use the RegisterEditorUI callback instead.
 */
static void OnEditorReady()
{
    sEngineAPI->LogDebug("OnEditorReady called");

    // Example: Query initial editor state
    sEngineAPI->LogDebug("Querying editor state...");

    // Example: Validate addon dependencies
    sEngineAPI->LogDebug("Verifying addon dependencies...");

    sEngineAPI->LogDebug("Addon fully initialized and ready");
}

/**
 * @brief Register editor UI hooks.
 * @param hooks Pointer to the EditorUIHooks function table.
 * @param hookId Unique identifier for this plugin's hooks.
 *
 * This is the correct place to register event hooks and UI extensions.
 * All hooks registered here are automatically cleaned up via
 * RemoveAllHooks(hookId) when the plugin unloads.
 */
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    // Register any event hooks here
    // hooks->RegisterOnSelectionChanged(hookId, MyCallback, nullptr);

    sEngineAPI->LogDebug("Editor UI hooks registered");
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
    api->LogDebug("Init Timing Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Init Timing Addon unloading.");
    sEngineAPI = nullptr;
}

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Init Timing Addon";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;
    desc->TickEditor = nullptr;
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = nullptr;
#if EDITOR
    desc->RegisterEditorUI = RegisterEditorUI;
    desc->OnEditorPreInit = OnEditorPreInit;
    desc->OnEditorReady = OnEditorReady;
#else
    desc->RegisterEditorUI = nullptr;
    desc->OnEditorPreInit = nullptr;
    desc->OnEditorReady = nullptr;
#endif
    return 0;
}
```

## API Reference

### OctavePluginDesc Initialization Fields

```cpp
// Set directly on the OctavePluginDesc struct (NOT via EditorUIHooks)
desc->OnEditorPreInit = OnEditorPreInit;  // Called before editor init
desc->OnEditorReady = OnEditorReady;      // Called after editor init
```

### OnEditorPreInit

```cpp
void (*OnEditorPreInit)();
```

Optional callback invoked before the editor initializes its core systems.

**When to use:**
- Loading custom fonts before ImGui font atlas is built
- Setting configuration values that affect editor initialization
- Preparing resources needed during editor startup

**Restrictions:**
- Editor systems are not yet available
- Do not register UI hooks or callbacks
- Do not access editor state
- Keep execution time minimal

### OnEditorReady

```cpp
void (*OnEditorReady)();
```

Optional callback invoked after the editor has fully initialized all systems.

**When to use:**
- Querying initial editor state
- Performing one-time setup that requires a fully initialized editor
- Validating addon dependencies

**Note:** For registering event hooks and UI extensions, use `RegisterEditorUI` instead.

## Initialization Order

The complete initialization sequence is:

1. `OctavePlugin_GetDesc()` - Descriptor is filled out
2. `OnLoad()` - Plugin loaded, store engine API
3. `OnEditorPreInit()` - (Optional) Pre-editor setup
4. Editor initializes core systems
5. `RegisterEditorUI()` - Register hooks, menus, toolbars
6. `OnEditorReady()` - (Optional) Post-editor setup
7. Editor main loop begins

## Best Practices

1. **Choose the Right Hook** - Use `OnEditorPreInit` only when necessary; most setup code belongs in `RegisterEditorUI` or `OnEditorReady`
2. **Font Loading** - Custom fonts must be loaded in `OnEditorPreInit` before ImGui builds its atlas
3. **Hook Registration** - Register event hooks in `RegisterEditorUI`, not in `OnEditorPreInit` or `OnEditorReady`
4. **Keep PreInit Fast** - Minimize work in `OnEditorPreInit` to avoid delaying editor startup
5. **Null Checks** - Both hooks are optional; set to `nullptr` if not needed
6. **Error Handling** - Log errors clearly to help diagnose initialization failures
7. **Desc Fields** - These are set on `OctavePluginDesc`, not registered through `EditorUIHooks`
