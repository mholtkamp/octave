#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Components/BoxComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define BOX_COMPONENT_LUA_NAME "BoxComponent"
#define BOX_COMPONENT_LUA_FLAG "cfBoxComponent"
#define CHECK_BOX_COMPONENT(L, arg) static_cast<BoxComponent*>(CheckComponentLuaType(L, arg, BOX_COMPONENT_LUA_NAME, BOX_COMPONENT_LUA_FLAG));

struct BoxComponent_Lua
{
    static int GetExtents(lua_State* L);
    static int SetExtents(lua_State* L);

    static void Bind();
};

#endif
