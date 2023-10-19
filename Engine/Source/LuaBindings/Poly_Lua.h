#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Poly.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define POLY_LUA_NAME "Poly"
#define POLY_LUA_FLAG "cfPoly"
#define CHECK_POLY(L, arg) (Poly*) CheckHierarchyLuaType<Widget_Lua>(L, arg, POLY_LUA_NAME, POLY_LUA_FLAG)->mWidget;

struct Poly_Lua
{
    static int CreateNew(lua_State* L);

    static int AddVertex(lua_State* L);
    static int ClearVertices(lua_State* L);
    static int GetNumVertices(lua_State* L);
    static int GetVertex(lua_State* L);

    static int SetTexture(lua_State* L);
    static int GetTexture(lua_State* L);

    static int GetLineWidth(lua_State* L);
    static int SetLineWidth(lua_State* L);

    static void Bind();
};

#endif
