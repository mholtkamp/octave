# Example: CustomContextMenuItem

A native addon that demonstrates adding custom context menu items to the editor's hierarchy and asset browser.

---

## Overview

This example demonstrates:
- Using `EditorUIHooks` to add context menu items to the node hierarchy right-click menu
- Using `EditorUIHooks` to add context menu items to the asset browser right-click menu
- Filtering asset context items by asset type
- Editor-only code with `#if EDITOR` guards

---

## Files

### package.json

```json
{
    "name": "Custom Context Menu Item",
    "author": "Octave Examples",
    "description": "Demonstrates adding custom context menu items to the editor.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "customcontextmenuitem",
        "apiVersion": 2
    }
}
```

> **Note:** `"target": "editor"` means this addon only runs in the editor and won't be compiled into final game builds.

### Source/CustomContextMenuItem.cpp

```cpp
/**
 * @file CustomContextMenuItem.cpp
 * @brief Demonstrates adding custom context menu items to the editor.
 *
 * This example shows how to:
 * - Use EditorUIHooks to add node hierarchy context menu items
 * - Use EditorUIHooks to add asset browser context menu items
 * - Filter asset context items by asset type
 * - Use proper #if EDITOR guards for editor-only code
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

#include "glm/glm.hpp"

static OctaveEngineAPI* sEngineAPI = nullptr;
static uint64_t sHookId = 0;

//=============================================================================
// Context Menu Callbacks
//=============================================================================

#if EDITOR

/**
 * @brief Called when the "Log Node Info" context menu item is clicked.
 * @param userData User data (unused).
 */
static void OnLogNodeInfo(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Node context menu item clicked!");
        sEngineAPI->LogDebug("Use this to perform operations on the selected node.");
    }
}

/**
 * @brief Called when the "Inspect Mesh" asset context menu item is clicked.
 * @param userData User data (unused).
 */
static void OnInspectMesh(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Inspect Mesh clicked!");
        sEngineAPI->LogDebug("This item only appears when right-clicking StaticMesh assets.");
    }
}

/**
 * @brief Called when the "Export Asset Info" context menu item is clicked.
 * @param userData User data (unused).
 */
static void OnExportAssetInfo(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Export Asset Info clicked!");
        sEngineAPI->LogDebug("This item appears for all asset types.");
    }
}

#endif // EDITOR

//=============================================================================
// Plugin Callbacks
//=============================================================================

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("CustomContextMenuItem addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomContextMenuItem addon unloaded.");
    }
    sEngineAPI = nullptr;
}

#if EDITOR
/**
 * @brief Registers editor UI hooks for context menu items.
 * @param hooks The editor UI hooks interface.
 * @param hookId Unique identifier for this addon's hooks.
 */
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;

    //=========================================================================
    // Node hierarchy context menu items
    //=========================================================================

    // This item appears in the right-click menu for any node in the hierarchy
    hooks->AddNodeContextItem(
        hookId,
        "Log Node Info",       // Item text shown in the context menu
        OnLogNodeInfo,         // Callback function
        nullptr                // User data
    );

    //=========================================================================
    // Asset browser context menu items
    //=========================================================================

    // This item only appears when right-clicking a StaticMesh asset
    hooks->AddAssetContextItem(
        hookId,
        "Inspect Mesh",        // Item text
        "StaticMesh",          // Asset type filter (only StaticMesh assets)
        OnInspectMesh,         // Callback
        nullptr                // User data
    );

    // This item appears for all asset types (use "*" for no filtering)
    hooks->AddAssetContextItem(
        hookId,
        "Export Asset Info",   // Item text
        "*",                   // No type filter - appears for all assets
        OnExportAssetInfo,     // Callback
        nullptr                // User data
    );

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomContextMenuItem: Registered context menu hooks");
    }
}
#endif

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Context Menu Item";
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

    return 0;
}
```

---

## Context Menus Modified

