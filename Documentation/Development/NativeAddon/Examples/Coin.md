# Example: Coin

A native addon that creates a Coin component to continuously rotate Node3D objects with configurable speed and axis. The rotation logic runs entirely in C++ with no Lua overhead per frame.

---

## Overview

This example demonstrates:
- Using the plugin `Tick` callback for frame updates entirely in C++
- Storing Node3D references in native addon components
- Using OctaveEngineAPI to directly manipulate Node3D transforms
- Exposing configurable properties to Lua scripts
- Managing multiple Coin instances from native code

---

## Files

### package.json

```json
{
    "name": "Coin",
    "author": "Octave Examples",
    "description": "Continuously rotates objects with configurable speed and axis.",
    "version": "1.0.0",
    "tags": ["gameplay", "utility"],
    "native": {
        "target": "engine",
        "sourceDir": "Source",
        "binaryName": "Coin",
        "apiVersion": 2
    }
}
```

### Source/Coin.cpp

```cpp
/**
 * @file Coin.cpp
 * @brief Native addon that provides rotation functionality for Node3D objects.
 *
 * This addon demonstrates using the plugin Tick callback to update all
 * Coin instances each frame entirely in C++. Lua only needs to create
 * and configure Coins - no Lua Tick function required.
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

// Include Lua headers for type definitions only (lua_State, luaL_Reg)
// DO NOT call lua_* functions directly - use sEngineAPI->Lua_* instead!
extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

#include <vector>
#include <algorithm>

static OctaveEngineAPI* sEngineAPI = nullptr;

//=============================================================================
// Coin Data Structure
//=============================================================================

struct CoinData
{
    Node3D* targetNode = nullptr;  // The node to rotate
    float speedX = 0.0f;           // Degrees per second on X axis
    float speedY = 45.0f;          // Degrees per second on Y axis (default)
    float speedZ = 0.0f;           // Degrees per second on Z axis
    bool enabled = true;
};

// Global list of all active Coins (managed by the plugin)
static std::vector<CoinData*> sActiveCoins;

//=============================================================================
// Plugin Tick - Called every frame by the engine
//=============================================================================

static void PluginTick(float deltaTime)
{
    // Update all active Coins
    for (CoinData* data : sActiveCoins)
    {
        if (data == nullptr || !data->enabled || data->targetNode == nullptr)
        {
            continue;
        }

        // Calculate rotation delta
        float deltaX = data->speedX * deltaTime;
        float deltaY = data->speedY * deltaTime;
        float deltaZ = data->speedZ * deltaTime;

        // Apply rotation directly to the Node3D using the engine API
        sEngineAPI->Node3D_AddRotation(data->targetNode, deltaX, deltaY, deltaZ);
    }
}

//=============================================================================
// Lua Bindings - Use sEngineAPI->Lua_* wrappers!
//=============================================================================

// Coin.Create(node) - Creates a new Coin attached to a Node3D
static int Lua_Coin_Create(lua_State* L)
{
    // First argument should be a Node3D userdata
    if (!sEngineAPI->Lua_isuserdata(L, 1))
    {
        sEngineAPI->LogError("Coin.Create: expected Node3D as first argument");
        sEngineAPI->Lua_pushnil(L);
        return 1;
    }

    // Get the Node3D pointer from the Lua userdata
    Node3D* node = *(Node3D**)sEngineAPI->Lua_touserdata(L, 1);
    if (node == nullptr)
    {
        sEngineAPI->LogError("Coin.Create: Node3D is null");
        sEngineAPI->Lua_pushnil(L);
        return 1;
    }

    // Create our CoinData userdata
    CoinData* data = (CoinData*)sEngineAPI->Lua_newuserdata(L, sizeof(CoinData));
    new (data) CoinData();  // Placement new to initialize
    data->targetNode = node;

    // Add to active Coins list
    sActiveCoins.push_back(data);

    sEngineAPI->LuaL_getmetatable(L, "Coin");
    sEngineAPI->Lua_setmetatable(L, -2);

    return 1;
}

// Coin:SetSpeed(x, y, z) - Set rotation speed for each axis
static int Lua_Coin_SetSpeed(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    data->speedX = (float)sEngineAPI->LuaL_checknumber(L, 2);
    data->speedY = (float)sEngineAPI->LuaL_checknumber(L, 3);
    data->speedZ = (float)sEngineAPI->LuaL_checknumber(L, 4);
    return 0;
}

// Coin:SetSpeedX(speed)
static int Lua_Coin_SetSpeedX(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    data->speedX = (float)sEngineAPI->LuaL_checknumber(L, 2);
    return 0;
}

// Coin:SetSpeedY(speed)
static int Lua_Coin_SetSpeedY(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    data->speedY = (float)sEngineAPI->LuaL_checknumber(L, 2);
    return 0;
}

// Coin:SetSpeedZ(speed)
static int Lua_Coin_SetSpeedZ(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    data->speedZ = (float)sEngineAPI->LuaL_checknumber(L, 2);
    return 0;
}

// Coin:GetSpeed() - Returns x, y, z rotation speeds
static int Lua_Coin_GetSpeed(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    sEngineAPI->Lua_pushnumber(L, data->speedX);
    sEngineAPI->Lua_pushnumber(L, data->speedY);
    sEngineAPI->Lua_pushnumber(L, data->speedZ);
    return 3;
}

// Coin:SetEnabled(enabled)
static int Lua_Coin_SetEnabled(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    data->enabled = sEngineAPI->Lua_toboolean(L, 2);
    return 0;
}

// Coin:IsEnabled()
static int Lua_Coin_IsEnabled(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");
    sEngineAPI->Lua_pushboolean(L, data->enabled);
    return 1;
}

// Coin:Destroy() - Remove from active list
static int Lua_Coin_Destroy(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->LuaL_checkudata(L, 1, "Coin");

    // Remove from active list
    auto it = std::find(sActiveCoins.begin(), sActiveCoins.end(), data);
    if (it != sActiveCoins.end())
    {
        sActiveCoins.erase(it);
    }

    // Clear the target to prevent updates
    data->targetNode = nullptr;
    data->enabled = false;

    return 0;
}

// Garbage collection - ensure cleanup
static int Lua_Coin_GC(lua_State* L)
{
    CoinData* data = (CoinData*)sEngineAPI->Lua_touserdata(L, 1);
    if (data != nullptr)
    {
        // Remove from active list if still there
        auto it = std::find(sActiveCoins.begin(), sActiveCoins.end(), data);
        if (it != sActiveCoins.end())
        {
            sActiveCoins.erase(it);
        }
    }
    return 0;
}

// Metatable methods
static const luaL_Reg sCoinMethods[] = {
    {"SetSpeed", Lua_Coin_SetSpeed},
    {"SetSpeedX", Lua_Coin_SetSpeedX},
    {"SetSpeedY", Lua_Coin_SetSpeedY},
    {"SetSpeedZ", Lua_Coin_SetSpeedZ},
    {"GetSpeed", Lua_Coin_GetSpeed},
    {"SetEnabled", Lua_Coin_SetEnabled},
    {"IsEnabled", Lua_Coin_IsEnabled},
    {"Destroy", Lua_Coin_Destroy},
    {"__gc", Lua_Coin_GC},
    {nullptr, nullptr}
};

// Module functions
static const luaL_Reg sCoinFuncs[] = {
    {"Create", Lua_Coin_Create},
    {nullptr, nullptr}
};

//=============================================================================
// Plugin Callbacks
//=============================================================================

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    sActiveCoins.clear();
    api->LogDebug("Coin addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("Coin addon unloaded.");
    }
    sActiveCoins.clear();
    sEngineAPI = nullptr;
}

static void RegisterScriptFuncs(lua_State* L)
{
    // Create the Coin metatable
    sEngineAPI->LuaL_newmetatable(L, "Coin");

    // Set __index to itself for method lookup
    sEngineAPI->Lua_pushvalue(L, -1);
    sEngineAPI->Lua_setfield(L, -2, "__index");

    // Register methods (including __gc for cleanup)
    sEngineAPI->LuaL_setfuncs(L, sCoinMethods, 0);
    sEngineAPI->Lua_pop(L, 1);

    // Create the Coin table and register module functions
    sEngineAPI->Lua_createtable(L, 0, 1);
    sEngineAPI->LuaL_setfuncs(L, sCoinFuncs, 0);
    sEngineAPI->Lua_setglobal(L, "Coin");
}

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "Coin";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = PluginTick;       // Gameplay tick (PIE or built game only)
    desc->TickEditor = nullptr;    // Editor tick (nullptr = don't tick in edit mode)
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = RegisterScriptFuncs;
    desc->RegisterEditorUI = nullptr;
    return 0;
}

// For compiled-in builds ONLY (when addon source is included in game executable)
// This is NOT used when building as a DLL for the editor
#if !defined(OCTAVE_PLUGIN_EXPORT)
#include "Plugins/RuntimePluginManager.h"
OCTAVE_REGISTER_PLUGIN(Coin, OctavePlugin_GetDesc)
#endif
```

