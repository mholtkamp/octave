#pragma once

#include "Engine.h"
#include "ObjectRef.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define COMPONENT_REF_LUA_NAME "ComponentRef"
#define CHECK_COMPONENT_REF(L, arg) CheckLuaType<ComponentRef_Lua>(L, arg, COMPONENT_REF_LUA_NAME)->mComponentRef;

struct ComponentRef_Lua
{
    ComponentRef mComponentRef;

    static int Create(lua_State* L);
    static int Destroy(lua_State* L);
    static int Get(lua_State* L);
    static int Set(lua_State* L);

    static void Bind();
};

#endif