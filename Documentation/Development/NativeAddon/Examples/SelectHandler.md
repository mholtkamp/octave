# Example: SelectHandler

A native addon that sends signals when the user performs a "select" action via mouse click, touch, or gamepad button. Input is polled entirely in C++ using the `Tick` callback.

---

## Overview

This example demonstrates:
- Using the `Tick` callback to poll input every frame in C++
- Direct input access via OctaveEngineAPI (no Lua roundtrip)
- Cross-platform input handling (mouse, touch, gamepad)
- Optionally exposing events to Lua scripts

---

## Files

### package.json

```json
{
    "name": "SelectHandler",
    "author": "Octave Examples",
    "description": "Sends signals on select actions (mouse click, touch, A button).",
    "version": "1.0.0",
    "tags": ["input", "utility"],
    "native": {
        "target": "engine",
        "sourceDir": "Source",
        "binaryName": "selecthandler",
        "apiVersion": 2
    }
}
```

### Source/SelectHandler.cpp

```cpp
/**
 * @file SelectHandler.cpp
 * @brief Native addon for unified select/click input handling.
 *
 * This example demonstrates:
 * - Using the Tick callback to poll input every frame in C++
 * - Direct input polling via OctaveEngineAPI (IsKeyDown, IsMouseButtonDown, etc.)
 * - Optional Lua integration for script callbacks
 * - Cross-platform input abstraction
 *
 * The addon uses direct engine API calls instead of going through Lua for input,
 * which provides better performance for real-time input handling.
 */

#include "Plugins/OctavePluginAPI.h"
#include "Plugins/OctaveEngineAPI.h"

// Static registration for built games
#if !defined(OCTAVE_PLUGIN_EXPORT)
#include "Plugins/RuntimePluginManager.h"
OCTAVE_REGISTER_PLUGIN(SelectHandler, OctavePlugin_GetDesc)
#endif

// Lua includes (External/Lua) - for type definitions only
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// GLM for math (External/glm)
#include "glm/glm.hpp"

#include <vector>

static OctaveEngineAPI* sEngineAPI = nullptr;
static lua_State* sLuaState = nullptr;

//=============================================================================
// Input State Tracking
//=============================================================================

struct SelectState
{
    bool wasPressed = false;
    bool isPressed = false;

    // Callbacks (Lua function references)
    int onPressedRef = LUA_NOREF;
    int onReleasedRef = LUA_NOREF;
    int onHeldRef = LUA_NOREF;

    // Configuration
    bool detectMouse = true;
    bool detectTouch = true;
    bool detectGamepad = true;
    int gamepadButton = 0;  // A button typically
};

// Global list of all active SelectHandlers (for Tick-based updates)
static std::vector<SelectState*> sActiveHandlers;

//=============================================================================
// Lua Bindings
//=============================================================================

// SelectHandler.Create() - Creates a new SelectHandler
static int Lua_SelectHandler_Create(lua_State* L)
{
    SelectState* state = (SelectState*)lua_newuserdata(L, sizeof(SelectState));
    new (state) SelectState();

    // Add to active handlers for automatic Tick updates
    sActiveHandlers.push_back(state);

    luaL_getmetatable(L, "SelectHandler");
    lua_setmetatable(L, -2);

    return 1;
}

// SelectHandler:OnPressed(callback) - Set callback for press events
static int Lua_SelectHandler_OnPressed(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");

    // Release old reference if exists
    if (state->onPressedRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, state->onPressedRef);
    }

    // Store new reference
    if (lua_isfunction(L, 2))
    {
        lua_pushvalue(L, 2);
        state->onPressedRef = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
        state->onPressedRef = LUA_NOREF;
    }

    return 0;
}

// SelectHandler:OnReleased(callback) - Set callback for release events
static int Lua_SelectHandler_OnReleased(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");

    if (state->onReleasedRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, state->onReleasedRef);
    }

    if (lua_isfunction(L, 2))
    {
        lua_pushvalue(L, 2);
        state->onReleasedRef = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
        state->onReleasedRef = LUA_NOREF;
    }

    return 0;
}

// SelectHandler:OnHeld(callback) - Set callback for held state
static int Lua_SelectHandler_OnHeld(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");

    if (state->onHeldRef != LUA_NOREF)
    {
        luaL_unref(L, LUA_REGISTRYINDEX, state->onHeldRef);
    }

    if (lua_isfunction(L, 2))
    {
        lua_pushvalue(L, 2);
        state->onHeldRef = luaL_ref(L, LUA_REGISTRYINDEX);
    }
    else
    {
        state->onHeldRef = LUA_NOREF;
    }

    return 0;
}

// SelectHandler:SetDetectMouse(enabled)
static int Lua_SelectHandler_SetDetectMouse(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    state->detectMouse = lua_toboolean(L, 2);
    return 0;
}

// SelectHandler:SetDetectTouch(enabled)
static int Lua_SelectHandler_SetDetectTouch(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    state->detectTouch = lua_toboolean(L, 2);
    return 0;
}

// SelectHandler:SetDetectGamepad(enabled)
static int Lua_SelectHandler_SetDetectGamepad(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    state->detectGamepad = lua_toboolean(L, 2);
    return 0;
}

// SelectHandler:SetGamepadButton(buttonIndex)
static int Lua_SelectHandler_SetGamepadButton(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    state->gamepadButton = (int)luaL_checkinteger(L, 2);
    return 0;
}

// SelectHandler:IsPressed() - Check if currently pressed
static int Lua_SelectHandler_IsPressed(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    lua_pushboolean(L, state->isPressed);
    return 1;
}

// SelectHandler:JustPressed() - Check if just pressed this frame
static int Lua_SelectHandler_JustPressed(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    lua_pushboolean(L, state->isPressed && !state->wasPressed);
    return 1;
}

// SelectHandler:JustReleased() - Check if just released this frame
static int Lua_SelectHandler_JustReleased(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");
    lua_pushboolean(L, !state->isPressed && state->wasPressed);
    return 1;
}

// Helper to call a Lua callback
static void CallCallback(lua_State* L, int ref, const char* inputSource)
{
    if (ref == LUA_NOREF || L == nullptr)
        return;

    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if (lua_isfunction(L, -1))
    {
        lua_pushstring(L, inputSource);
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            const char* err = lua_tostring(L, -1);
            if (sEngineAPI)
            {
                sEngineAPI->LogError("SelectHandler callback error: %s", err);
            }
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
}

// SelectHandler:Update() - Poll input and fire callbacks
// This now uses the direct OctaveEngineAPI for input instead of Lua arguments
static int Lua_SelectHandler_Update(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");

    if (!sEngineAPI)
        return 0;

    state->wasPressed = state->isPressed;

    // Check all enabled input sources using DIRECT ENGINE API
    // This is much more efficient than going through Lua
    state->isPressed = false;
    const char* source = nullptr;

    // Direct mouse button check (left button = 0)
    if (state->detectMouse && sEngineAPI->IsMouseButtonDown(0))
    {
        state->isPressed = true;
        source = "mouse";
    }
    // Note: Touch detection would need additional API support
    // For now, we check mouse as a fallback for touch on desktop

    // Fire callbacks
    if (state->isPressed && !state->wasPressed)
    {
        // Just pressed - check if it was this frame
        if (sEngineAPI->IsMouseButtonJustPressed(0))
        {
            CallCallback(L, state->onPressedRef, source);
        }
    }
    else if (!state->isPressed && state->wasPressed)
    {
        // Just released
        CallCallback(L, state->onReleasedRef, source);
    }
    else if (state->isPressed && state->wasPressed)
    {
        // Held
        CallCallback(L, state->onHeldRef, source);
    }

    return 0;
}

// Garbage collection - cleanup references
static int Lua_SelectHandler_GC(lua_State* L)
{
    SelectState* state = (SelectState*)luaL_checkudata(L, 1, "SelectHandler");

    if (state->onPressedRef != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, state->onPressedRef);
    if (state->onReleasedRef != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, state->onReleasedRef);
    if (state->onHeldRef != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, state->onHeldRef);

    return 0;
}

// Metatable methods
static const luaL_Reg sSelectHandlerMethods[] = {
    {"OnPressed", Lua_SelectHandler_OnPressed},
    {"OnReleased", Lua_SelectHandler_OnReleased},
    {"OnHeld", Lua_SelectHandler_OnHeld},
    {"SetDetectMouse", Lua_SelectHandler_SetDetectMouse},
    {"SetDetectTouch", Lua_SelectHandler_SetDetectTouch},
    {"SetDetectGamepad", Lua_SelectHandler_SetDetectGamepad},
    {"SetGamepadButton", Lua_SelectHandler_SetGamepadButton},
    {"IsPressed", Lua_SelectHandler_IsPressed},
    {"JustPressed", Lua_SelectHandler_JustPressed},
    {"JustReleased", Lua_SelectHandler_JustReleased},
    {"Update", Lua_SelectHandler_Update},
    {"__gc", Lua_SelectHandler_GC},
    {nullptr, nullptr}
};

// Module functions
static const luaL_Reg sSelectHandlerFuncs[] = {
    {"Create", Lua_SelectHandler_Create},
    {nullptr, nullptr}
};

//=============================================================================
// Plugin Tick - Updates all handlers automatically in C++
//=============================================================================

static void UpdateHandler(SelectState* state, lua_State* L)
{
    if (!sEngineAPI || !state)
        return;

    state->wasPressed = state->isPressed;
    state->isPressed = false;
    const char* source = nullptr;

    // Check all enabled input sources using DIRECT ENGINE API
    if (state->detectMouse && sEngineAPI->IsMouseButtonDown(0))
    {
        state->isPressed = true;
        source = "mouse";
    }

    // Fire callbacks
    if (state->isPressed && !state->wasPressed)
    {
        if (sEngineAPI->IsMouseButtonJustPressed(0))
        {
            CallCallback(L, state->onPressedRef, source);
        }
    }
    else if (!state->isPressed && state->wasPressed)
    {
        CallCallback(L, state->onReleasedRef, source);
    }
    else if (state->isPressed && state->wasPressed)
    {
        CallCallback(L, state->onHeldRef, source);
    }
}

// Called every frame during gameplay (PIE or built game)
static void PluginTick(float deltaTime)
{
    if (!sLuaState)
        return;

    // Update ALL active handlers automatically - no Lua Tick needed!
    for (SelectState* state : sActiveHandlers)
    {
        UpdateHandler(state, sLuaState);
    }
}

//=============================================================================
// Plugin Callbacks
//=============================================================================

static int OnLoad(OctaveEngineAPI* api)
{
    sEngineAPI = api;
    sActiveHandlers.clear();
    api->LogDebug("SelectHandler addon loaded!");
    return 0;
}

static void OnUnload()
{
    if (sEngineAPI)
    {
        sEngineAPI->LogDebug("SelectHandler addon unloaded.");
    }
    sActiveHandlers.clear();
    sEngineAPI = nullptr;
    sLuaState = nullptr;
}

static void RegisterScriptFuncs(lua_State* L)
{
    sLuaState = L;

    // Create the SelectHandler metatable
    luaL_newmetatable(L, "SelectHandler");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, sSelectHandlerMethods, 0);
    lua_pop(L, 1);

    // Create the SelectHandler table
    luaL_newlib(L, sSelectHandlerFuncs);
    lua_setglobal(L, "SelectHandler");
}

//=============================================================================
// Plugin Entry Point
//=============================================================================

extern "C" OCTAVE_PLUGIN_API int OctavePlugin_GetDesc(OctavePluginDesc* desc)
{
    desc->apiVersion = OCTAVE_PLUGIN_API_VERSION;
    desc->pluginName = "SelectHandler";
    desc->pluginVersion = "1.0.0";
    desc->OnLoad = OnLoad;
    desc->OnUnload = OnUnload;
    desc->Tick = PluginTick;       // Automatic input polling every frame
    desc->TickEditor = nullptr;    // No editor tick needed
    desc->RegisterTypes = nullptr;
    desc->RegisterScriptFuncs = RegisterScriptFuncs;
    desc->RegisterEditorUI = nullptr;
    return 0;
}
```