---

## Usage in Lua Scripts

### Basic Usage

```lua
-- RotatingCube.lua
-- Attach this script to any Node3D

RotatingCube = {}

local Coin = nil

function RotatingCube:Create()
    -- Create a Coin attached to this node
    -- Default: rotates at 45 degrees/sec on Y axis
    -- No Tick function needed - C++ handles everything!
    Coin = Coin.Create(self)
end

function RotatingCube:Destroy()
    -- Clean up when node is destroyed
    if Coin then
        Coin:Destroy()
        Coin = nil
    end
end
```

### Advanced Usage

```lua
-- SpinningPlatform.lua
-- A platform that spins on multiple axes with exposed properties

SpinningPlatform = {}

local Coin = nil

-- Exposed properties (editable in inspector)
SpeedX = 0.0
SpeedY = 90.0
SpeedZ = 0.0
StartEnabled = true

function SpinningPlatform:Create()
    Coin = Coin.Create(self)
    Coin:SetSpeed(SpeedX, SpeedY, SpeedZ)
    Coin:SetEnabled(StartEnabled)
end

function SpinningPlatform:Destroy()
    if Coin then
        Coin:Destroy()
        Coin = nil
    end
end

-- Called from other scripts or events
function SpinningPlatform:SetRotationEnabled(enabled)
    if Coin then
        Coin:SetEnabled(enabled)
    end
end

function SpinningPlatform:SetRotationSpeed(x, y, z)
    if Coin then
        Coin:SetSpeed(x, y, z)
    end
end
```

