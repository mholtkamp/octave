# Example: CustomMenuItem

A native addon that demonstrates adding custom menu items to the editor.

---

## Overview

This example demonstrates:
- Using the `EditorUIHooks` system to add menu items
- Creating submenus and separators
- Keyboard shortcuts for menu items
- Editor-only code with `#if EDITOR` guards

---

## Files

### package.json

```json
{
    "name": "Custom Menu Item",
    "author": "Octave Examples",
    "description": "Demonstrates adding custom menu items to the editor.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "custommenuitem",
        "apiVersion": 2
    }
}
```

> **Note:** `"target": "editor"` means this addon only runs in the editor and won't be compiled into final game builds.

### Source/CustomMenuItem.cpp

```cpp
/**
 * @file CustomMenuItem.cpp
 * @brief Demonstrates adding custom menu items to the editor.
 *
 * This example shows how to:
 * - Use EditorUIHooks to add custom menus
 * - Access engine subsystems directly via OctaveEngineAPI
 * - Use proper #if EDITOR guards for editor-only code
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
// ImGui is available in editor builds for custom windows
#include "imgui.h"
#endif

// GLM for math operations (available to all addons)
#include "glm/glm.hpp"

static OctaveEngineAPI* sEngineAPI = nullptr;
static uint64_t sHookId = 0;

//=============================================================================
// Menu Callbacks
//=============================================================================

#if EDITOR

static void OnHelloWorld(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Hello from custom menu item!");

        // Example: Use direct engine API access
        int32_t numWorlds = sEngineAPI->GetNumWorlds();
        sEngineAPI->LogDebug("Number of active worlds: %d", numWorlds);

        // Get elapsed time since engine start
        float elapsedTime = sEngineAPI->GetElapsedTime();
        sEngineAPI->LogDebug("Engine running for %.2f seconds", elapsedTime);
    }
}

static void OnOpenDocs(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Opening documentation...");
    }
    // In a real addon, you might open a URL or window here
}

static void OnResetSettings(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Settings reset to defaults!");

        // Example: Reset audio to default volume
        sEngineAPI->SetMasterVolume(1.0f);
        sEngineAPI->LogDebug("Audio volume reset to 100%%");
    }
}

static void OnToggleFeatureA(void* userData)
{
    static bool enabled = false;
    enabled = !enabled;

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Feature A is now %s", enabled ? "ON" : "OFF");
    }
}

static void OnToggleFeatureB(void* userData)
{
    static bool enabled = true;
    enabled = !enabled;

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Feature B is now %s", enabled ? "ON" : "OFF");
    }
}

static void OnAbout(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Custom Menu Item Example v1.0.0");
        sEngineAPI->LogDebug("This addon demonstrates the EditorUIHooks system.");
    }
}

static void OnQuickAction(void* userData)
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Quick action triggered via Ctrl+Shift+Q!");
    }
}

#endif // EDITOR

//=============================================================================
// Plugin Callbacks
//=============================================================================

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("CustomMenuItem addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomMenuItem addon unloaded.");
    }
    sEngineAPI = nullptr;
}

#if EDITOR
static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;

    //=========================================================================
    // Adding to existing menus
    //=========================================================================

    // Add item to the Developer menu
    hooks->AddMenuItem(
        hookId,
        "Developer",           // Menu to add to
        "Hello World",         // Item text
        OnHelloWorld,          // Callback
        nullptr,               // User data
        nullptr                // No shortcut
    );

    // Add item with keyboard shortcut
    hooks->AddMenuItem(
        hookId,
        "Developer",
        "Quick Action",
        OnQuickAction,
        nullptr,
        "Ctrl+Shift+Q"         // Shortcut displayed (note: actual binding needs input system)
    );

    //=========================================================================
    // Creating submenus
    //=========================================================================

    // Items with "/" create submenus automatically
    hooks->AddMenuItem(hookId, "Developer", "My Addon/Open Documentation", OnOpenDocs, nullptr, "F1");
    hooks->AddMenuItem(hookId, "Developer", "My Addon/Reset Settings", OnResetSettings, nullptr, nullptr);

    // Add a separator in the submenu
    hooks->AddMenuSeparator(hookId, "Developer");

    // More submenu items
    hooks->AddMenuItem(hookId, "Developer", "My Addon/Features/Toggle Feature A", OnToggleFeatureA, nullptr, nullptr);
    hooks->AddMenuItem(hookId, "Developer", "My Addon/Features/Toggle Feature B", OnToggleFeatureB, nullptr, nullptr);

    //=========================================================================
    // Adding to other menus
    //=========================================================================

    // Add to Help menu
    hooks->AddMenuItem(hookId, "Help", "About My Addon", OnAbout, nullptr, nullptr);

    //=========================================================================
    // Creating a top-level menu (if supported)
    //=========================================================================

    // This creates a new top-level menu called "My Addon"
    // hooks->AddMenuItem(hookId, "My Addon", "Settings...", OnOpenSettings, nullptr, nullptr);
    // hooks->AddMenuItem(hookId, "My Addon", "About", OnAbout, nullptr, nullptr);

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomMenuItem: Registered editor UI hooks");
    }
}
#endif

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Menu Item";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;          // Editor-only addon, no gameplay tick
    desc->TickEditor = nullptr;    // No per-frame editor updates needed
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

## Menu Structure Created

After loading this addon, the Developer menu will contain:

```
Developer
├── Hello World
├── Quick Action                    Ctrl+Shift+Q
├── ─────────────────────────────── (separator)
└── My Addon
    ├── Open Documentation          F1
    ├── Reset Settings
    └── Features
        ├── Toggle Feature A
        └── Toggle Feature B
