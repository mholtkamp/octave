#include "LuaBindings/StaticMesh_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int StaticMesh_Lua::GetMaterial(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    Material* ret = mesh->GetMaterial();

    Asset_Lua::Create(L, ret);
    return 1;
}

int StaticMesh_Lua::SetMaterial(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);
    Material* material = nullptr;
    if (!lua_isnil(L, 2)) { material = CHECK_MATERIAL(L, 2); }

    mesh->SetMaterial(material);

    return 0;
}

int StaticMesh_Lua::GetNumIndices(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    uint32_t ret = mesh->GetNumIndices();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int StaticMesh_Lua::GetNumFaces(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    uint32_t ret = mesh->GetNumFaces();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int StaticMesh_Lua::GetNumVertices(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    uint32_t ret = mesh->GetNumVertices();

    lua_pushinteger(L, (int)ret);
    return 1;
}

int StaticMesh_Lua::HasVertexColor(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    bool ret = mesh->HasVertexColor();

    lua_pushboolean(L, ret);
    return 1;
}

int StaticMesh_Lua::HasTriangleMeshCollision(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    bool ret = (mesh->GetTriangleCollisionShape() != nullptr);

    lua_pushboolean(L, ret);
    return 1;
}

void StaticMesh_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        STATIC_MESH_LUA_NAME,
        STATIC_MESH_LUA_FLAG,
        ASSET_LUA_NAME);

    lua_pushcfunction(L, Asset_Lua::Destroy);
    lua_setfield(L, mtIndex, "__gc");

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

    lua_pushcfunction(L, HasVertexColor);
    lua_setfield(L, mtIndex, "HasVertexColor");

    lua_pushcfunction(L, HasTriangleMeshCollision);
    lua_setfield(L, mtIndex, "HasTriangleMeshCollision");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
