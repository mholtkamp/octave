# Custom Play Targets

## Overview

This example demonstrates how to add custom play targets to the play dropdown menu. Play targets appear in the dropdown next to the Play button, alongside built-in targets like "Play In Editor" and "Play on Dolphin".

## Files

### package.json

```json
{
    "name": "Custom Play Target Addon",
    "author": "Octave Examples",
    "description": "Adds a custom play target to the play dropdown.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "customplaytargetaddon",
        "apiVersion": 2
    }
}
```

### Source/CustomPlayTargetAddon.cpp

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
 * @brief Called when the user selects this play target and presses Play.
 */
static void OnPlayRemoteDevice(void* userData)
{
    sEngineAPI->LogDebug("Deploying to remote device...");
    // In a real addon: build, deploy, and launch on remote device
}

/**
 * @brief Called when the user selects the web preview target.
 */
static void OnPlayWebPreview(void* userData)
{
    sEngineAPI->LogDebug("Starting web preview server...");
    // In a real addon: start a local server and open browser
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
    hooks->AddPlayTarget(
        hookId,
        "Remote Device",       // Target name shown in dropdown
        "[R]",                 // Icon text (shown next to name)
        OnPlayRemoteDevice,    // Callback when play is pressed
        nullptr                // User data
    );

    hooks->AddPlayTarget(
        hookId,
        "Web Preview",
        "[W]",
        OnPlayWebPreview,
        nullptr
    );
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Play Target Addon";
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

## Result

The play target dropdown will show:

```
Play In Editor
Play on Dolphin
Play on Azahar
Play Standalone
Send 3dsLink
──────────────
[R] Remote Device     <-- addon target
[W] Web Preview       <-- addon target
```

## API Reference

### AddPlayTarget

```cpp
void (*AddPlayTarget)(HookId hookId, const char* targetName,
    const char* iconText, PlayTargetCallback callback, void* userData);
```

**Parameters:**
- `hookId` - The hook identifier
- `targetName` - Display name in the dropdown
- `iconText` - Short icon/prefix text
- `callback` - Function called when play is pressed with this target selected
- `userData` - Optional user data

### RemovePlayTarget

```cpp
void (*RemovePlayTarget)(HookId hookId, const char* targetName);
```

## Best Practices

1. **Descriptive Names** - Use clear target names that describe the platform or mode
2. **Icon Text** - Keep icon text short (1-3 chars); it appears next to the target name
3. **Build First** - In your callback, consider building the project before deploying
