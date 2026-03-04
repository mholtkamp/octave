# Top Level Menu Extension

## Overview

This example demonstrates how to add a custom top-level menu to the editor's viewport bar using the `AddTopLevelMenuItem` hook. Top-level menus appear alongside "File", "Edit", "View", etc., and can contain submenus and custom actions. Cleanup is automatic when the plugin unloads.

## Files

### package.json

```json
{
    "name": "Custom Menu Addon",
    "author": "Octave Examples",
    "description": "Adds a custom top-level menu to the editor.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "custommenuaddon",
        "apiVersion": 2
    }
}
```

### Source/CustomMenuAddon.cpp

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
 * @brief Draw callback for the custom top-level menu.
 * @param userData User-provided data (unused in this example).
 *
 * Called each frame when the menu is open. Use ImGui menu functions
 * such as ImGui::MenuItem() and ImGui::BeginMenu() to draw items.
 */
static void DrawCustomMenu(void* userData)
{
    if (ImGui::MenuItem("Say Hello"))
    {
        sEngineAPI->LogDebug("Hello from custom menu!");
    }

    if (ImGui::MenuItem("Open Documentation"))
    {
        sEngineAPI->LogDebug("Opening documentation...");
    }

    ImGui::Separator();

    if (ImGui::BeginMenu("Submenu"))
    {
        if (ImGui::MenuItem("Option 1"))
        {
            sEngineAPI->LogDebug("Option 1 selected");
        }
        if (ImGui::MenuItem("Option 2"))
        {
            sEngineAPI->LogDebug("Option 2 selected");
        }
        if (ImGui::MenuItem("Option 3", nullptr, false, false))
        {
            // Disabled item - will not be called
        }
        ImGui::EndMenu();
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
    api->LogDebug("Custom Menu Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Custom Menu Addon unloading.");
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
    hooks->AddTopLevelMenuItem(hookId, "Custom", DrawCustomMenu, nullptr);
    sEngineAPI->LogDebug("Custom top-level menu registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Menu Addon";
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

### AddTopLevelMenuItem

```cpp
void (*AddTopLevelMenuItem)(HookId hookId, const char* menuName, TopLevelMenuDrawCallback drawFunc, void* userData);
```

Adds a custom top-level menu to the editor's viewport bar.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `menuName` - Display name for the menu (e.g., "Custom", "Tools")
- `drawFunc` - Callback invoked each frame when the menu is open
- `userData` - Optional user data passed to the callback

### RemoveTopLevelMenuItem

```cpp
void (*RemoveTopLevelMenuItem)(HookId hookId, const char* menuName);
```

Removes a previously registered top-level menu. Called automatically on plugin unload.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `menuName` - Name of the menu to remove

### TopLevelMenuDrawCallback

```cpp
typedef void (*TopLevelMenuDrawCallback)(void* userData);
```

Callback signature for menu drawing functions.

**Parameters:**
- `userData` - User-provided data from registration

## Best Practices

1. **Menu Naming** - Use concise, descriptive names that fit the editor's existing menu bar
2. **ImGui Context** - The ImGui context is already set when your callback is invoked
3. **Separator Usage** - Use `ImGui::Separator()` to logically group menu items
4. **Disabled Items** - Use the fourth parameter of `ImGui::MenuItem()` to disable items when appropriate
5. **Automatic Cleanup** - Hooks are cleaned up automatically; manual removal is not required
6. **Editor Guard** - Wrap all editor-specific code in `#if EDITOR` blocks
7. **RegisterEditorUI** - Register menus in `RegisterEditorUI`, not in `OnLoad`
