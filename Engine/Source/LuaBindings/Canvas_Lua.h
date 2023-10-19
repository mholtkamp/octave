#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/Canvas.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define CANVAS_LUA_NAME "Canvas"
#define CANVAS_LUA_FLAG "cfCanvas"
#define CHECK_CANVAS(L, arg) (Canvas*)CheckNodeLuaType(L, arg, CANVAS_LUA_NAME, CANVAS_LUA_FLAG);

struct Canvas_Lua
{
    static void Bind();
};

#endif