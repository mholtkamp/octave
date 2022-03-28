#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Components/LightComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define LIGHT_COMPONENT_LUA_NAME "LightComponent"
#define LIGHT_COMPONENT_LUA_FLAG "cfLightComponent"
#define CHECK_LIGHT_COMPONENT(L, arg) static_cast<LightComponent*>(CheckComponentLuaType(L, arg, LIGHT_COMPONENT_LUA_NAME, LIGHT_COMPONENT_LUA_FLAG));

struct LightComponent_Lua
{
    static int SetColor(lua_State* L);
    static int GetColor(lua_State* L);
    static int SetIntensity(lua_State* L);
    static int GetIntensity(lua_State* L);
    static int SetCastShadows(lua_State* L);
    static int ShouldCastShadows(lua_State* L);

    static void Bind();
};

#endif
