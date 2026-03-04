# Example: Custom Script Inspector

A native addon that extends the inspector panel with custom UI when a specific script is attached to a node.

---

## Overview

This example demonstrates:
- Detecting when a specific script is attached to a node
- Drawing custom ImGui UI in the inspector
- Interacting with script properties
- Providing enhanced editing tools for script data

---

## Files

### package.json

```json
{
    "name": "Custom Script Inspector",
    "author": "Octave Examples",
    "description": "Extends the inspector for specific scripts with custom UI.",
    "version": "1.0.0",
    "tags": ["editor", "inspector"],
    "native": {
        "target": "editor",
        "sourceDir": "Source",
        "binaryName": "scriptinspector",
        "apiVersion": 2
    }
}
```

### Source/CustomScriptInspector.cpp

```cpp
/**
 * @file CustomScriptInspector.cpp
 * @brief Custom inspector UI for specific Lua scripts.
 *
 * Include paths available to addons:
 * - Engine/Source - Engine headers (World.h, Node.h, etc.)
 * - Engine/Source/Plugins - Plugin API headers
 * - External/Lua - Lua scripting (lua.h, lauxlib.h)
 * - External/glm - GLM math library
 * - External/Imgui - ImGui (editor builds only)
 * - External/bullet3/src - Physics headers
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

#if EDITOR
#include "Plugins/EditorUIHooks.h"
// ImGui from External/Imgui
#include "imgui.h"
#endif

// GLM from External/glm
#include "glm/glm.hpp"

// Lua from External/Lua (if you need to interact with scripts)
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

static OctaveEngineAPI* sEngineAPI = nullptr;
static uint64_t sHookId = 0;

#if EDITOR

//=============================================================================
// Inspector for "EnemyAI" Script
//=============================================================================

// Custom data we track for the inspector
struct EnemyAIInspectorData
{
    // Cached values for preview
    float previewSpeed = 5.0f;
    float previewRange = 10.0f;
    int previewState = 0;  // 0=Idle, 1=Patrol, 2=Chase, 3=Attack

    // UI state
    bool showAdvanced = false;
    bool showDebug = false;
};

static EnemyAIInspectorData sEnemyAIData;

static void DrawEnemyAIInspector(void* node, void* userData)
{
    // In a real implementation, you'd cast 'node' to the actual node type
    // and access its script component to get/set values

    ImGui::Text("Enemy AI Configuration");
    ImGui::Separator();

    // Basic Settings
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Movement");

    if (ImGui::DragFloat("Move Speed", &sEnemyAIData.previewSpeed, 0.1f, 0.0f, 50.0f, "%.1f units/sec"))
    {
        // Apply change to actual script property
        // script->SetProperty("moveSpeed", sEnemyAIData.previewSpeed);
    }

    if (ImGui::DragFloat("Detection Range", &sEnemyAIData.previewRange, 0.5f, 1.0f, 100.0f, "%.1f units"))
    {
        // script->SetProperty("detectionRange", sEnemyAIData.previewRange);
    }

    ImGui::Spacing();

    // State selector with visual feedback
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Behavior");

    const char* states[] = { "Idle", "Patrol", "Chase", "Attack" };
    ImVec4 stateColors[] = {
        ImVec4(0.5f, 0.5f, 0.5f, 1.0f),  // Idle - Gray
        ImVec4(0.2f, 0.6f, 1.0f, 1.0f),  // Patrol - Blue
        ImVec4(1.0f, 0.8f, 0.0f, 1.0f),  // Chase - Yellow
        ImVec4(1.0f, 0.2f, 0.2f, 1.0f),  // Attack - Red
    };

    // Show current state with color
    ImGui::TextColored(stateColors[sEnemyAIData.previewState],
                       "Current State: %s", states[sEnemyAIData.previewState]);

    if (ImGui::Combo("Default State", &sEnemyAIData.previewState, states, 4))
    {
        // script->SetProperty("defaultState", sEnemyAIData.previewState);
    }

    ImGui::Spacing();

    // Quick action buttons
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Quick Actions");

    if (ImGui::Button("Test Patrol Path", ImVec2(-1, 0)))
    {
        if (sEngineAPI)
        {
            sEngineAPI->LogDebug("Testing patrol path...");
        }
    }

    if (ImGui::Button("Visualize Detection Range", ImVec2(-1, 0)))
    {
        if (sEngineAPI)
        {
            sEngineAPI->LogDebug("Showing detection range gizmo...");
        }
    }

    ImGui::Spacing();

    // Advanced section (collapsible)
    if (ImGui::CollapsingHeader("Advanced Settings"))
    {
        static float aggroDecay = 1.0f;
        static float attackCooldown = 2.0f;
        static bool canFlee = false;

        ImGui::DragFloat("Aggro Decay Rate", &aggroDecay, 0.1f, 0.0f, 10.0f);
        ImGui::DragFloat("Attack Cooldown", &attackCooldown, 0.1f, 0.0f, 10.0f);
        ImGui::Checkbox("Can Flee When Low HP", &canFlee);
    }

    // Debug section (collapsible)
    if (ImGui::CollapsingHeader("Debug Info"))
    {
        ImGui::TextDisabled("Runtime Values (Play Mode Only)");
        ImGui::Text("Current Target: None");
        ImGui::Text("Distance to Target: N/A");
        ImGui::Text("Aggro Level: 0.0");
        ImGui::Text("Last State Change: N/A");

        if (ImGui::Button("Force State: Idle"))
        {
            // script->Call("ForceState", "Idle");
        }
        ImGui::SameLine();
        if (ImGui::Button("Force State: Attack"))
        {
            // script->Call("ForceState", "Attack");
        }
    }
}

//=============================================================================
// Inspector for "Waypoints" Script
//=============================================================================

static void DrawWaypointsInspector(void* node, void* userData)
{
    ImGui::Text("Waypoint Path Editor");
    ImGui::Separator();

    static int waypointCount = 3;
    static bool loopPath = true;
    static float waitTime = 1.0f;

    ImGui::DragInt("Waypoint Count", &waypointCount, 1, 1, 50);
    ImGui::Checkbox("Loop Path", &loopPath);
    ImGui::DragFloat("Wait Time at Each Point", &waitTime, 0.1f, 0.0f, 30.0f, "%.1f sec");

    ImGui::Spacing();

    // Waypoint list
    if (ImGui::CollapsingHeader("Waypoints", ImGuiTreeNodeFlags_DefaultOpen))
    {
        for (int i = 0; i < waypointCount; i++)
        {
            ImGui::PushID(i);

            char label[32];
            snprintf(label, sizeof(label), "Point %d", i + 1);

            if (ImGui::TreeNode(label))
            {
                static float pos[3] = {0, 0, 0};
                ImGui::DragFloat3("Position", pos, 0.1f);

                static float customWait = 1.0f;
                ImGui::DragFloat("Custom Wait", &customWait, 0.1f, 0.0f, 30.0f);

                if (ImGui::Button("Go To"))
                {
                    // Focus editor camera on this waypoint
                }
                ImGui::SameLine();
                if (ImGui::Button("Set From Current"))
                {
                    // Set waypoint to node's current position
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    ImGui::Spacing();

    // Path visualization options
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Visualization");

    static bool showPath = true;
    static bool showArrows = true;
    static ImVec4 pathColor = ImVec4(0.0f, 1.0f, 0.5f, 1.0f);

    ImGui::Checkbox("Show Path in Scene", &showPath);
    ImGui::Checkbox("Show Direction Arrows", &showArrows);
    ImGui::ColorEdit4("Path Color", &pathColor.x);

    if (ImGui::Button("Preview Animation", ImVec2(-1, 0)))
    {
        if (sEngineAPI)
        {
            sEngineAPI->LogDebug("Previewing waypoint animation...");
        }
    }
}

//=============================================================================
// Inspector for "DialogueTrigger" Script
//=============================================================================

static void DrawDialogueTriggerInspector(void* node, void* userData)
{
    ImGui::Text("Dialogue Trigger");
    ImGui::Separator();

    static char dialogueId[128] = "intro_001";
    static bool oneShot = true;
    static bool requiresInteraction = true;
    static float triggerRadius = 2.0f;

    ImGui::InputText("Dialogue ID", dialogueId, sizeof(dialogueId));

    // Show a preview button
    ImGui::SameLine();
    if (ImGui::SmallButton("Preview"))
    {
        if (sEngineAPI)
        {
            sEngineAPI->LogDebug("Previewing dialogue: %s", dialogueId);
        }
    }

    ImGui::Checkbox("One Shot (Triggers Once)", &oneShot);
    ImGui::Checkbox("Requires Player Interaction", &requiresInteraction);
    ImGui::DragFloat("Trigger Radius", &triggerRadius, 0.1f, 0.5f, 20.0f);

    ImGui::Spacing();

    // Dialogue preview section
    if (ImGui::CollapsingHeader("Dialogue Preview"))
    {
        ImGui::TextWrapped("Speaker: Mysterious Figure");
        ImGui::TextWrapped("\"Ah, you've finally arrived. I've been waiting for someone like you...\"");

        ImGui::Spacing();

        ImGui::TextDisabled("Responses:");
        ImGui::BulletText("Who are you?");
        ImGui::BulletText("What do you want?");
        ImGui::BulletText("[Leave]");
    }

    // Conditions section
    if (ImGui::CollapsingHeader("Trigger Conditions"))
    {
        static bool requiresQuest = false;
        static char questId[64] = "";
        static bool requiresItem = false;
        static char itemId[64] = "";

        ImGui::Checkbox("Requires Quest Progress", &requiresQuest);
        if (requiresQuest)
        {
            ImGui::Indent();
            ImGui::InputText("Quest ID", questId, sizeof(questId));
            ImGui::Unindent();
        }

        ImGui::Checkbox("Requires Item", &requiresItem);
        if (requiresItem)
        {
            ImGui::Indent();
            ImGui::InputText("Item ID", itemId, sizeof(itemId));
            ImGui::Unindent();
        }
    }
}

//=============================================================================
// Plugin Callbacks
//=============================================================================

static void RegisterEditorUI(EditorUIHooks* hooks, uint64_t hookId)
{
    sHookId = hookId;

    // Register custom inspectors for specific script types
    // The first parameter is the script name (without .lua extension)

    hooks->RegisterInspector(hookId, "EnemyAI", DrawEnemyAIInspector, nullptr);
    hooks->RegisterInspector(hookId, "Waypoints", DrawWaypointsInspector, nullptr);
    hooks->RegisterInspector(hookId, "DialogueTrigger", DrawDialogueTriggerInspector, nullptr);

    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomScriptInspector: Registered inspectors for EnemyAI, Waypoints, DialogueTrigger");
    }
}

#endif // EDITOR

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    api->LogDebug("CustomScriptInspector addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("CustomScriptInspector addon unloaded.");
    }
    sEngineAPI = nullptr;
}

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Custom Script Inspector";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = nullptr;          // Editor-only addon, no gameplay tick
    desc->TickEditor = nullptr;    // Inspector updates via ImGui draw callback
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

## How It Works

1. **Script Detection**: When a node is selected, the editor checks if any scripts are attached
2. **Inspector Matching**: If a script name matches a registered inspector, the custom draw function is called
3. **Custom UI**: Your draw function receives the node pointer and can display any ImGui UI
4. **Property Binding**: You can read/write script properties through the node's script component

---

## Example Scripts (Lua)

### Scripts/EnemyAI.lua

```lua
-- EnemyAI.lua
-- The custom inspector will appear when this script is attached

