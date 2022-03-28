#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Components/MeshComponent.h"

#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define MESH_COMPONENT_LUA_NAME "MeshComponent"
#define MESH_COMPONENT_LUA_FLAG "cfMeshComponent"
#define CHECK_MESH_COMPONENT(L, arg) static_cast<MeshComponent*>(CheckComponentLuaType(L, arg, MESH_COMPONENT_LUA_NAME, MESH_COMPONENT_LUA_FLAG));

struct MeshComponent_Lua
{
    static int GetMaterial(lua_State* L);
    static int GetMaterialOverride(lua_State* L);
    static int SetMaterialOverride(lua_State* L);

    static int IsBillboard(lua_State* L);
    static int SetBillboard(lua_State* L);

    static void Bind();
};

#endif