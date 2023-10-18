#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/PointLight3d.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define POINT_LIGHT_COMPONENT_LUA_NAME "PointLight3D"
#define POINT_LIGHT_COMPONENT_LUA_FLAG "cfPointLightComponent"
#define CHECK_POINT_LIGHT_COMPONENT(L, arg) static_cast<PointLight3D*>(CheckComponentLuaType(L, arg, POINT_LIGHT_COMPONENT_LUA_NAME, POINT_LIGHT_COMPONENT_LUA_FLAG));

struct PointLightComponent_Lua
{
    static int SetRadius(lua_State* L);
    static int GetRadius(lua_State* L);

    static void Bind();
};

#endif
