#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Widgets/Canvas.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define CANVAS_LUA_NAME "Canvas"
#define CANVAS_LUA_FLAG "cfCanvas"
#define CHECK_CANVAS(L, arg) (Canvas*) CheckHierarchyLuaType<Widget_Lua>(L, arg, CANVAS_LUA_NAME, CANVAS_LUA_FLAG)->mWidget;

struct Canvas_Lua
{
    static int CreateNew(lua_State* L);

    static void Bind();
};

#endif