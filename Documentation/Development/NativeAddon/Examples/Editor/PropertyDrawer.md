# Custom Property Drawers

## Overview

This example demonstrates how to register custom property drawers that override the default rendering of specific property types in the Inspector panel. When a property matches your registered type name, your draw callback is invoked instead of the default property widget.

## Files

### package.json

```json
{
    "name": "Property Drawer Addon",
    "author": "Octave Examples",
    "description": "Custom property drawers for the Inspector.",
    "version": "1.0.0",
    "tags": ["editor", "example"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "propertydraweraddon",
        "apiVersion": 2
    }
}
```

### Source/PropertyDrawerAddon.cpp

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
 * @brief Custom drawer for color properties.
 * @param propertyName The name of the property being drawn.
 * @param propertyOwner Pointer to the object that owns the property.
 * @param propertyType Integer type ID of the property.
 * @param userData User data (unused).
 * @return true if the property was handled, false to fall back to default.
 */
static bool DrawColorProperty(const char* propertyName, void* propertyOwner,
                               int32_t propertyType, void* userData)
{
    // Only handle properties whose name contains "Color"
    if (strstr(propertyName, "Color") == nullptr)
        return false; // Not handled - use default drawer

    ImGui::Text("%s:", propertyName);
    ImGui::SameLine();

    // Draw a color picker button
    static float sColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    char id[128];
    snprintf(id, sizeof(id), "##%s", propertyName);
    ImGui::ColorEdit4(id, sColor, ImGuiColorEditFlags_NoInputs);

    return true; // Handled
}

/**
 * @brief Custom drawer for "Health" named properties - shows a progress bar.
 */
static bool DrawHealthProperty(const char* propertyName, void* propertyOwner,
                                int32_t propertyType, void* userData)
{
    if (strcmp(propertyName, "Health") != 0)
        return false;

    static float sHealth = 0.75f;
    ImGui::Text("Health:");
    ImGui::SameLine();
    ImGui::ProgressBar(sHealth, ImVec2(150.0f, 0.0f));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60.0f);
    ImGui::DragFloat("##HealthVal", &sHealth, 0.01f, 0.0f, 1.0f, "%.0f%%");

    return true;
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
    // Register a drawer that matches by property name content
    hooks->RegisterPropertyDrawer(hookId, "Color", DrawColorProperty, nullptr);
    hooks->RegisterPropertyDrawer(hookId, "Health", DrawHealthProperty, nullptr);
}
#endif

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Property Drawer Addon";
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

### RegisterPropertyDrawer

```cpp
void (*RegisterPropertyDrawer)(HookId hookId, const char* propertyTypeName,
    PropertyDrawCallback drawFunc, void* userData);
```

**Parameters:**
- `hookId` - The hook identifier
- `propertyTypeName` - Type name to match against (matched against the property name string)
- `drawFunc` - Draw callback, returns `true` if handled
- `userData` - Optional user data

### PropertyDrawCallback

```cpp
typedef bool (*PropertyDrawCallback)(const char* propertyName, void* propertyOwner,
    int32_t propertyType, void* userData);
```

**Return value:**
- `true` - Property was drawn by this callback; skip default rendering
- `false` - Property was not handled; fall back to default rendering

## Best Practices

1. **Return false for Unhandled** - If your drawer doesn't recognize the property, return `false`
2. **Unique IDs** - Use `##PropertyName` format for ImGui widget IDs to avoid conflicts
3. **Match Carefully** - The `propertyTypeName` is matched against the property name; be specific
4. **Preserve Layout** - Keep your custom drawer roughly the same height as default drawers
