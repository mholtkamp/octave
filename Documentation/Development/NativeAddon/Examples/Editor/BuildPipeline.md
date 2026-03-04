# Pre/Post Build Hooks

## Overview

This example demonstrates how to hook into the build/packaging pipeline using `RegisterOnPreBuild` and `RegisterOnPostBuild`. Pre-build hooks can cancel the build (e.g., for validation), and post-build hooks can run follow-up actions (e.g., copy files, notify, upload).

## Build Hook Call Order

When a build is triggered, hooks fire in this order:

1. **`OnPreBuild`** - Can cancel the build by returning `false`
2. **`OnPackageStarted`** - Notification that packaging has begun
3. *(build/packaging happens)*
4. **`OnPackageFinished`** - Notification with success/failure status
5. **`OnPostBuild`** - Final hook with success/failure status

## Files

### package.json

```json
{
    "name": "Build Pipeline Addon",
    "author": "Octave Examples",
    "description": "Pre/post build hooks for validation and automation.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "buildpipelineaddon",
        "apiVersion": 2
    }
}
```

### Source/BuildPipelineAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR

static bool sRequireCleanBuild = false;

/**
 * @brief Pre-build hook - validate before packaging starts.
 * @param platform Platform enum (e.g., Windows, GameCube, N3DS).
 * @param userData User data (unused).
 * @return true to allow the build, false to cancel it.
 *
 * Use this to run validation checks, ensure required files exist,
 * or prompt the user before a long build process.
 */
static bool OnPreBuild(int32_t platform, void* userData)
{
    sEngineAPI->LogDebug("Pre-build check for platform %d...", platform);

    // Example: cancel build if a required file is missing
    // if (!RequiredConfigExists())
    // {
    //     sEngineAPI->LogError("Build cancelled: missing required config file");
    //     return false;
    // }

    // Example: clean build directory if flag is set
    if (sRequireCleanBuild)
    {
        sEngineAPI->LogDebug("Cleaning previous build output...");
        // CleanBuildDirectory(platform);
        sRequireCleanBuild = false;
    }

    sEngineAPI->LogDebug("Pre-build checks passed.");
    return true; // Allow build to proceed
}

/**
 * @brief Post-build hook - runs after packaging completes.
 * @param platform Platform enum.
 * @param success true if the build succeeded.
 * @param userData User data (unused).
 *
 * Use this for post-processing: copying additional files,
 * generating manifests, uploading builds, sending notifications, etc.
 */
static void OnPostBuild(int32_t platform, bool success, void* userData)
{
    if (success)
    {
        sEngineAPI->LogDebug("Post-build: Build succeeded for platform %d", platform);

        // Example: copy additional data files to the output directory
        // CopyAddonDataToPackage(platform);

        // Example: generate a build manifest
        // GenerateBuildManifest(platform);

        // Example: notify a CI/CD system
        // NotifyBuildServer(platform, true);
    }
    else
    {
        sEngineAPI->LogError("Post-build: Build FAILED for platform %d", platform);

        // Example: log diagnostics
        // CollectBuildDiagnostics(platform);
    }
}

/**
 * @brief Package started notification (fires after PreBuild passes).
 */
static void OnPackageStarted(int32_t platform, void* userData)
{
    sEngineAPI->LogDebug("Package started for platform %d", platform);
}

/**
 * @brief Package finished notification (fires before PostBuild).
 */
static void OnPackageFinished(int32_t platform, bool success, void* userData)
{
    sEngineAPI->LogDebug("Package finished for platform %d (success=%d)", platform, success);
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
    // Pre/post build hooks (Batch 10)
    hooks->RegisterOnPreBuild(hookId, OnPreBuild, nullptr);
    hooks->RegisterOnPostBuild(hookId, OnPostBuild, nullptr);

    // Existing packaging event hooks (for comparison)
    hooks->RegisterOnPackageStarted(hookId, OnPackageStarted, nullptr);
    hooks->RegisterOnPackageFinished(hookId, OnPackageFinished, nullptr);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Build Pipeline Addon";
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

### RegisterOnPreBuild

```cpp
void (*RegisterOnPreBuild)(HookId hookId, PreBuildCallback cb, void* userData);
```

Called before packaging begins. Return `false` from the callback to cancel the build.

```cpp
typedef bool (*PreBuildCallback)(int32_t platform, void* userData);
```

**Parameters:**
- `platform` - Platform enum value (matches the `Platform` enum)
- `userData` - User data from registration

**Return:** `true` to allow the build, `false` to cancel

### RegisterOnPostBuild

```cpp
void (*RegisterOnPostBuild)(HookId hookId, PackageFinishedCallback cb, void* userData);
```

Called after packaging completes (both success and failure).

```cpp
typedef void (*PackageFinishedCallback)(int32_t platform, bool success, void* userData);
```

**Parameters:**
- `platform` - Platform enum value
- `success` - `true` if the build succeeded
- `userData` - User data from registration

### Existing Hooks (for reference)

```cpp
void (*RegisterOnPackageStarted)(HookId hookId, PlatformEventCallback cb, void* userData);
void (*RegisterOnPackageFinished)(HookId hookId, PackageFinishedCallback cb, void* userData);
```

These fire between PreBuild and PostBuild and cannot cancel the build.

## Use Cases

### Build Validation

```cpp
static bool OnPreBuild(int32_t platform, void* userData)
{
    if (!AllScenesValid())
    {
        sEngineAPI->LogError("Build cancelled: invalid scenes detected");
        return false; // Cancel
    }
    return true;
}
```

### Post-Build File Copy

```cpp
static void OnPostBuild(int32_t platform, bool success, void* userData)
{
    if (success)
    {
        // Copy addon-specific data files to the packaged output
        CopyFile("addon_data.dat", GetPackageOutputDir(platform));
    }
}
```

### Build Notifications

```cpp
static void OnPostBuild(int32_t platform, bool success, void* userData)
{
    if (success)
        sEngineAPI->LogDebug("BUILD PASSED - ready for deployment");
    else
        sEngineAPI->LogError("BUILD FAILED - check log for errors");
}
```

## Best Practices

1. **PreBuild is for validation** - Keep it fast; don't do heavy work before the build starts
2. **Return true by default** - Only return `false` from PreBuild when there's a genuine reason to cancel
3. **Log cancellation reasons** - Always log why a build was cancelled so the user understands
4. **PostBuild for cleanup** - Use PostBuild for file copying, manifest generation, or notifications
5. **Check success flag** - Always check the `success` parameter in PostBuild before running post-processing
