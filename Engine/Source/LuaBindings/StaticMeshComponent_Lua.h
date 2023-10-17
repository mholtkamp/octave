#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/StaticMeshComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define STATIC_MESH_COMPONENT_LUA_NAME "StaticMeshComponent"
#define STATIC_MESH_COMPONENT_LUA_FLAG "cfStaticMeshComponent"
#define CHECK_STATIC_MESH_COMPONENT(L, arg) static_cast<StaticMeshComponent*>(CheckComponentLuaType(L, arg, STATIC_MESH_COMPONENT_LUA_NAME, STATIC_MESH_COMPONENT_LUA_FLAG));

struct StaticMeshComponent_Lua
{
    static int SetStaticMesh(lua_State* L);
    static int GetStaticMesh(lua_State* L);

    static int SetUseTriangleCollision(lua_State* L);
    static int GetUseTriangleCollision(lua_State* L);

    static int GetBakeLighting(lua_State* L);

    static void Bind();
};

#endif