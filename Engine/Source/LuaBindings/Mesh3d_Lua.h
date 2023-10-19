#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/Mesh3d.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define MESH_3D_LUA_NAME "Mesh3D"
#define MESH_3D_LUA_FLAG "cfMesh3D"
#define CHECK_MESH_3D(L, arg) static_cast<Mesh3D*>(CheckNodeLuaType(L, arg, MESH_3D_LUA_NAME, MESH_3D_LUA_FLAG));

struct Mesh3D_Lua
{
    static int GetMaterial(lua_State* L);
    static int GetMaterialOverride(lua_State* L);
    static int SetMaterialOverride(lua_State* L);
    static int InstantiateMaterial(lua_State* L);

    static int IsBillboard(lua_State* L);
    static int SetBillboard(lua_State* L);

    static void Bind();
};

#endif