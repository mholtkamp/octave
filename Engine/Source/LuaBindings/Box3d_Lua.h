#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Box3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define BOX_COMPONENT_LUA_NAME "Box3D"
#define BOX_COMPONENT_LUA_FLAG "cfBox3D"
#define CHECK_BOX_COMPONENT(L, arg) static_cast<Box3D*>(CheckComponentLuaType(L, arg, BOX_COMPONENT_LUA_NAME, BOX_COMPONENT_LUA_FLAG));

struct Box3D_Lua
{
    static int GetExtents(lua_State* L);
    static int SetExtents(lua_State* L);

    static void Bind();
};

#endif
