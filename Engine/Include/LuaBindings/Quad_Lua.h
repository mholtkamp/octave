#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Widgets/Quad.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define QUAD_LUA_NAME "Quad"
#define QUAD_LUA_FLAG "cfQuad"
#define CHECK_QUAD(L, arg) (Quad*) CheckHierarchyLuaType<Widget_Lua>(L, arg, QUAD_LUA_NAME, QUAD_LUA_FLAG)->mWidget;

struct Quad_Lua
{
    static int CreateNew(lua_State* L);

    static int SetTexture(lua_State* L);
    static int GetTexture(lua_State* L);
    static int SetVertexColors(lua_State* L);
    static int SetTint(lua_State* L);
    static int GetTint(lua_State* L);
    static int SetUvScale(lua_State* L);
    static int GetUvScale(lua_State* L);
    static int SetUvOffset(lua_State* L);
    static int GetUvOffset(lua_State* L);

    static void Bind();
};

#endif