```

And the Help menu will contain:

```
Help
├── ... (existing items)
└── About My Addon
```

---

## API Reference

### hooks->AddMenuItem(hookId, menuPath, itemPath, callback, userData, shortcut)

Adds a menu item to the editor.

**Parameters:**
- `hookId` (uint64_t): Your plugin's hook ID (provided to RegisterEditorUI)
- `menuPath` (const char*): Top-level menu name ("Developer", "Help", "Edit", etc.)
- `itemPath` (const char*): Item path, use "/" for submenus ("My Tool" or "Submenu/Item")
- `callback` (MenuCallback): Function to call when clicked: `void callback(void* userData)`
- `userData` (void*): Custom data passed to callback
- `shortcut` (const char*): Display text for keyboard shortcut (e.g., "Ctrl+S"), or nullptr

---

### hooks->AddMenuSeparator(hookId, menuPath)

Adds a separator line in a menu.

**Parameters:**
- `hookId` (uint64_t): Your plugin's hook ID
- `menuPath` (const char*): Menu to add separator to

---

### hooks->RemoveMenuItem(hookId, menuPath, itemPath)

Removes a previously added menu item.

**Parameters:**
- `hookId` (uint64_t): Your plugin's hook ID
- `menuPath` (const char*): Menu containing the item
- `itemPath` (const char*): Item path to remove

---

### hooks->RemoveAllHooks(hookId)

Removes all hooks registered with this hookId. Called automatically on plugin unload.

---

## Best Practices

### 1. Use Descriptive Names

```cpp
// Good - Clear and professional
hooks->AddMenuItem(hookId, "Developer", "My Addon/Export Scene Data", ...);

// Bad - Vague
hooks->AddMenuItem(hookId, "Developer", "Do Thing", ...);
```

### 2. Group Related Items in Submenus

```cpp
// Group related features
hooks->AddMenuItem(hookId, "Developer", "Level Tools/Validate Geometry", ...);
hooks->AddMenuItem(hookId, "Developer", "Level Tools/Optimize Meshes", ...);
hooks->AddMenuItem(hookId, "Developer", "Level Tools/Check Collisions", ...);
```

### 3. Use Separators to Organize

```cpp
hooks->AddMenuItem(hookId, "Developer", "My Addon/Primary Action", ...);
hooks->AddMenuItem(hookId, "Developer", "My Addon/Secondary Action", ...);
hooks->AddMenuSeparator(hookId, "Developer");
hooks->AddMenuItem(hookId, "Developer", "My Addon/Settings...", ...);
```

### 4. Consistent Shortcut Style

```cpp
// Follow platform conventions
"Ctrl+S"        // Save
"Ctrl+Shift+S"  // Save As
"F5"            // Play
"Ctrl+Z"        // Undo
```

### 5. Guard Editor Code

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

## Common Menu Paths

| Menu Path | Description |
|-----------|-------------|
| `"File"` | File operations |
| `"Edit"` | Edit operations (undo, preferences) |
| `"View"` | View/display options |
| `"Developer"` | Developer/debug tools (recommended for addons) |
| `"Help"` | Help and about |

---

## Troubleshooting

### Menu item doesn't appear

1. Check that `RegisterEditorUI` is being called (add a log message)
2. Verify the hookId is valid
3. Ensure the menu path is correct (case-sensitive)

### Callback not firing

1. Verify callback function signature: `void callback(void* userData)`
2. Check that the callback isn't null
3. Look for errors in the console

### Shortcut not working

Note: The shortcut parameter is for **display only**. To actually bind a keyboard shortcut, you need to use the input system. The shortcut text is shown in the menu for user reference.
