# Menu Positioning (AddTopLevelMenuItemEx)

## Overview

This example demonstrates how to insert a custom top-level menu at a specific position in the editor's menu bar using `AddTopLevelMenuItemEx`. Unlike `AddTopLevelMenuItem` which appends after all built-in menus, this hook lets you place your menu between existing ones (e.g., between File and Edit).

## Position Index Reference

| Position | Inserted After |
|----------|---------------|
| `-1` | After all menus (same as `AddTopLevelMenuItem`) |
| `0` | After **File** |
| `1` | After **Edit** |
| `2` | After **View** |
| `3` | After **World** |
| `4` | After **Developer** |
| `5` | After **Addons** |
| `6` | After **Extra** |

## Files

### package.json

```json
{
    "name": "Menu Positioning Addon",
    "author": "Octave Examples",
    "description": "Inserts a custom menu between File and Edit.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "menupositioningaddon",
        "apiVersion": 2
    }
}
```

### Source/MenuPositioningAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static bool sValidateExport(void* userData)
{
    // Return false to grey out the item (e.g., when no scene is loaded)
    return true;
}

static void DrawBuildMenu(void* userData)
{
    if (ImGui::MenuItem("Build Current Scene"))
    {
        sEngineAPI->LogDebug("Building current scene...");
    }

    if (ImGui::MenuItem("Build All Scenes"))
    {
        sEngineAPI->LogDebug("Building all scenes...");
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Export"))
    {
        if (ImGui::MenuItem("Export as FBX"))
        {
            sEngineAPI->LogDebug("Exporting as FBX...");
        }
        if (ImGui::MenuItem("Export as GLTF"))
        {
            sEngineAPI->LogDebug("Exporting as GLTF...");
        }
        ImGui::EndMenu();
    }
}

static void OnExportAction(void* userData)
{
    sEngineAPI->LogDebug("Export action triggered from File menu!");
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
    // Insert "Build" menu after File (position 0)
    hooks->AddTopLevelMenuItemEx(hookId, "Build", DrawBuildMenu, nullptr, 0);

    // Add a menu item with validation to the File menu
    hooks->AddMenuItemEx(
        hookId,
        "File",               // Top-level menu to add to
        "Export Scene...",     // Item display name
        OnExportAction,        // Click callback
        nullptr,               // User data
        "Ctrl+E",             // Shortcut display text
        sValidateExport        // Validation callback (nullptr = always enabled)
    );
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Menu Positioning Addon";
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

The menu bar will look like:

```
File | Build | Edit | View | World | Developer | Addons | Extra
```

The "Build" menu is inserted at position 0, placing it right after File.

## API Reference

### AddTopLevelMenuItemEx

```cpp
void (*AddTopLevelMenuItemEx)(HookId hookId, const char* menuName,
    TopLevelMenuDrawCallback drawFunc, void* userData, int32_t position);
```

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `menuName` - Display name for the menu
- `drawFunc` - Callback invoked each frame when the menu is open
- `userData` - Optional user data passed to the callback
- `position` - Position index (-1 to append, 0-6 to insert after a built-in menu)

### AddMenuItemEx

```cpp
void (*AddMenuItemEx)(HookId hookId, const char* menuPath, const char* itemPath,
    MenuCallback callback, void* userData, const char* shortcut,
    MenuValidationCallback validateFunc);
```

**Parameters:**
- `hookId` - The hook identifier
- `menuPath` - Top-level menu name (e.g., "File", "Edit")
- `itemPath` - Item display text
- `callback` - Function called when clicked
- `userData` - Optional user data
- `shortcut` - Shortcut display text (e.g., "Ctrl+E"), or nullptr
- `validateFunc` - Returns false to grey out the item, or nullptr for always enabled

## Best Practices

1. **Position Carefully** - Don't insert too many menus; consider using submenus instead
2. **Validation Callbacks** - Use `MenuValidationCallback` to disable items when they shouldn't be available
3. **Shortcut Text** - The shortcut parameter is display-only; use `RegisterShortcut` to bind actual keys