-- Exposed properties (shown in default inspector, enhanced by custom inspector)
MoveSpeed = 5.0
DetectionRange = 10.0
DefaultState = 0  -- 0=Idle, 1=Patrol, 2=Chase, 3=Attack
AggroDecayRate = 1.0
AttackCooldown = 2.0
CanFlee = false

local currentState = 0
local currentTarget = nil
local aggroLevel = 0.0

function Start()
    currentState = DefaultState
end

function Tick(deltaTime)
    -- AI logic here
end

function ForceState(newState)
    currentState = newState
    Log.Debug("EnemyAI: Forced to state " .. newState)
end
```

### Scripts/Waypoints.lua

```lua
-- Waypoints.lua
-- The custom inspector provides a visual path editor

WaypointPositions = {}  -- Array of {x, y, z} tables
LoopPath = true
WaitTimeAtEachPoint = 1.0

local currentWaypoint = 1
local waitTimer = 0

function Start()
    -- Initialize waypoints
end

function Tick(deltaTime)
    -- Move along waypoints
end
```

---

## API Reference

### hooks->RegisterInspector(hookId, nodeTypeName, drawFunc, userData)

Registers a custom inspector for a specific node type or script name.

**Parameters:**
- `hookId` (uint64_t): Your plugin's hook ID
- `nodeTypeName` (const char*): Node class name or script name (without .lua)
- `drawFunc` (InspectorDrawCallback): Function to draw the inspector: `void drawFunc(void* node, void* userData)`
- `userData` (void*): Custom data passed to callback

---

### hooks->UnregisterInspector(hookId, nodeTypeName)

Removes a previously registered inspector.

---

## Best Practices

### 1. Provide Visual Feedback

```cpp
// Use colors to indicate state
ImVec4 stateColor = isActive ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
ImGui::TextColored(stateColor, "Status: %s", isActive ? "Active" : "Inactive");
```

### 2. Group Related Properties

```cpp
if (ImGui::CollapsingHeader("Movement", ImGuiTreeNodeFlags_DefaultOpen))
{
    // Movement-related properties
}

if (ImGui::CollapsingHeader("Combat"))
{
    // Combat-related properties
}
```

### 3. Add Quick Actions

```cpp
if (ImGui::Button("Reset to Defaults"))
{
    // Reset all values
}

if (ImGui::Button("Copy Settings"))
{
    // Copy to clipboard
}
```

### 4. Show Runtime Data in Debug Section

```cpp
if (ImGui::CollapsingHeader("Debug (Runtime Only)"))
{
    ImGui::TextDisabled("These values update during play mode");
    // Show live data
}
```

### 5. Validate Input

```cpp
if (ImGui::DragFloat("Speed", &speed, 0.1f, 0.0f, 100.0f))
{
    // Clamp to valid range
    speed = std::max(0.0f, std::min(100.0f, speed));
    ApplyToScript(speed);
}
```
