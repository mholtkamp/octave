#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/StaticMesh.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define STATIC_MESH_LUA_NAME "StaticMesh"
#define STATIC_MESH_LUA_FLAG "cfStaticMesh"
#define CHECK_STATIC_MESH(L, arg) CheckAssetLuaType<StaticMesh>(L, arg, STATIC_MESH_LUA_NAME, STATIC_MESH_LUA_FLAG)

struct StaticMesh_Lua
{
    static int GetMaterial(lua_State* L);
    static int SetMaterial(lua_State* L);
    static int GetNumIndices(lua_State* L);
    static int GetNumFaces(lua_State* L);
    static int GetNumVertices(lua_State* L);
    static int HasVertexColor(lua_State* L);
    static int GetVertices(lua_State* L);
    static int GetIndices(lua_State* L);
    static int HasTriangleMeshCollision(lua_State* L);
    static int EnableTriangleMeshCollision(lua_State* L);

    static void Bind();
};

#endif