After loading this addon:

**Node hierarchy right-click menu** will contain:

```
(existing items)
├── ...
├── ─────────────────────────────── (separator)
└── Log Node Info
```

**Asset browser right-click menu** (on a StaticMesh asset) will contain:

```
(existing items)
├── ...
├── ─────────────────────────────── (separator)
├── Inspect Mesh
└── Export Asset Info
```

**Asset browser right-click menu** (on any other asset) will contain:

```
(existing items)
├── ...
├── ─────────────────────────────── (separator)
└── Export Asset Info
```

---

## API Reference

### hooks->AddNodeContextItem(hookId, itemPath, callback, userData)

Adds a context menu item to the node hierarchy right-click menu.

**Parameters:**
- `hookId` (uint64_t): Your plugin's hook ID (provided to RegisterEditorUI)
- `itemPath` (const char*): Display text for the context menu item
- `callback` (MenuCallback): Function to call when clicked: `void callback(void* userData)`
- `userData` (void*): Custom data passed to callback

---

### hooks->AddAssetContextItem(hookId, itemPath, assetTypeFilter, callback, userData)

Adds a context menu item to the asset browser right-click menu.

**Parameters:**
- `hookId` (uint64_t): Your plugin's hook ID (provided to RegisterEditorUI)
- `itemPath` (const char*): Display text for the context menu item
- `assetTypeFilter` (const char*): Asset type to filter by (e.g., `"StaticMesh"`, `"Texture"`, `"Scene"`), or `"*"` for all types
- `callback` (MenuCallback): Function to call when clicked: `void callback(void* userData)`
- `userData` (void*): Custom data passed to callback

---

### hooks->RemoveAllHooks(hookId)

Removes all hooks registered with this hookId. Called automatically on plugin unload.

---

## Best Practices

### 1. Use Descriptive Names

```cpp
// Good - Clear about what it does
hooks->AddNodeContextItem(hookId, "Export Node Transform", ...);

// Bad - Vague
hooks->AddNodeContextItem(hookId, "Do Thing", ...);
```

### 2. Filter by Asset Type When Appropriate

```cpp
// Only show for mesh assets
hooks->AddAssetContextItem(hookId, "Optimize Mesh", "StaticMesh", ...);

// Only show for materials
hooks->AddAssetContextItem(hookId, "Preview Material", "MaterialLite", ...);

// Show for all assets
hooks->AddAssetContextItem(hookId, "Copy Asset Path", "*", ...);
```

### 3. Guard Editor Code

```cpp
#if EDITOR
// All editor-specific code here
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    // ...
}
#endif

// In plugin descriptor:
#if EDITOR
    desc->RegisterEditorUI = RegisterEditorUI;
#else
    desc->RegisterEditorUI = nullptr;
#endif
```

---

## Common Asset Type Names

| Type Name | Description |
|-----------|-------------|
| `"StaticMesh"` | Static mesh geometry |
| `"SkeletalMesh"` | Skeletal mesh with bones |
| `"MaterialLite"` | Lightweight material |
| `"MaterialBase"` | Full-featured material |
| `"MaterialInstance"` | Material instance |
| `"Scene"` | Scene asset |
| `"Texture"` | Texture asset |
| `"SoundWave"` | Audio asset |
| `"ParticleSystem"` | Particle system |
| `"*"` | All asset types |

---

## Troubleshooting

### Context menu item doesn't appear

1. Check that `RegisterEditorUI` is being called (add a log message)
2. Verify the hookId is valid
3. For asset context items, check that the asset type filter matches the asset you're right-clicking

### Callback not firing

1. Verify callback function signature: `void callback(void* userData)`
2. Check that the callback isn't null
3. Look for errors in the console

### Asset type filter not working

The `assetTypeFilter` parameter is case-sensitive and must match the asset's type name exactly. Use `"*"` to match all types, or check the Common Asset Type Names table above.
