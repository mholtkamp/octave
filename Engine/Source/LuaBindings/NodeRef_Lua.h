#pragma once

#include "Engine.h"
#include "ObjectRef.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define NODE_REF_LUA_NAME "NodeRef"
#define CHECK_NODE_REF(L, arg) CheckLuaType<NodeRef_Lua>(L, arg, NODE_REF_LUA_NAME)->mNodeRef;

struct NodeRef_Lua
{
    NodeRef mNodeRef;

    static int Create(lua_State* L);
    static int Destroy(lua_State* L);
    static int Equals(lua_State* L);
    static int Get(lua_State* L);
    static int Set(lua_State* L);

    static void Bind();
};

#endif