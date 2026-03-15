# Custom Preferences Panel

## Overview

This example demonstrates how to add a custom preferences panel to the editor's Preferences window. Addon panels appear under an "Addons" section in the sidebar. You can optionally provide load/save callbacks for persisting settings.

## Files

### package.json

```json
{
    "name": "Preferences Panel Addon",
    "author": "Octave Examples",
    "description": "Adds a custom preferences panel to the editor.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "prefspaneladdon",
        "apiVersion": 2
    }
}
```

### Source/PrefsPanelAddon.cpp

```cpp
#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
#include "imgui.h"
#endif

static OctaveEngineAPI* sEngineAPI = nullptr;

#if EDITOR
// Settings state
static bool sEnableFeatureA = true;
static bool sEnableFeatureB = false;
static float sCustomScale = 1.0f;
static int sQualityLevel = 1;
static char sServerAddress[256] = "localhost";

/**
 * @brief Draw callback for the preferences panel.
 * @param userData User data (unused).
 *
 * Called each frame when this panel is selected in the Preferences window.
 * Use ImGui widgets to create the settings UI.
 */
static void DrawPreferencesPanel(void* userData)
{
    ImGui::Text("My Addon Settings");
    ImGui::Separator();
    ImGui::Spacing();

    // Boolean toggles
    ImGui::Checkbox("Enable Feature A", &sEnableFeatureA);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Enables the primary feature of this addon");

    ImGui::Checkbox("Enable Feature B", &sEnableFeatureB);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Slider
    ImGui::SliderFloat("Custom Scale", &sCustomScale, 0.1f, 5.0f, "%.2f");

    // Combo box
    const char* qualityNames[] = { "Low", "Medium", "High", "Ultra" };
    ImGui::Combo("Quality Level", &sQualityLevel, qualityNames, 4);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Text input
    ImGui::InputText("Server Address", sServerAddress, sizeof(sServerAddress));
}

/**
 * @brief Load callback - called when Preferences window opens or Cancel is pressed.
 * @param userData User data (unused).
 *
 * Load your settings from disk here (e.g., from a JSON file).
 */
static void LoadPreferences(void* userData)
{
    sEngineAPI->LogDebug("MyAddon: Loading preferences...");
    // In a real addon, load from a config file here
}

/**
 * @brief Save callback - called when Apply or OK is pressed.
 * @param userData User data (unused).
 *
 * Save your settings to disk here.
 */
static void SavePreferences(void* userData)
{
    sEngineAPI->LogDebug("MyAddon: Saving preferences...");
    sEngineAPI->LogDebug("  FeatureA=%d, FeatureB=%d, Scale=%.2f, Quality=%d",
        sEnableFeatureA, sEnableFeatureB, sCustomScale, sQualityLevel);
    // In a real addon, write to a config file here
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
    hooks->RegisterPreferencesPanel(
        hookId,
        "My Addon",            // Panel name (shown in sidebar)
        "Addons/My Addon",     // Category path
        DrawPreferencesPanel,  // Draw callback
        LoadPreferences,       // Load callback (nullable)
        SavePreferences,       // Save callback (nullable)
        nullptr                // User data
    );
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Preferences Panel Addon";
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

The Preferences window sidebar will show:

```
General
External
  Launchers
────────────────
Addons
  My Addon       <-- addon panel
```

Clicking "My Addon" displays the custom settings panel. The Apply/OK buttons trigger `SavePreferences`, and Cancel triggers `LoadPreferences` to revert changes.

## API Reference

### RegisterPreferencesPanel

```cpp
void (*RegisterPreferencesPanel)(
    HookId hookId,
    const char* panelName,
    const char* panelCategory,
    PreferencesPanelDrawCallback drawFunc,
    PreferencesLoadCallback loadFunc,
    PreferencesSaveCallback saveFunc,
    void* userData
);
```

**Parameters:**
- `hookId` - The hook identifier
- `panelName` - Display name in the sidebar
- `panelCategory` - Category path (e.g., `"Addons/My Addon"`)
- `drawFunc` - Called each frame to draw panel content: `void drawFunc(void* userData)`
- `loadFunc` - Called on load/cancel (nullable): `void loadFunc(void* userData)`
- `saveFunc` - Called on apply/save (nullable): `void saveFunc(void* userData)`
- `userData` - Optional user data passed to all callbacks

## Best Practices

1. **Load/Save Callbacks** - Always provide these if your settings need persistence
2. **Cancel = Reload** - The Cancel button calls `loadFunc` to revert unsaved changes, then closes
3. **Apply vs OK** - Apply saves without closing; OK saves and closes
4. **ImGui Widgets** - Use standard ImGui widgets (Checkbox, Slider, Combo, InputText, etc.)
5. **Tooltips** - Add `ImGui::SetTooltip()` on hover for non-obvious settings
6. **Grouping** - Use `ImGui::Separator()` and `ImGui::Spacing()` to organize settings visually
