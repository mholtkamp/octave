# Viewport Context Menu & Overlays

## Overview

This example demonstrates two viewport extension features:
- **Viewport Context Menu**: Adding items to the right-click menu in the 3D viewport
- **Viewport Overlays**: Drawing custom overlays (text, shapes, guides) on top of the viewport each frame

## Files

### package.json

```json
{
    "name": "Viewport Extensions Addon",
    "author": "Octave Examples",
    "description": "Adds viewport context menu items and overlays.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "viewportextensionsaddon",
        "apiVersion": 2
    }
}
```

### Source/ViewportExtensionsAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static bool sShowGrid = true;
static bool sShowStats = true;

/**
 * @brief Context menu callback: spawn a node at the clicked position.
 */
static void OnSpawnAtCursor(void* userData)
{
    sEngineAPI->LogDebug("Spawn at cursor position");
}

/**
 * @brief Context menu callback: toggle grid overlay.
 */
static void OnToggleGrid(void* userData)
{
    sShowGrid = !sShowGrid;
    sEngineAPI->LogDebug("Grid overlay %s", sShowGrid ? "enabled" : "disabled");
}

/**
 * @brief Viewport overlay: draws stats and guides on the viewport.
 * @param viewportX Viewport top-left X coordinate (screen space).
 * @param viewportY Viewport top-left Y coordinate (screen space).
 * @param viewportW Viewport width in pixels.
 * @param viewportH Viewport height in pixels.
 * @param userData User data (unused).
 */
static void DrawStatsOverlay(float viewportX, float viewportY,
                              float viewportW, float viewportH, void* userData)
{
    if (!sShowStats) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw an info box in the top-right corner
    float boxW = 160.0f;
    float boxH = 50.0f;
    float margin = 10.0f;
    float boxX = viewportX + viewportW - boxW - margin;
    float boxY = viewportY + margin;

    drawList->AddRectFilled(
        ImVec2(boxX, boxY),
        ImVec2(boxX + boxW, boxY + boxH),
        IM_COL32(0, 0, 0, 160),
        4.0f
    );

    drawList->AddText(
        ImVec2(boxX + 8.0f, boxY + 6.0f),
        IM_COL32(200, 200, 200, 255),
        "Custom Overlay"
    );

    drawList->AddText(
        ImVec2(boxX + 8.0f, boxY + 24.0f),
        IM_COL32(150, 255, 150, 255),
        "Status: Active"
    );
}

/**
 * @brief Viewport overlay: draws a center crosshair.
 */
static void DrawCrosshairOverlay(float viewportX, float viewportY,
                                  float viewportW, float viewportH, void* userData)
{
    if (!sShowGrid) return;

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    float cx = viewportX + viewportW * 0.5f;
    float cy = viewportY + viewportH * 0.5f;
    float size = 20.0f;
    ImU32 color = IM_COL32(255, 255, 0, 100);

    drawList->AddLine(ImVec2(cx - size, cy), ImVec2(cx + size, cy), color);
    drawList->AddLine(ImVec2(cx, cy - size), ImVec2(cx, cy + size), color);
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
    // Viewport context menu items
    hooks->AddViewportContextItem(hookId, "Spawn Node at Cursor", OnSpawnAtCursor, nullptr);
    hooks->AddViewportContextItem(hookId, "Toggle Grid Overlay", OnToggleGrid, nullptr);

    // Viewport overlays (drawn every frame)
    hooks->RegisterViewportOverlay(hookId, "StatsOverlay", DrawStatsOverlay, nullptr);
    hooks->RegisterViewportOverlay(hookId, "CrosshairOverlay", DrawCrosshairOverlay, nullptr);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Viewport Extensions Addon";
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

### AddViewportContextItem

```cpp
void (*AddViewportContextItem)(HookId hookId, const char* itemPath,
    MenuCallback callback, void* userData);
```

Adds an item to the viewport right-click context menu.

### RegisterViewportOverlay

```cpp
void (*RegisterViewportOverlay)(HookId hookId, const char* overlayName,
    ViewportOverlayCallback drawFunc, void* userData);
```

Registers a per-frame overlay callback. The callback receives viewport bounds in screen coordinates.

### ViewportOverlayCallback

```cpp
typedef void (*ViewportOverlayCallback)(float viewportX, float viewportY,
    float viewportW, float viewportH, void* userData);
```

## Best Practices

1. **Use ImDrawList** - Access via `ImGui::GetWindowDrawList()` for drawing shapes, lines, and text
2. **Screen Coordinates** - All viewport parameters are in screen space; use them directly with ImDrawList
3. **Performance** - Overlays are called every frame; keep drawing lightweight
4. **Toggle Support** - Provide context menu items or shortcuts to toggle overlays on/off
5. **Semi-Transparent Backgrounds** - Use alpha values (e.g., `IM_COL32(0,0,0,160)`) for overlay boxes
