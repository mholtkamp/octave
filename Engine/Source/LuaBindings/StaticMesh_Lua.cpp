#include "LuaBindings/StaticMesh_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#include "TableDatum.h"

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

int StaticMesh_Lua::GetVertices(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    int32_t numVerts = (int32_t)mesh->GetNumVertices();
    bool hasColor = mesh->HasVertexColor();
    Vertex* verts = hasColor ? nullptr : mesh->GetVertices();
    VertexColor* cverts = hasColor ? mesh->GetColorVertices() : nullptr;

    lua_newtable(L);
    int vertArrayIdx = lua_gettop(L);

    for (int32_t i = 0; i < numVerts; ++i)
    {
        lua_newtable(L);
        int vertTableIdx = lua_gettop(L);

        Vector_Lua::Create(L, hasColor ? cverts[i].mPosition : verts[i].mPosition);
        lua_setfield(L, vertTableIdx, "position");
        Vector_Lua::Create(L, hasColor ? cverts[i].mTexcoord0 : verts[i].mTexcoord0);
        lua_setfield(L, vertTableIdx, "texcoord0");
        Vector_Lua::Create(L, hasColor ? cverts[i].mTexcoord1 : verts[i].mTexcoord1);
        lua_setfield(L, vertTableIdx, "texcoord1");
        Vector_Lua::Create(L, hasColor ? cverts[i].mNormal : verts[i].mNormal);
        lua_setfield(L, vertTableIdx, "normal");

        if (hasColor)
        {
            lua_pushinteger(L, (int)cverts[i].mColor);
            lua_setfield(L, vertTableIdx, "color");
        }

        lua_seti(L, vertArrayIdx, i + 1);
    }

    // vertArray table should be on top
    return 1;
}

int StaticMesh_Lua::GetIndices(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    IndexType* indices = mesh->GetIndices();
    int32_t numIndices = (int32_t)mesh->GetNumIndices();

    lua_newtable(L);
    int indexArrayIdx = lua_gettop(L);

    for (int32_t i = 0; i < numIndices; ++i)
    {
        lua_pushinteger(L, (int32_t)indices[i]);
        lua_seti(L, indexArrayIdx, i + 1);
    }

    // indexArray table should be on top.
    return 1;
}

int StaticMesh_Lua::HasTriangleMeshCollision(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);

    bool ret = (mesh->GetTriangleCollisionShape() != nullptr);

    lua_pushboolean(L, ret);
    return 1;
}

int StaticMesh_Lua::EnableTriangleMeshCollision(lua_State* L)
{
    StaticMesh* mesh = CHECK_STATIC_MESH(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    mesh->SetGenerateTriangleCollisionMesh(value);

    return 0;
}


void StaticMesh_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        STATIC_MESH_LUA_NAME,
        STATIC_MESH_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetMaterial);

    REGISTER_TABLE_FUNC(L, mtIndex, SetMaterial);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumIndices);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumFaces);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumVertices);

    REGISTER_TABLE_FUNC(L, mtIndex, HasVertexColor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetVertices);

    REGISTER_TABLE_FUNC(L, mtIndex, GetIndices);

    REGISTER_TABLE_FUNC(L, mtIndex, HasTriangleMeshCollision);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableTriangleMeshCollision);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
