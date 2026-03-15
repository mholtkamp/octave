# Custom Keyboard Shortcuts

## Overview

This example demonstrates how to register custom keyboard shortcuts that trigger addon actions. Shortcuts are processed each frame and respect modifier keys (Ctrl, Shift, Alt). They are automatically disabled when the user is typing in a text field.

## Supported Key Bindings

**Format:** `"Modifier+Modifier+Key"` (case-insensitive)

**Modifiers:** `Ctrl`, `Shift`, `Alt`

**Keys:**
| Category | Keys |
|----------|------|
| Letters | `A` - `Z` |
| Numbers | `0` - `9` |
| Function | `F1` - `F12` |
| Special | `Space`, `Enter`/`Return`, `Escape`/`Esc`, `Tab`, `Delete`/`Del`, `Backspace` |

**Examples:** `"Ctrl+Shift+M"`, `"Alt+F5"`, `"Ctrl+1"`, `"F9"`

## Files

### package.json

```json
{
    "name": "Keyboard Shortcuts Addon",
    "author": "Octave Examples",
    "description": "Registers custom keyboard shortcuts.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "shortcutsaddon",
        "apiVersion": 2
    }
}
```

### Source/ShortcutsAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
static bool sPanelVisible = false;

static void OnTogglePanel(void* userData)
{
    sPanelVisible = !sPanelVisible;
    sEngineAPI->LogDebug("Panel %s (Ctrl+Shift+P)", sPanelVisible ? "shown" : "hidden");
}

static void OnQuickSave(void* userData)
{
    sEngineAPI->LogDebug("Quick save triggered (Ctrl+Shift+S)");
}

static void OnDebugAction(void* userData)
{
    sEngineAPI->LogDebug("Debug action triggered (F9)");
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
    hooks->RegisterShortcut(
        hookId,
        "myaddon.toggle_panel",        // Unique shortcut ID
        "My Addon: Toggle Panel",      // Display name
        "Ctrl+Shift+P",                // Default key binding
        OnTogglePanel,                  // Callback
        nullptr                         // User data
    );

    hooks->RegisterShortcut(
        hookId,
        "myaddon.quick_save",
        "My Addon: Quick Save",
        "Ctrl+Shift+S",
        OnQuickSave,
        nullptr
    );

    hooks->RegisterShortcut(
        hookId,
        "myaddon.debug_action",
        "My Addon: Debug Action",
        "F9",
        OnDebugAction,
        nullptr
    );
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Keyboard Shortcuts Addon";
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

### RegisterShortcut

```cpp
void (*RegisterShortcut)(
    HookId hookId,
    const char* shortcutId,
    const char* displayName,
    const char* defaultBinding,
    ShortcutCallback callback,
    void* userData
);
```

**Parameters:**
- `hookId` - The hook identifier
- `shortcutId` - Unique ID (e.g., `"myaddon.toggle_panel"`)
- `displayName` - Human-readable name (e.g., `"My Addon: Toggle Panel"`)
- `defaultBinding` - Key binding string (e.g., `"Ctrl+Shift+P"`)
- `callback` - Function called when shortcut is pressed: `void callback(void* userData)`
- `userData` - Optional user data

### UnregisterShortcut

```cpp
void (*UnregisterShortcut)(HookId hookId, const char* shortcutId);
```

## Best Practices

1. **Unique IDs** - Use `"addonname.action"` format to avoid conflicts
2. **Avoid Conflicts** - Don't override common editor shortcuts (Ctrl+S, Ctrl+Z, Ctrl+C, etc.)
3. **Modifier Keys** - Use at least one modifier (Ctrl/Shift/Alt) to avoid conflicting with typing
4. **Text Input Safety** - Shortcuts are automatically suppressed when a text field has focus
5. **Exact Match** - Modifiers must match exactly: `Ctrl+A` won't fire if Shift is also held
