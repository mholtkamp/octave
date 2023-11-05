#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Quad.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define QUAD_LUA_NAME "Quad"
#define QUAD_LUA_FLAG "cfQuad"
#define CHECK_QUAD(L, arg) (Quad*)CheckNodeLuaType(L, arg, QUAD_LUA_NAME, QUAD_LUA_FLAG);

struct Quad_Lua
{
    static int SetTexture(lua_State* L);
    static int GetTexture(lua_State* L);
    static int SetVertexColors(lua_State* L);
    static int SetUvScale(lua_State* L);
    static int GetUvScale(lua_State* L);
    static int SetUvOffset(lua_State* L);
    static int GetUvOffset(lua_State* L);

    static void Bind();
};

#endif