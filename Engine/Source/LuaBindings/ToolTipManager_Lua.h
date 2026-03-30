#pragma once

#include "EngineTypes.h"

#if LUA_ENABLED

#include "LuaBindings/LuaUtils.h"

struct ToolTipManager_Lua
{
    // Configuration
    static int SetShowDelay(lua_State* L);
    static int GetShowDelay(lua_State* L);
    static int SetHideDelay(lua_State* L);
    static int GetHideDelay(lua_State* L);
    static int SetCursorOffset(lua_State* L);
    static int GetCursorOffset(lua_State* L);
    static int SetEnabled(lua_State* L);
    static int IsEnabled(lua_State* L);

    // Manual control
    static int Show(lua_State* L);
    static int Hide(lua_State* L);
    static int GetWidget(lua_State* L);

    // State
    static int GetHoveredWidget(lua_State* L);
    static int IsVisible(lua_State* L);

    // Callbacks
    static int SetOnShowCallback(lua_State* L);
    static int SetOnHideCallback(lua_State* L);

    // Templates
    static int RegisterTemplate(lua_State* L);
    static int UnregisterTemplate(lua_State* L);
    static int GetTemplate(lua_State* L);
    static int HasTemplate(lua_State* L);

    static void Bind();
};

#endif
