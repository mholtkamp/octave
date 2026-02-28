# Toolbar Extension

## Overview

This example demonstrates how to add custom buttons and controls to the editor's viewport toolbar using `AddToolbarItem`. Toolbar items appear in the horizontal bar above the viewport alongside existing editor tools. Cleanup is automatic when the plugin unloads.

## Files

### package.json

```json
{
    "name": "Toolbar Extension Addon",
    "author": "Octave Examples",
    "description": "Adds custom toolbar buttons to the viewport.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "toolbarextensionaddon",
        "apiVersion": 2
    }
}
```

### Source/ToolbarExtensionAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static bool sCustomToolEnabled = false;

/**
 * @brief Draw callback for a simple action button.
 * @param userData User-provided data (unused in this example).
 */
static void DrawActionButton(void* userData)
{
    if (ImGui::Button("Quick Action"))
    {
        sEngineAPI->LogDebug("Quick action button clicked!");
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Executes a quick action");
    }
}

/**
 * @brief Draw callback for a toggle tool button.
 * @param userData User-provided data (unused in this example).
 */
static void DrawToggleTool(void* userData)
{
    if (sCustomToolEnabled)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    }

    if (ImGui::Button("Custom Tool"))
    {
        sCustomToolEnabled = !sCustomToolEnabled;
        sEngineAPI->LogDebug("Custom tool %s", sCustomToolEnabled ? "enabled" : "disabled");
    }

    if (sCustomToolEnabled)
    {
        ImGui::PopStyleColor();
    }

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("Toggle custom editing tool");
    }

    // Show additional controls when tool is active
    if (sCustomToolEnabled)
    {
        ImGui::SameLine();
        if (ImGui::SmallButton("Options"))
        {
            sEngineAPI->LogDebug("Tool options opened");
        }
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
    api->LogDebug("Toolbar Extension Addon loaded!");
    return 0;
}

/**
 * @brief Called when the plugin is about to be unloaded.
 */
static void OnUnload()
{
    if (sEngineAPI) sEngineAPI->LogDebug("Toolbar Extension Addon unloading.");
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
    hooks->AddToolbarItem(hookId, "QuickAction", DrawActionButton, nullptr);
    hooks->AddToolbarItem(hookId, "CustomTool", DrawToggleTool, nullptr);
    sEngineAPI->LogDebug("Toolbar items registered");
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Toolbar Extension Addon";
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

### AddToolbarItem

```cpp
void (*AddToolbarItem)(HookId hookId, const char* itemName, ToolbarDrawCallback drawFunc, void* userData);
```

Adds a custom item to the viewport toolbar.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `itemName` - Unique name for the toolbar item
- `drawFunc` - Callback invoked each frame to draw the toolbar item
- `userData` - Optional user data passed to the callback

### RemoveToolbarItem

```cpp
void (*RemoveToolbarItem)(HookId hookId, const char* itemName);
```

Removes a previously registered toolbar item. Called automatically on plugin unload.

**Parameters:**
- `hookId` - The hook identifier provided by `RegisterEditorUI`
- `itemName` - Name of the toolbar item to remove

### ToolbarDrawCallback

```cpp
typedef void (*ToolbarDrawCallback)(void* userData);
```

Callback signature for toolbar item drawing functions.

**Parameters:**
- `userData` - User-provided data from registration

## Toolbar Item Patterns

### Action Buttons

Simple buttons that trigger one-time actions.

```cpp
if (ImGui::Button("Action")) { /* Execute action */ }
```

### Toggle Buttons

Buttons that switch between active/inactive states with visual feedback.

```cpp
if (gToolActive) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
if (ImGui::Button("Tool")) gToolActive = !gToolActive;
if (gToolActive) ImGui::PopStyleColor();
```

### Compound Controls

Multiple controls grouped together using `ImGui::SameLine()`.

```cpp
ImGui::Button("Main");
ImGui::SameLine();
ImGui::SmallButton("Sub");
```

## Best Practices

1. **Tooltips** - Use `ImGui::SetTooltip()` on hover for better UX
2. **Visual Feedback** - Use color changes to show active states
3. **Sizing** - Keep toolbar items compact; use `ImGui::SetNextItemWidth()` for combos
4. **State Management** - Track tool state in static variables or user data
5. **Automatic Cleanup** - Hooks are cleaned up automatically; manual removal is not required
6. **RegisterEditorUI** - Register toolbar items in `RegisterEditorUI`, not in `OnLoad`
7. **ImGui Context** - The ImGui context is already set when your callback is invoked
8. **Spacing** - Use `ImGui::SameLine()` carefully to avoid toolbar overflow
