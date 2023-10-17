#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SCRIPT_COMPONENT_LUA_NAME "ScriptComponent"
#define SCRIPT_COMPONENT_LUA_FLAG "cfScriptComponent"
#define CHECK_SCRIPT_COMPONENT(L, arg) static_cast<ScriptComponent*>(CheckComponentLuaType(L, arg, SCRIPT_COMPONENT_LUA_NAME, SCRIPT_COMPONENT_LUA_FLAG));

struct ScriptComponent_Lua
{
    static int SetFile(lua_State* L);
    static int GetFile(lua_State* L);
    static int StartScript(lua_State* L);
    static int RestartScript(lua_State* L);
    static int StopScript(lua_State* L);
    static int ReloadScriptFile(lua_State* L);
    static int InvokeNetFunc(lua_State* L);
    static int GetScript(lua_State* L);

    static void Bind();
};

#endif
