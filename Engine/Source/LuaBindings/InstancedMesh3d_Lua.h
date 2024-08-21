#pragma once
#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/InstancedMesh3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define INSTANCED_MESH_3D_LUA_NAME "InstancedMesh3D"
#define INSTANCED_MESH_3D_LUA_FLAG "cfInstancedMesh3D"
#define CHECK_INSTANCED_MESH_3D(L, arg) static_cast<InstancedMesh3D*>(CheckNodeLuaType(L, arg, INSTANCED_MESH_3D_LUA_NAME, INSTANCED_MESH_3D_LUA_FLAG));

struct InstancedMesh3D_Lua
{
    static int GetNumInstances(lua_State* L);
    static int GetInstanceData(lua_State* L);
    static int SetInstanceData(lua_State* L);
    static int AddInstanceData(lua_State* L);
    static int RemoveInstanceData(lua_State* L);

    static void Bind();
};

#endif
