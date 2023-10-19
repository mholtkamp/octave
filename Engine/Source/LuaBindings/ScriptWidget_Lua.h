#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/ScriptWidget.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SCRIPT_WIDGET_LUA_NAME "ScriptWidget"
#define SCRIPT_WIDGET_LUA_FLAG "cfScriptWidget"
#define CHECK_SCRIPT_WIDGET(L, arg) (ScriptWidget*)CheckNodeLuaType(L, arg, SCRIPT_WIDGET_LUA_NAME, SCRIPT_WIDGET_LUA_FLAG);

struct ScriptWidget_Lua
{
    static int CustomIndex(lua_State* L);
    static int CustomNewIndex(lua_State* L);

    static int GetScript(lua_State* L);
    static int SetFile(lua_State* L);
    static int StartScript(lua_State* L);
    static int RestartScript(lua_State* L);
    static int StopScript(lua_State* L);

    static void Bind();
};

#endif