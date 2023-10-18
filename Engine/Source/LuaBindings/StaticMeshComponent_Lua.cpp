#include "LuaBindings/StaticMeshComponent_Lua.h"
#include "LuaBindings/MeshComponent_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/StaticMesh_Lua.h"

#include "AssetManager.h"

int StaticMeshComponent_Lua::SetStaticMesh(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_COMPONENT(L, 1);
    StaticMesh* mesh = nullptr;

    if (!lua_isnil(L, 2))
    {
        mesh = CHECK_STATIC_MESH(L, 2);
    }

    comp->SetStaticMesh(mesh);

    return 0;
}

int StaticMeshComponent_Lua::GetStaticMesh(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_COMPONENT(L, 1);

    StaticMesh* ret = comp->GetStaticMesh();

    Asset_Lua::Create(L, ret);
    return 1;
}

int StaticMeshComponent_Lua::SetUseTriangleCollision(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetUseTriangleCollision(value);

    return 0;
}

int StaticMeshComponent_Lua::GetUseTriangleCollision(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_COMPONENT(L, 1);

    bool ret = comp->GetUseTriangleCollision();

    lua_pushboolean(L, ret);
    return 1;
}

int StaticMeshComponent_Lua::GetBakeLighting(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_COMPONENT(L, 1);

    bool ret = comp->GetBakeLighting();

    lua_pushboolean(L, ret);
    return 1;
}

void StaticMeshComponent_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        STATIC_MESH_COMPONENT_LUA_NAME,
        STATIC_MESH_COMPONENT_LUA_FLAG,
        MESH_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, SetStaticMesh);
    lua_setfield(L, mtIndex, "SetStaticMesh");

    lua_pushcfunction(L, GetStaticMesh);
    lua_setfield(L, mtIndex, "GetStaticMesh");

    lua_pushcfunction(L, SetUseTriangleCollision);
    lua_setfield(L, mtIndex, "SetUseTriangleCollision");

    lua_pushcfunction(L, GetUseTriangleCollision);
    lua_setfield(L, mtIndex, "GetUseTriangleCollision");

    lua_pushcfunction(L, GetBakeLighting);
    lua_setfield(L, mtIndex, "GetBakeLighting");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);

}
