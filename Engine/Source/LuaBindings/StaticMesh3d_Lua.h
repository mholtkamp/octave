#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/StaticMesh3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define STATIC_MESH_COMPONENT_LUA_NAME "StaticMesh3D"
#define STATIC_MESH_COMPONENT_LUA_FLAG "cfStaticMesh3D"
#define CHECK_STATIC_MESH_COMPONENT(L, arg) static_cast<StaticMesh3D*>(CheckComponentLuaType(L, arg, STATIC_MESH_COMPONENT_LUA_NAME, STATIC_MESH_COMPONENT_LUA_FLAG));

struct StaticMesh3D_Lua
{
    static int SetStaticMesh(lua_State* L);
    static int GetStaticMesh(lua_State* L);

    static int SetUseTriangleCollision(lua_State* L);
    static int GetUseTriangleCollision(lua_State* L);

    static int GetBakeLighting(lua_State* L);

    static void Bind();
};

#endif