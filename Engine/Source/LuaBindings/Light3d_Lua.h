#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Light3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define LIGHT_COMPONENT_LUA_NAME "Light3D"
#define LIGHT_COMPONENT_LUA_FLAG "cfLight3D"
#define CHECK_LIGHT_COMPONENT(L, arg) static_cast<Light3D*>(CheckComponentLuaType(L, arg, LIGHT_COMPONENT_LUA_NAME, LIGHT_COMPONENT_LUA_FLAG));

struct Light3D_Lua
{
    static int SetColor(lua_State* L);
    static int GetColor(lua_State* L);
    static int SetIntensity(lua_State* L);
    static int GetIntensity(lua_State* L);
    static int SetCastShadows(lua_State* L);
    static int ShouldCastShadows(lua_State* L);
    static int GetDomain(lua_State* L);

    static void Bind();
};

#endif