---

## API Reference

### Coin.Create(node)
Creates a new Coin instance attached to a Node3D.

**Parameters:**
- `node` (Node3D): The node to rotate (typically `self`)

**Returns:** Coin userdata, or nil on error

---

### Coin:SetSpeed(x, y, z)
Sets the rotation speed for all axes.

**Parameters:**
- `x` (number): Degrees per second on X axis
- `y` (number): Degrees per second on Y axis
- `z` (number): Degrees per second on Z axis

---

### Coin:SetSpeedX(speed) / SetSpeedY(speed) / SetSpeedZ(speed)
Sets the rotation speed for a single axis.

**Parameters:**
- `speed` (number): Degrees per second

---

### Coin:GetSpeed()
Gets the current rotation speeds.

**Returns:** x, y, z (numbers)

---

### Coin:SetEnabled(enabled)
Enables or disables the rotation.

**Parameters:**
- `enabled` (boolean): Whether rotation is active

---

### Coin:IsEnabled()
Checks if rotation is enabled.

**Returns:** boolean

---

### Coin:Destroy()
Removes the Coin from the update list. Call this in your Destroy callback.

---

## Tick Callbacks

Native addons have two tick callbacks:

| Callback | When Called | Use Case |
|----------|-------------|----------|
| `Tick` | During gameplay only (PIE or built game) | Gameplay logic like rotation, movement, AI |
| `TickEditor` | Every frame in editor (regardless of play state) | Editor tools, visualizations, gizmos |

