#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/CapsuleComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define CAPSULE_COMPONENT_LUA_NAME "CapsuleComponent"
#define CAPSULE_COMPONENT_LUA_FLAG "cfCapsuleComponent"
#define CHECK_CAPSULE_COMPONENT(L, arg) static_cast<CapsuleComponent*>(CheckComponentLuaType(L, arg, CAPSULE_COMPONENT_LUA_NAME, CAPSULE_COMPONENT_LUA_FLAG));

struct CapsuleComponent_Lua
{
    static int GetHeight(lua_State* L);
    static int SetHeight(lua_State* L);
    static int GetRadius(lua_State* L);
    static int SetRadius(lua_State* L);

    static void Bind();
};

#endif
