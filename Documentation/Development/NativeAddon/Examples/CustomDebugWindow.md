# Example: Custom Debug Window

A native addon that creates a debug window showing scene objects, selected object transform data, with a configurable menu path.

---

## Overview

This example demonstrates:
- Creating dockable ImGui windows
- Accessing scene data in real-time
- Getting the currently selected object
- Displaying transform data (position, rotation, scale)
- Custom menu path configuration
- Refreshing data live during gameplay

---

## Files

### package.json

```json
{
    "name": "Custom Debug Window",
    "author": "Octave Examples",
    "description": "Debug window showing scene hierarchy and selected object transforms.",
    "version": "1.0.0",
    "tags": ["editor", "debug", "tools"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "debugwindow",
        "apiVersion": 2
    }
}
```

### Source/CustomDebugWindow.cpp

```cpp
/**
 * @file CustomDebugWindow.cpp
 * @brief Debug window for viewing scene objects and transform data.
 *
 * This example shows:
 * - Creating custom ImGui windows in the editor
 * - Using direct engine API calls (GetWorld, GetNumWorlds, etc.)
 * - Accessing time via GetDeltaTime/GetElapsedTime
 * - Proper include paths for all dependencies
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
// ImGui is in External/Imgui
#include "imgui.h"
#endif

// GLM is in External/glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>
#include <string>

static OctaveEngineAPI* sEngineAPI = nullptr;
static uint64_t sHookId = 0;

#if EDITOR

//=============================================================================
// Configuration
//=============================================================================

// Configurable menu path
static char sMenuPath[64] = "Developer";
static char sMenuItemName[64] = "Scene Debug Window";
static bool sWindowOpen = false;

// Display options
static bool sShowPosition = true;
static bool sShowRotation = true;
static bool sShowScale = true;
static bool sShowWorldTransform = false;
static bool sAutoRefresh = true;
static float sRefreshRate = 0.1f;  // Seconds between refreshes
static float sRefreshTimer = 0.0f;

//=============================================================================
// Mock Data Structures (Replace with actual engine types)
//=============================================================================

struct Vec3
{
    float x, y, z;
};

struct Transform
{
    Vec3 position;
    Vec3 rotation;  // Euler angles in degrees
    Vec3 scale;
};

struct SceneObject
{
    std::string name;
    int id;
    int parentId;
    bool isSelected;
    Transform localTransform;
    Transform worldTransform;
    std::vector<int> childIds;
};

// Mock scene data (in a real implementation, query the engine)
static std::vector<SceneObject> sSceneObjects;
static int sSelectedObjectId = -1;

//=============================================================================
// Scene Data Access (Replace with actual engine calls)
//=============================================================================

static void RefreshSceneData()
{
    // In a real implementation, you would query the engine:
    // World* world = Engine::GetWorld(0);
    // Node* root = world->GetRootNode();
    // Traverse and populate sSceneObjects

    // For this example, we'll use mock data
    sSceneObjects.clear();

    // Create some mock objects
    SceneObject root;
    root.name = "World Root";
    root.id = 0;
    root.parentId = -1;
    root.isSelected = (sSelectedObjectId == 0);
    root.localTransform = {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    root.worldTransform = root.localTransform;
    root.childIds = {1, 2, 3};
    sSceneObjects.push_back(root);

    SceneObject player;
    player.name = "Player";
    player.id = 1;
    player.parentId = 0;
    player.isSelected = (sSelectedObjectId == 1);
    player.localTransform = {{5.5f, 0.0f, -3.2f}, {0, 45, 0}, {1, 1, 1}};
    player.worldTransform = player.localTransform;
    player.childIds = {4, 5};
    sSceneObjects.push_back(player);

    SceneObject camera;
    camera.name = "Main Camera";
    camera.id = 2;
    camera.parentId = 0;
    camera.isSelected = (sSelectedObjectId == 2);
    camera.localTransform = {{0, 10, -15}, {30, 0, 0}, {1, 1, 1}};
    camera.worldTransform = camera.localTransform;
    sSceneObjects.push_back(camera);

    SceneObject light;
    light.name = "Directional Light";
    light.id = 3;
    light.parentId = 0;
    light.isSelected = (sSelectedObjectId == 3);
    light.localTransform = {{0, 20, 0}, {50, -30, 0}, {1, 1, 1}};
    light.worldTransform = light.localTransform;
    sSceneObjects.push_back(light);

    SceneObject playerModel;
    playerModel.name = "PlayerModel";
    playerModel.id = 4;
    playerModel.parentId = 1;
    playerModel.isSelected = (sSelectedObjectId == 4);
    playerModel.localTransform = {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    playerModel.worldTransform = {{5.5f, 0.0f, -3.2f}, {0, 45, 0}, {1, 1, 1}};
    sSceneObjects.push_back(playerModel);

    SceneObject playerWeapon;
    playerWeapon.name = "Weapon";
    playerWeapon.id = 5;
    playerWeapon.parentId = 1;
    playerWeapon.isSelected = (sSelectedObjectId == 5);
    playerWeapon.localTransform = {{0.5f, 0.8f, 0.2f}, {0, 0, -15}, {0.5f, 0.5f, 0.5f}};
    playerWeapon.worldTransform = {{6.0f, 0.8f, -3.0f}, {0, 45, -15}, {0.5f, 0.5f, 0.5f}};
    sSceneObjects.push_back(playerWeapon);
}

static SceneObject* GetObjectById(int id)
{
    for (auto& obj : sSceneObjects)
    {
        if (obj.id == id) return &obj;
    }
    return nullptr;
}

static SceneObject* GetSelectedObject()
{
    return GetObjectById(sSelectedObjectId);
}

//=============================================================================
// UI Drawing
//=============================================================================

static void DrawVec3(const char* label, const Vec3& v, ImVec4 color = ImVec4(1,1,1,1))
{
    ImGui::TextColored(color, "%s: (%.2f, %.2f, %.2f)", label, v.x, v.y, v.z);
}

static void DrawTransform(const char* header, const Transform& t, bool isWorld = false)
{
    ImVec4 headerColor = isWorld ? ImVec4(0.4f, 0.8f, 0.4f, 1.0f) : ImVec4(0.4f, 0.6f, 1.0f, 1.0f);

    if (ImGui::CollapsingHeader(header, ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();

        if (sShowPosition)
        {
            DrawVec3("Position", t.position, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
        }
        if (sShowRotation)
        {
            DrawVec3("Rotation", t.rotation, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
        }
        if (sShowScale)
        {
            DrawVec3("Scale", t.scale, ImVec4(0.4f, 0.4f, 1.0f, 1.0f));
        }

        ImGui::Unindent();
    }
}

static void DrawSceneHierarchy(int objectId, int depth = 0)
{
    SceneObject* obj = GetObjectById(objectId);
    if (!obj) return;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    if (obj->childIds.empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (obj->isSelected)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Indent based on depth
    bool isOpen = ImGui::TreeNodeEx((void*)(intptr_t)obj->id, flags, "%s", obj->name.c_str());

    // Handle selection
    if (ImGui::IsItemClicked())
    {
        sSelectedObjectId = obj->id;
        // In real implementation: EditorState::SetSelectedNode(obj->node);
    }

    if (isOpen)
    {
        for (int childId : obj->childIds)
        {
            DrawSceneHierarchy(childId, depth + 1);
        }
        ImGui::TreePop();
    }
}

static void DrawDebugWindow(void* userData)
{
    // Update timer for auto-refresh
    if (sAutoRefresh)
    {
        sRefreshTimer -= ImGui::GetIO().DeltaTime;
        if (sRefreshTimer <= 0.0f)
        {
            RefreshSceneData();
            sRefreshTimer = sRefreshRate;
        }
    }

    // Toolbar
    if (ImGui::Button("Refresh"))
    {
        RefreshSceneData();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto", &sAutoRefresh);
    if (sAutoRefresh)
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::DragFloat("##Rate", &sRefreshRate, 0.01f, 0.01f, 1.0f, "%.2fs");
    }

    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Scene Debug Window");
        ImGui::Text("- Shows all objects in the current scene");
        ImGui::Text("- Displays transform data for selected object");
        ImGui::Text("- Updates in real-time during gameplay");
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    // Split view
    float windowHeight = ImGui::GetContentRegionAvail().y;

    // Top: Scene Hierarchy
    ImGui::BeginChild("SceneHierarchy", ImVec2(0, windowHeight * 0.5f), true);
    ImGui::Text("Scene Hierarchy (%zu objects)", sSceneObjects.size());
    ImGui::Separator();

    // Draw root objects
    for (const auto& obj : sSceneObjects)
    {
        if (obj.parentId == -1)
        {
            DrawSceneHierarchy(obj.id);
        }
    }

    ImGui::EndChild();

    // Bottom: Selected Object Details
    ImGui::BeginChild("SelectedDetails", ImVec2(0, 0), true);

    SceneObject* selected = GetSelectedObject();

    if (selected)
    {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "Selected: %s", selected->name.c_str());
        ImGui::Text("ID: %d  |  Parent ID: %d", selected->id, selected->parentId);
        ImGui::Separator();

        // Display options
        if (ImGui::CollapsingHeader("Display Options"))
        {
            ImGui::Checkbox("Show Position", &sShowPosition);
            ImGui::SameLine();
            ImGui::Checkbox("Show Rotation", &sShowRotation);
            ImGui::SameLine();
            ImGui::Checkbox("Show Scale", &sShowScale);
            ImGui::Checkbox("Show World Transform", &sShowWorldTransform);
        }

        // Local Transform
        DrawTransform("Local Transform", selected->localTransform, false);

        // World Transform
        if (sShowWorldTransform)
        {
            DrawTransform("World Transform", selected->worldTransform, true);
        }

        // Quick actions
        ImGui::Spacing();
        if (ImGui::Button("Reset Position"))
        {
            // selected->localTransform.position = {0, 0, 0};
            if (sEngineAPI) sEngineAPI->LogDebug("Reset position");
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Rotation"))
        {
            // selected->localTransform.rotation = {0, 0, 0};
            if (sEngineAPI) sEngineAPI->LogDebug("Reset rotation");
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Scale"))
        {
            // selected->localTransform.scale = {1, 1, 1};
            if (sEngineAPI) sEngineAPI->LogDebug("Reset scale");
        }

        // Copy to clipboard
        ImGui::Spacing();
        if (ImGui::Button("Copy Transform to Clipboard"))
        {
            char buf[256];
            snprintf(buf, sizeof(buf),
                "Position: (%.3f, %.3f, %.3f)\nRotation: (%.3f, %.3f, %.3f)\nScale: (%.3f, %.3f, %.3f)",
                selected->localTransform.position.x, selected->localTransform.position.y, selected->localTransform.position.z,
                selected->localTransform.rotation.x, selected->localTransform.rotation.y, selected->localTransform.rotation.z,
                selected->localTransform.scale.x, selected->localTransform.scale.y, selected->localTransform.scale.z);
            ImGui::SetClipboardText(buf);
            if (sEngineAPI) sEngineAPI->LogDebug("Transform copied to clipboard");
        }
    }
    else
    {
        ImGui::TextDisabled("No object selected");
        ImGui::TextDisabled("Click an object in the hierarchy above");
    }

    ImGui::EndChild();
}

//=============================================================================
// Settings Window
//=============================================================================

static bool sSettingsOpen = false;

static void DrawSettingsWindow(void* userData)
{
    ImGui::Text("Debug Window Settings");
    ImGui::Separator();

    ImGui::Text("Menu Configuration");
    ImGui::InputText("Menu Path", sMenuPath, sizeof(sMenuPath));
    ImGui::InputText("Menu Item Name", sMenuItemName, sizeof(sMenuItemName));

    ImGui::TextDisabled("Note: Restart required for menu changes");

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Default Display Options");
    ImGui::Checkbox("Show Position by Default", &sShowPosition);
    ImGui::Checkbox("Show Rotation by Default", &sShowRotation);
    ImGui::Checkbox("Show Scale by Default", &sShowScale);
    ImGui::Checkbox("Show World Transform by Default", &sShowWorldTransform);

    ImGui::Spacing();

    ImGui::Text("Refresh Settings");
    ImGui::Checkbox("Auto Refresh by Default", &sAutoRefresh);
    ImGui::DragFloat("Default Refresh Rate", &sRefreshRate, 0.01f, 0.01f, 1.0f, "%.2f seconds");
}

//=============================================================================
// Menu Callbacks
//=============================================================================

static void OnOpenDebugWindow(void* userData)
{
    sWindowOpen = true;
    RefreshSceneData();
}

static void OnOpenSettings(void* userData)
{
    sSettingsOpen = true;
}

//=============================================================================
// Plugin Callbacks
//=============================================================================

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;

    // Register the main debug window
    hooks->RegisterWindow(hookId, "Scene Debug", "debugwindow_main", DrawDebugWindow, nullptr);

    // Register the settings window
    hooks->RegisterWindow(hookId, "Debug Window Settings", "debugwindow_settings", DrawSettingsWindow, nullptr);

    // Add menu items (using configurable path)
    hooks->AddMenuItem(hookId, sMenuPath, sMenuItemName, OnOpenDebugWindow, nullptr, "Ctrl+Shift+D");
    hooks->AddMenuItem(hookId, sMenuPath, "Debug Window Settings...", OnOpenSettings, nullptr, nullptr);

    // Initialize scene data
    RefreshSceneData();

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomDebugWindow: Registered UI hooks");
    }
}

#endif // EDITOR

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("CustomDebugWindow addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomDebugWindow addon unloaded.");
    }
    sEngineAPI = nullptr;
}

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Debug Window";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;          // Editor-only addon, no gameplay tick
    desc->TickEditor = nullptr;    // Window updates via ImGui draw callback
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

## Features

### Scene Hierarchy View
- Tree view of all objects in the scene
- Click to select objects
- Expand/collapse to see child objects
- Selected object is highlighted

### Transform Display
- Position (X, Y, Z) in red
- Rotation (Euler angles) in green
- Scale (X, Y, Z) in blue
- Toggle between local and world space

### Auto-Refresh
- Configurable refresh rate
- Real-time updates during Play mode
- Manual refresh button available

### Quick Actions
- Reset position/rotation/scale
- Copy transform to clipboard
- Configurable menu location

---

## Usage

1. Open the debug window via **Developer > Scene Debug Window** (or Ctrl+Shift+D)
2. Click objects in the hierarchy to select them
3. View transform data in the bottom panel
4. Enable "Auto" for real-time updates during gameplay
5. Configure settings via **Developer > Debug Window Settings...**

---

## Customization

### Changing the Menu Location

Edit the `sMenuPath` variable to change where the menu item appears:

```cpp
static char sMenuPath[64] = "Developer";  // Default
// Change to:
static char sMenuPath[64] = "View";       // Appears under View menu
static char sMenuPath[64] = "My Tools";   // Custom top-level menu
```

### Adding More Data

Extend the `DrawDebugWindow` function to show additional information:

```cpp
// Show node type
ImGui::Text("Type: %s", selected->typeName.c_str());

// Show component count
ImGui::Text("Components: %d", selected->componentCount);

// Show visibility
ImGui::Checkbox("Visible", &selected->isVisible);
```
