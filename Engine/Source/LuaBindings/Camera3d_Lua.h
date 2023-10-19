#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Camera3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define CAMERA_3D_LUA_NAME "Camera3D"
#define CAMERA_3D_LUA_FLAG "cfCamera3D"
#define CHECK_CAMERA_3D(L, arg) static_cast<Camera3D*>(CheckNodeLuaType(L, arg, CAMERA_3D_LUA_NAME, CAMERA_3D_LUA_FLAG));

struct Camera3D_Lua
{
    static int SetPerspective(lua_State* L);
    static int SetOrtho(lua_State* L);
    static int IsPerspective(lua_State* L);
    static int IsOrtho(lua_State* L);
    static int GetNear(lua_State* L);
    static int GetFar(lua_State* L);
    static int GetFieldOfView(lua_State* L);
    static int GetAspectRatio(lua_State* L);
    static int GetWidth(lua_State* L);
    static int GetHeight(lua_State* L);
    static int SetNear(lua_State* L);
    static int SetFar(lua_State* L);
    static int SetFieldOfView(lua_State* L);
    static int SetAspectRatio(lua_State* L);
    static int SetWidth(lua_State* L);
    static int SetHeight(lua_State* L);
    static int WorldToScreenPosition(lua_State* L);
    static int ScreenToWorldPosition(lua_State* L);
    static int TraceScreenToWorld(lua_State* L);

    static void Bind();
};

#endif
