#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Widgets/PolyRect.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define POLY_RECT_LUA_NAME "PolyRect"
#define POLY_RECT_LUA_FLAG "cfPolyRect"
#define CHECK_POLY_RECT(L, arg) (PolyRect*) CheckHierarchyLuaType<Widget_Lua>(L, arg, POLY_RECT_LUA_NAME, POLY_RECT_LUA_FLAG)->mWidget;

struct PolyRect_Lua
{
    static int CreateNew(lua_State* L);

    static void Bind();
};

#endif
