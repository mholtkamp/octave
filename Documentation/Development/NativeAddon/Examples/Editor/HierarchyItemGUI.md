# Hierarchy & Asset Browser Item GUI

## Overview

This example demonstrates how to draw custom overlays on hierarchy tree items and asset browser entries. Use this to add icons, status indicators, badges, or interactive buttons to individual items in these panels.

## Files

### package.json

```json
{
    "name": "Item GUI Overlay Addon",
    "author": "Octave Examples",
    "description": "Adds custom overlays to hierarchy and asset browser items.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "itemguiaddon",
        "apiVersion": 2
    }
}
```

### Source/ItemGUIAddon.cpp

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
 * @brief Draws a status indicator on each hierarchy node.
 * @param node Pointer to the Node being drawn.
 * @param rowX Row left edge X (screen coords).
 * @param rowY Row top edge Y (screen coords).
 * @param rowW Row width.
 * @param rowH Row height.
 * @param userData User data (unused).
 */
static void DrawHierarchyOverlay(void* node, float rowX, float rowY,
                                  float rowW, float rowH, void* userData)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw a small colored dot on the right side of each row
    float dotRadius = 4.0f;
    float dotX = rowX + rowW - dotRadius - 4.0f;
    float dotY = rowY + rowH * 0.5f;

    // Green dot (could vary based on node state)
    drawList->AddCircleFilled(ImVec2(dotX, dotY), dotRadius, IM_COL32(100, 200, 100, 200));
}

/**
 * @brief Draws a type badge on each asset browser entry.
 * @param assetName Name of the asset.
 * @param assetType Type name of the asset (e.g., "StaticMesh").
 * @param rowX Row left edge X (screen coords).
 * @param rowY Row top edge Y (screen coords).
 * @param rowW Row width.
 * @param rowH Row height.
 * @param userData User data (unused).
 */
static void DrawAssetOverlay(const char* assetName, const char* assetType,
                              float rowX, float rowY, float rowW, float rowH,
                              void* userData)
{
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw a small type abbreviation badge on the right
    const char* abbr = "?";
    ImU32 badgeColor = IM_COL32(80, 80, 80, 200);

    if (assetType != nullptr)
    {
        if (strcmp(assetType, "StaticMesh") == 0)    { abbr = "SM"; badgeColor = IM_COL32(100, 150, 200, 200); }
        else if (strcmp(assetType, "Texture") == 0)  { abbr = "TX"; badgeColor = IM_COL32(200, 150, 100, 200); }
        else if (strcmp(assetType, "Scene") == 0)    { abbr = "SC"; badgeColor = IM_COL32(100, 200, 100, 200); }
        else if (strcmp(assetType, "SoundWave") == 0){ abbr = "AU"; badgeColor = IM_COL32(200, 100, 200, 200); }
    }

    float badgeW = 24.0f;
    float badgeH = 14.0f;
    float badgeX = rowX + rowW - badgeW - 4.0f;
    float badgeY = rowY + (rowH - badgeH) * 0.5f;

    drawList->AddRectFilled(
        ImVec2(badgeX, badgeY),
        ImVec2(badgeX + badgeW, badgeY + badgeH),
        badgeColor, 2.0f
    );

    drawList->AddText(
        ImVec2(badgeX + 3.0f, badgeY),
        IM_COL32(255, 255, 255, 220),
        abbr
    );
}

/**
 * @brief Hierarchy changed event callback.
 * @param changeType 0=NodeCreated, 1=NodeDestroyed, 2=NodeReparented, 3=NodeRenamed.
 * @param node Pointer to the affected node.
 * @param userData User data (unused).
 */
static void OnHierarchyChanged(int32_t changeType, void* node, void* userData)
{
    const char* typeNames[] = { "Created", "Destroyed", "Reparented", "Renamed" };
    const char* typeName = (changeType >= 0 && changeType < 4) ? typeNames[changeType] : "Unknown";
    sEngineAPI->LogDebug("Hierarchy changed: %s", typeName);
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
    hooks->RegisterHierarchyItemGUI(hookId, DrawHierarchyOverlay, nullptr);
    hooks->RegisterAssetItemGUI(hookId, DrawAssetOverlay, nullptr);
    hooks->RegisterOnHierarchyChanged(hookId, OnHierarchyChanged, nullptr);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Item GUI Overlay Addon";
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

### RegisterHierarchyItemGUI

```cpp
void (*RegisterHierarchyItemGUI)(HookId hookId, HierarchyItemGUICallback drawFunc, void* userData);
```

Called for each visible node in the hierarchy. Use `ImGui::GetWindowDrawList()` to draw overlays.

### RegisterAssetItemGUI

```cpp
void (*RegisterAssetItemGUI)(HookId hookId, AssetItemGUICallback drawFunc, void* userData);
```

Called for each visible asset in the browser. Receives asset name, type, and row bounds.

### RegisterOnHierarchyChanged

```cpp
void (*RegisterOnHierarchyChanged)(HookId hookId, HierarchyChangedCallback cb, void* userData);
```

Fired when nodes are created, destroyed, reparented, or renamed. `changeType`: 0=Created, 1=Destroyed, 2=Reparented, 3=Renamed.

## Best Practices

1. **Use ImDrawList** - Draw overlays with `ImGui::GetWindowDrawList()`, not regular ImGui widgets
2. **Keep It Light** - These callbacks run for every visible item every frame
3. **Right-Aligned** - Place indicators on the right side to avoid overlapping item labels
4. **Semi-Transparent** - Use alpha values so overlays don't obscure the underlying content
5. **Conditional Drawing** - Check node/asset state before drawing to avoid unnecessary visuals
