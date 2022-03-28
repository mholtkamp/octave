#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/SkeletalMesh.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define SKELETAL_MESH_LUA_NAME "SkeletalMesh"
#define SKELETAL_MESH_LUA_FLAG "cfSkeletalMesh"
#define CHECK_SKELETAL_MESH(L, arg) CheckAssetLuaType<SkeletalMesh>(L, arg, SKELETAL_MESH_LUA_NAME, SKELETAL_MESH_LUA_FLAG)

struct SkeletalMesh_Lua
{
    static int GetMaterial(lua_State* L);
    static int SetMaterial(lua_State* L);
    static int GetNumIndices(lua_State* L);
    static int GetNumFaces(lua_State* L);
    static int GetNumVertices(lua_State* L);
    static int FindBoneIndex(lua_State* L);
    static int GetNumBones(lua_State* L);
    static int GetAnimationName(lua_State* L);
    static int GetNumAnimations(lua_State* L);
    static int GetAnimationDuration(lua_State* L);

    static void Bind();
};

#endif