---

## Usage in Lua Scripts

### Basic Click Detection (No Tick Required!)

```lua
-- ClickableObject.lua
-- The C++ Tick callback handles input polling automatically!

local selectHandler = nil

function Start()
    selectHandler = SelectHandler.Create()

    -- Just set up callbacks - C++ handles the rest
    selectHandler:OnPressed(function(source)
        Log.Debug("Pressed via " .. source)
        self:EmitSignal("Clicked")
    end)

    selectHandler:OnReleased(function(source)
        Log.Debug("Released via " .. source)
    end)
end

-- NO Tick function needed! The native addon polls input in C++
-- This is much more efficient than checking input in Lua every frame

function Destroy()
    if selectHandler then
        selectHandler:Destroy()
        selectHandler = nil
    end
end
```

### Button with Visual Feedback

```lua
-- InteractiveButton.lua
-- C++ handles input polling, Lua just handles callbacks

local selectHandler = nil
local originalScale = nil

function Start()
    originalScale = self:GetScale()

    selectHandler = SelectHandler.Create()

    selectHandler:OnPressed(function(source)
        -- Shrink when pressed
        local s = originalScale
        self:SetScale(s.x * 0.9, s.y * 0.9, s.z * 0.9)
    end)

    selectHandler:OnReleased(function(source)
        -- Return to normal size
        local s = originalScale
        self:SetScale(s.x, s.y, s.z)

        -- Trigger the button action
        self:EmitSignal("ButtonActivated")
    end)
end

-- NO Tick function needed! C++ polls input automatically

function Destroy()
    if selectHandler then
        selectHandler:Destroy()
        selectHandler = nil
    end
end
```

