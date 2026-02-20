# Custom Gizmo Tools

## Overview

This example demonstrates how to register a custom gizmo tool that appears alongside the built-in Translate, Rotate, and Scale tools in the viewport toolbar. When your tool is active and a node is selected, your draw callback is invoked each frame.

## Files

### package.json

```json
{
    "name": "Gizmo Tool Addon",
    "author": "Octave Examples",
    "description": "Adds a custom gizmo tool to the viewport toolbar.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "gizmotooladdon",
        "apiVersion": 2
    }
}
```

### Source/GizmoToolAddon.cpp

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
 * @brief Draw callback for the custom gizmo tool.
 * Called each frame when this tool is active and a node is selected.
 *
 * @param selectedNode Pointer to the currently selected Node.
 * @param userData User data (unused).
 */
static void DrawAlignTool(void* selectedNode, void* userData)
{
    // Draw a small tool options panel
    ImGui::Text("Align Tool Options:");
    ImGui::Separator();

    if (ImGui::Button("Align to Grid"))
    {
        sEngineAPI->LogDebug("Aligning selected node to grid");
    }

    if (ImGui::Button("Align to Ground"))
    {
        sEngineAPI->LogDebug("Aligning selected node to ground");
    }

    if (ImGui::Button("Center on Parent"))
    {
        sEngineAPI->LogDebug("Centering selected node on parent");
    }
}

/**
 * @brief Draw callback for a snap tool.
 */
static void DrawSnapTool(void* selectedNode, void* userData)
{
    static float sSnapDistance = 1.0f;

    ImGui::Text("Snap Tool:");
    ImGui::Separator();
    ImGui::DragFloat("Snap Distance", &sSnapDistance, 0.1f, 0.1f, 100.0f);

    if (ImGui::Button("Snap to Nearest"))
    {
        sEngineAPI->LogDebug("Snapping to nearest object (dist=%.1f)", sSnapDistance);
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
    hooks->RegisterGizmoTool(
        hookId,
        "AlignTool",           // Unique tool name
        "A",                   // Icon character for toolbar button
        "Align Tool",          // Tooltip text
        DrawAlignTool,         // Draw callback
        nullptr                // User data
    );

    hooks->RegisterGizmoTool(
        hookId,
        "SnapTool",
        "S",
        "Snap Tool",
        DrawSnapTool,
        nullptr
    );
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Gizmo Tool Addon";
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

The viewport toolbar will show:

```
[T] [R] [S] [A] [S]
 ^   ^   ^   ^   ^
 |   |   |   |   └── Snap Tool (addon)
 |   |   |   └────── Align Tool (addon)
 |   |   └────────── Scale (built-in)
 |   └────────────── Rotate (built-in)
 └────────────────── Translate (built-in)
```

## API Reference

### RegisterGizmoTool

```cpp
void (*RegisterGizmoTool)(
    HookId hookId,
    const char* toolName,
    const char* iconText,
    const char* tooltip,
    GizmoToolDrawCallback drawFunc,
    void* userData
);
```

**Parameters:**
- `hookId` - The hook identifier
- `toolName` - Unique name for the tool
- `iconText` - Character(s) shown on the toolbar button
- `tooltip` - Tooltip displayed on hover
- `drawFunc` - Called each frame when tool is active: `void drawFunc(void* selectedNode, void* userData)`
- `userData` - Optional user data

### UnregisterGizmoTool

```cpp
void (*UnregisterGizmoTool)(HookId hookId, const char* toolName);
```

## Best Practices

1. **Icon Text** - Use a single character for consistency with built-in tools
2. **Node Required** - Your draw callback only fires when a node is selected
3. **Tool State** - Use static variables or userData to persist tool settings
4. **Compact UI** - Keep the tool options UI small since it shares space with the viewport
