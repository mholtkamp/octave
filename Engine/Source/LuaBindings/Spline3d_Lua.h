#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Spline3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SPLINE_3D_LUA_NAME "Spline3D"
#define SPLINE_3D_LUA_FLAG "cfSpline3D"
#define CHECK_SPLINE_3D(L, arg) static_cast<Spline3D*>(CheckNodeLuaType(L, arg, SPLINE_3D_LUA_NAME, SPLINE_3D_LUA_FLAG));

struct Spline3D_Lua
{
    static int AddPoint(lua_State* L);
    static int ClearPoints(lua_State* L);
    static int GetPointCount(lua_State* L);
    static int GetPoint(lua_State* L);
    static int SetPoint(lua_State* L);
    static int GetPositionAt(lua_State* L);
    static int GetTangentAt(lua_State* L);
    static int Play(lua_State* L);
    static int Stop(lua_State* L);

    static void Bind();
};

#endif