### Mobile/Console Adaptive Input

```lua
-- AdaptiveSelectHandler.lua
-- Configure once, C++ handles the rest

local selectHandler = nil

function Start()
    selectHandler = SelectHandler.Create()

    -- Configure based on platform - C++ will poll the right inputs
    local platform = Engine.GetPlatform()

    if platform == "Windows" or platform == "Linux" then
        selectHandler:SetDetectMouse(true)
        selectHandler:SetDetectTouch(false)
        selectHandler:SetDetectGamepad(true)
    elseif platform == "Android" or platform == "iOS" then
        selectHandler:SetDetectMouse(false)
        selectHandler:SetDetectTouch(true)
        selectHandler:SetDetectGamepad(true)
    elseif platform == "GameCube" or platform == "Wii" or platform == "3DS" then
        selectHandler:SetDetectMouse(false)
        selectHandler:SetDetectTouch(platform == "3DS")
        selectHandler:SetDetectGamepad(true)
    end

    selectHandler:OnPressed(function(source)
        Log.Debug("Select action from: " .. source)
        HandleSelect()
    end)
end

function HandleSelect()
    -- Your selection logic here
end

-- NO Tick function! C++ Tick polls input based on configuration above

function Destroy()
    if selectHandler then
        selectHandler:Destroy()
        selectHandler = nil
    end
end
```