This Coin uses `Tick` so objects only rotate during gameplay, not while editing.

---

## Key Features

| Feature | Description |
|---------|-------------|
| **Zero Lua overhead** | No Tick function needed in Lua - C++ handles all updates |
| **Automatic cleanup** | `__gc` metamethod ensures cleanup when Lua garbage collects |
| **Direct Node3D access** | Uses `Node3D_AddRotation` for maximum performance |
| **Multiple instances** | Plugin manages all Coins in a single tick loop |
| **Gameplay-only rotation** | Uses `Tick` callback so objects don't rotate while editing |

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     Engine Main Loop                      │
│                                                           │
│  ┌─────────────┐    ┌─────────────┐    ┌──────────────┐ │
│  │   Update()  │───▶│PluginTick() │───▶│ Next Frame   │ │
│  └─────────────┘    └──────┬──────┘    └──────────────┘ │
│                            │                              │
│                            ▼                              │
│              ┌─────────────────────────┐                 │
│              │  For each CoinData:  │                 │
│              │  - Calculate delta      │                 │
│              │  - Node3D_AddRotation() │                 │
│              └─────────────────────────┘                 │
└─────────────────────────────────────────────────────────┘
```

---

## Important Notes

**Lifecycle Management:** Always call `Coin:Destroy()` in your Lua script's `Destroy()` callback, or the Coin may continue trying to update a destroyed node.

**Garbage Collection:** The `__gc` metamethod provides automatic cleanup when Lua garbage collects the Coin, but explicit `Destroy()` is recommended for deterministic cleanup.

**Node Validity:** The plugin stores a raw pointer to the Node3D. If the node is destroyed before the Coin, ensure you call `Coin:Destroy()` first.

---

## How It Works in Built Games

When you build your game, the native addon source files are compiled directly into the game executable (not as a DLL). The plugin uses the `OCTAVE_REGISTER_PLUGIN` macro for automatic registration:

```cpp
// At the end of the plugin source file (ONLY for compiled-in builds):
#if !defined(OCTAVE_PLUGIN_EXPORT)
#include "Plugins/RuntimePluginManager.h"
OCTAVE_REGISTER_PLUGIN(Coin, OctavePlugin_GetDesc)
#endif
```

**Important:** The `#if !defined(OCTAVE_PLUGIN_EXPORT)` guard ensures this code is only compiled when building directly into the game. When building as a DLL for the editor (which defines `OCTAVE_PLUGIN_EXPORT`), the macro is skipped because the editor uses dynamic loading via `OctavePlugin_GetDesc` instead.

This macro creates a static initializer that registers the plugin with the `RuntimePluginManager` when the game starts. The registration flow is:

1. **Static initialization** - `OCTAVE_REGISTER_PLUGIN` queues the plugin descriptor
2. **Engine Initialize()** - `RuntimePluginManager::Create()` processes queued plugins
3. **RuntimePluginManager::Initialize()** - Calls `OnLoad` and `RegisterScriptFuncs` for each plugin
4. **Every frame** - `RuntimePluginManager::TickAllPlugins()` calls each plugin's `Tick` callback
5. **Shutdown** - `RuntimePluginManager::Destroy()` calls `OnUnload` for each plugin

Both the editor (via `NativeAddonManager`) and built games (via `RuntimePluginManager`) use the same plugin code, ensuring consistent behavior between development and release.
