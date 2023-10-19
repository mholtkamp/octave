#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Nodes/Widgets/PolyRect.h"

#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define POLY_RECT_LUA_NAME "PolyRect"
#define POLY_RECT_LUA_FLAG "cfPolyRect"
#define CHECK_POLY_RECT(L, arg) (PolyRect*)CheckNodeLuaType(L, arg, POLY_RECT_LUA_NAME, POLY_RECT_LUA_FLAG);

struct PolyRect_Lua
{
    static void Bind();
};

#endif
