# Game Preview Custom Resolution

## Overview

This example demonstrates how to add custom resolution presets to the Game Preview panel. Resolution presets appear in the Resolution dropdown alongside built-in presets like "GameCube 640x480", "720p 1280x720", etc.

## Files

### package.json

```json
{
    "name": "Game Preview Resolutions",
    "author": "Octave Examples",
    "description": "Adds custom resolution presets to the Game Preview panel.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "gamepreviewresolutions",
        "apiVersion": 2
    }
}
```

### Source/GamePreviewResolutions.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

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
    // Add custom resolution presets to Game Preview
    hooks->AddGamePreviewResolution(hookId, "Ultra-wide 2560x1080", 2560, 1080);
    hooks->AddGamePreviewResolution(hookId, "Steam Deck 1280x800", 1280, 800);
    hooks->AddGamePreviewResolution(hookId, "Switch 1280x720", 1280, 720);
    hooks->AddGamePreviewResolution(hookId, "GBA 240x160", 240, 160);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Game Preview Resolutions";
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

The Game Preview resolution dropdown will show:

```
GameCube  640x480
Wii  854x480
3DS Top  400x240
3DS Bottom  320x240
720p  1280x720
1080p  1920x1080
4K  3840x2160
Ultra-wide 2560x1080    <-- addon preset
Steam Deck 1280x800     <-- addon preset
Switch 1280x720         <-- addon preset
GBA 240x160             <-- addon preset
```

## API Reference

### AddGamePreviewResolution

```cpp
void (*AddGamePreviewResolution)(HookId hookId, const char* name,
    uint32_t width, uint32_t height);
```

**Parameters:**
- `hookId` - The hook identifier
- `name` - Display name for the resolution preset
- `width` - Resolution width in pixels
- `height` - Resolution height in pixels

### RemoveGamePreviewResolution

```cpp
void (*RemoveGamePreviewResolution)(HookId hookId, const char* name);
```

**Parameters:**
- `hookId` - The hook identifier used during registration
- `name` - Name of the preset to remove

## Best Practices

1. **Descriptive Names** - Include both a label and the resolution in the name (e.g., "Steam Deck 1280x800")
2. **Reasonable Sizes** - Very large resolutions (e.g., 8K) will use significant VRAM for the render targets
3. **Cleanup** - Presets are automatically removed when `RemoveAllHooks` is called during addon unload
