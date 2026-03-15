# Extending the Add Node Menu

## Overview

This example demonstrates how to extend the "Add Node" submenu that appears in the hierarchy context menu and World > Spawn Node. You can add items to existing categories (3D, Widget, Other) or create entirely new categories.

## Files

### package.json

```json
{
    "name": "Custom Node Types Addon",
    "author": "Octave Examples",
    "description": "Adds custom node types to the Add Node menu.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "customnodetypesaddon",
        "apiVersion": 2
    }
}
```

### Source/CustomNodeTypesAddon.cpp

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
 * @brief Adds items to the "3D" category of the Add Node menu.
 * @param parentNode The node to parent the new node under (may be null).
 * @param userData User data (unused).
 */
static void DrawCustom3dNodes(void* parentNode, void* userData)
{
    if (ImGui::MenuItem("Waypoint3D"))
    {
        sEngineAPI->LogDebug("Spawning Waypoint3D node");
    }

    if (ImGui::MenuItem("TriggerVolume3D"))
    {
        sEngineAPI->LogDebug("Spawning TriggerVolume3D node");
    }
}

/**
 * @brief Creates a new "AI" category in the Add Node menu.
 * @param parentNode The node to parent the new node under (may be null).
 * @param userData User data (unused).
 */
static void DrawAINodes(void* parentNode, void* userData)
{
    if (ImGui::MenuItem("NavMeshAgent"))
    {
        sEngineAPI->LogDebug("Spawning NavMeshAgent node");
    }

    if (ImGui::MenuItem("AIController"))
    {
        sEngineAPI->LogDebug("Spawning AIController node");
    }

    if (ImGui::MenuItem("BehaviorTree"))
    {
        sEngineAPI->LogDebug("Spawning BehaviorTree node");
    }
}

/**
 * @brief Adds items to the "Create Asset" submenu in the asset browser.
 * @param parentNode Unused for asset creation.
 * @param userData User data (unused).
 */
static void DrawCustomAssetTypes(void* parentNode, void* userData)
{
    if (ImGui::MenuItem("AI Behavior"))
    {
        sEngineAPI->LogDebug("Creating AI Behavior asset...");
    }

    if (ImGui::MenuItem("Dialogue Tree"))
    {
        sEngineAPI->LogDebug("Creating Dialogue Tree asset...");
    }
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
    // Add items to existing "3D" category
    hooks->AddNodeMenuItems(hookId, "3D", DrawCustom3dNodes, nullptr);

    // Create a new "AI" category submenu
    hooks->AddNodeMenuItems(hookId, "AI", DrawAINodes, nullptr);

    // Extend the Create Asset submenu
    hooks->AddCreateAssetItems(hookId, DrawCustomAssetTypes, nullptr);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Node Types Addon";
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

The Add Node menu will look like:

```
Node
3D >
    (built-in 3D nodes)
    Waypoint3D          <-- addon item
    TriggerVolume3D     <-- addon item
Widget >
    (built-in widgets)
AI >                    <-- new custom category
    NavMeshAgent
    AIController
    BehaviorTree
Other >
    (built-in other nodes)
```

The Create Asset submenu will have:

```
Create Asset >
    (built-in asset types)
    AI Behavior         <-- addon item
    Dialogue Tree       <-- addon item
```

## API Reference

### AddNodeMenuItems

```cpp
void (*AddNodeMenuItems)(HookId hookId, const char* category,
    MenuSectionDrawCallback drawFunc, void* userData);
```

**Parameters:**
- `hookId` - The hook identifier
- `category` - Category name: `"3D"`, `"Widget"`, `"Other"`, or any custom name for a new submenu
- `drawFunc` - Callback: `void drawFunc(void* parentNode, void* userData)`
- `userData` - Optional user data

### AddCreateAssetItems

```cpp
void (*AddCreateAssetItems)(HookId hookId, MenuSectionDrawCallback drawFunc, void* userData);
```

**Parameters:**
- `hookId` - The hook identifier
- `drawFunc` - Callback: `void drawFunc(void* parentNode, void* userData)`
- `userData` - Optional user data

### AddSpawnBasic3dItems / AddSpawnBasicWidgetItems

```cpp
void (*AddSpawnBasic3dItems)(HookId hookId, MenuSectionDrawCallback drawFunc, void* userData);
void (*AddSpawnBasicWidgetItems)(HookId hookId, MenuSectionDrawCallback drawFunc, void* userData);
```

Extends the Spawn Basic 3D/Widget menus (Shift+Q/Shift+W hotkey menus).

## Best Practices

1. **Built-in Categories** - Use `"3D"`, `"Widget"`, or `"Other"` to add items to existing submenus
2. **Custom Categories** - Use any other string to create a new submenu section
3. **Parent Node** - The `parentNode` parameter is the node the user right-clicked on; use it to parent new nodes
4. **Node Spawning** - In a real addon, call the engine API to spawn nodes instead of just logging