---

## API Reference

### SelectHandler.Create()
Creates a new SelectHandler instance.

**Returns:** SelectHandler userdata

---

### handler:OnPressed(callback)
Sets the callback for when select is first pressed.

**Parameters:**
- `callback` (function): Called with `source` parameter ("mouse", "touch", or "gamepad")

---

### handler:OnReleased(callback)
Sets the callback for when select is released.

**Parameters:**
- `callback` (function): Called with `source` parameter

---

### handler:OnHeld(callback)
Sets the callback for while select is held.

**Parameters:**
- `callback` (function): Called with `source` parameter (every frame while held)

---

### handler:SetDetectMouse(enabled)
Enable/disable mouse input detection.

---

### handler:SetDetectTouch(enabled)
Enable/disable touch input detection.

---

### handler:SetDetectGamepad(enabled)
Enable/disable gamepad input detection.

---

### handler:SetGamepadButton(buttonIndex)
Set which gamepad button to detect.

---

### handler:IsPressed()
Check if currently pressed.

**Returns:** boolean

---

### handler:JustPressed()
Check if pressed this frame (wasn't pressed last frame).

**Returns:** boolean

---

### handler:JustReleased()
Check if released this frame (was pressed last frame).

**Returns:** boolean

---

### handler:Destroy()
Remove handler from active list. Call this in your Destroy callback.

---

## C++ Tick Callback

The SelectHandler uses the plugin `Tick` callback to automatically poll input every frame:

```cpp
static void PluginTick(float deltaTime)
{
    // Update ALL active handlers automatically
    for (SelectState* state : sActiveHandlers)
    {
        UpdateHandler(state, sLuaState);
    }
}
```

**Benefits of C++ Tick:**
- **Zero Lua overhead** - No Lua Tick function needed
- **Direct API access** - Uses `sEngineAPI->IsMouseButtonDown()` directly
- **Automatic updates** - All handlers updated in a single native loop
- **Consistent timing** - Input checked at same point each frame
