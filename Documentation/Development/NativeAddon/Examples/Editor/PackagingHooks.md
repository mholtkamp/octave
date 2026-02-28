# Packaging Hooks

## Overview

This example demonstrates how to hook into the packaging/build process using `RegisterOnPackageStarted` and `RegisterOnPackageFinished`. These hooks allow addons to perform pre-build validation and post-build actions such as asset optimization, custom data generation, or deployment automation.

## Files

### package.json

```json
{
    "name": "Package Validator Addon",
    "author": "Octave Examples",
    "description": "Validates and processes builds during packaging.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "packagevalidatoraddon",
        "apiVersion": 2
    }
}
```

### Source/PackageValidatorAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
/**
 * @brief Called when packaging starts for a platform.
 * @param platform Platform being packaged (0=Windows, 1=Linux, 2=GameCube, etc.).
 * @param userData User-provided data.
 */
static void OnPackageStarted(int32_t platform, void* userData)
{
    const char* platformNames[] = { "Windows", "Linux", "GameCube", "Wii", "3DS" };
    const char* name = (platform >= 0 && platform < 5) ? platformNames[platform] : "Unknown";

    sEngineAPI->LogDebug("Package started for platform: %s", name);

    // Example: Run pre-build validation
    sEngineAPI->LogDebug("Running pre-build validation...");

    // Example: Clean old build artifacts
    sEngineAPI->LogDebug("Cleaning build directory for %s...", name);
}

/**
 * @brief Called when packaging completes or fails.
 * @param platform Platform that was packaged.
 * @param success Whether the build succeeded.
 * @param userData User-provided data.
 */
static void OnPackageFinished(int32_t platform, bool success, void* userData)
{
    const char* platformNames[] = { "Windows", "Linux", "GameCube", "Wii", "3DS" };
    const char* name = (platform >= 0 && platform < 5) ? platformNames[platform] : "Unknown";

    sEngineAPI->LogDebug("Package finished for %s: %s", name, success ? "SUCCESS" : "FAILED");

    if (success)
    {
        // Example: Post-build processing
        sEngineAPI->LogDebug("Generating deployment manifest...");
        sEngineAPI->LogDebug("Build ready for distribution");
    }
    else
    {
        sEngineAPI->LogDebug("Build failed, cleaning up partial artifacts...");
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
    api->LogDebug("Package Validator Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Package Validator Addon unloading.");
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
    hooks->RegisterOnPackageStarted(hookId, OnPackageStarted, nullptr);
    hooks->RegisterOnPackageFinished(hookId, OnPackageFinished, nullptr);
    sEngineAPI->LogDebug("Packaging hooks registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Package Validator Addon";
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

### RegisterOnPackageStarted

```cpp
void (*RegisterOnPackageStarted)(HookId hookId, PlatformEventCallback cb, void* userData);
```

Registers a callback invoked when packaging begins for a platform.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when packaging starts
- `userData` - Optional user data passed to the callback

### RegisterOnPackageFinished

```cpp
void (*RegisterOnPackageFinished)(HookId hookId, PackageFinishedCallback cb, void* userData);
```

Registers a callback invoked when packaging completes or fails.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `cb` - Function to call when packaging finishes
- `userData` - Optional user data passed to the callback

### PlatformEventCallback

```cpp
typedef void (*PlatformEventCallback)(int32_t platform, void* userData);
```

**Parameters:**
- `platform` - Platform ID (0=Windows, 1=Linux, 2=GameCube, 3=Wii, 4=3DS)
- `userData` - User-provided data from registration

### PackageFinishedCallback

```cpp
typedef void (*PackageFinishedCallback)(int32_t platform, bool success, void* userData);
```

**Parameters:**
- `platform` - Platform ID that was packaged
- `success` - `true` if packaging succeeded, `false` if it failed
- `userData` - User-provided data from registration

## Best Practices

1. **Pre-build Validation** - Use `OnPackageStarted` for validation checks before the build runs
2. **Post-build Actions** - Use `OnPackageFinished` with the success flag for deployment/optimization
3. **Platform-Specific Logic** - Check the platform parameter for platform-specific operations
4. **Error Handling** - Always check the success flag before running post-build actions
5. **Automatic Cleanup** - Hooks are cleaned up automatically; manual unregistration is not required
6. **Performance** - Keep pre-build checks fast to avoid delaying build start
7. **Logging** - Log all validation and processing steps for debugging
