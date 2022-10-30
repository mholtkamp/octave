#include "LuaBindings/SkeletalMesh_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int SkeletalMesh_Lua::GetMaterial(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);

    Material* ret = mesh->GetMaterial();

    Asset_Lua::Create(L, ret);
    return 1;
}

int SkeletalMesh_Lua::SetMaterial(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);
    Material* material = nullptr;
    if (!lua_isnil(L, 2)) { material = CHECK_MATERIAL(L, 2); }

    mesh->SetMaterial(material);

    return 0;
}

int SkeletalMesh_Lua::GetNumIndices(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);

    uint32_t ret = mesh->GetNumIndices();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SkeletalMesh_Lua::GetNumFaces(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);

    uint32_t ret = mesh->GetNumFaces();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SkeletalMesh_Lua::GetNumVertices(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);

    uint32_t ret = mesh->GetNumVertices();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SkeletalMesh_Lua::FindBoneIndex(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);
    const char* name = CHECK_STRING(L, 2);

    int32_t index = mesh->FindBoneIndex(name);

    lua_pushinteger(L, index);
    return 1;
}

int SkeletalMesh_Lua::GetNumBones(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);

    uint32_t ret = mesh->GetNumBones();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SkeletalMesh_Lua::GetAnimationName(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    const char* ret = nullptr;
    const std::vector<Animation>& animations = mesh->GetAnimations();
    if (index >= 0 &&
        index < int32_t(animations.size()))
    {
        ret = mesh->GetAnimations()[index].mName.c_str();
    }

    lua_pushstring(L, ret);
    return 1;
}

int SkeletalMesh_Lua::GetNumAnimations(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);

    uint32_t ret = (uint32_t) mesh->GetAnimations().size();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int SkeletalMesh_Lua::GetAnimationDuration(lua_State* L)
{
    SkeletalMesh* mesh = CHECK_SKELETAL_MESH(L, 1);
    const char* name = CHECK_STRING(L, 2);

    float ret =mesh->GetAnimationDuration(name);

    lua_pushnumber(L, ret);
    return 1;
}

void SkeletalMesh_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SKELETAL_MESH_LUA_NAME,
        SKELETAL_MESH_LUA_FLAG,
        ASSET_LUA_NAME);

    lua_pushcfunction(L, GetMaterial);
    lua_setfield(L, mtIndex, "GetMaterial");

    lua_pushcfunction(L, SetMaterial);
    lua_setfield(L, mtIndex, "SetMaterial");

    lua_pushcfunction(L, GetNumIndices);
    lua_setfield(L, mtIndex, "GetNumIndices");

    lua_pushcfunction(L, GetNumFaces);
    lua_setfield(L, mtIndex, "GetNumFaces");

    lua_pushcfunction(L, GetNumVertices);
    lua_setfield(L, mtIndex, "GetNumVertices");

    lua_pushcfunction(L, FindBoneIndex);
    lua_setfield(L, mtIndex, "FindBoneIndex");

    lua_pushcfunction(L, GetNumBones);
    lua_setfield(L, mtIndex, "GetNumBones");

    lua_pushcfunction(L, GetAnimationName);
    lua_setfield(L, mtIndex, "GetAnimationName");

    lua_pushcfunction(L, GetNumAnimations);
    lua_setfield(L, mtIndex, "GetNumAnimations");

    lua_pushcfunction(L, GetAnimationDuration);
    lua_setfield(L, mtIndex, "GetAnimationDuration");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
