#include "LuaBindings/StaticMesh3d_Lua.h"
#include "LuaBindings/Mesh3d_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/StaticMesh_Lua.h"

#include "AssetManager.h"

int StaticMesh3D_Lua::SetStaticMesh(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_3D(L, 1);
    StaticMesh* mesh = nullptr;

    if (!lua_isnil(L, 2))
    {
        mesh = CHECK_STATIC_MESH(L, 2);
    }

    comp->SetStaticMesh(mesh);

    return 0;
}

int StaticMesh3D_Lua::GetStaticMesh(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_3D(L, 1);

    StaticMesh* ret = comp->GetStaticMesh();

    Asset_Lua::Create(L, ret);
    return 1;
}

int StaticMesh3D_Lua::SetUseTriangleCollision(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_3D(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetUseTriangleCollision(value);

    return 0;
}

int StaticMesh3D_Lua::GetUseTriangleCollision(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_3D(L, 1);

    bool ret = comp->GetUseTriangleCollision();

    lua_pushboolean(L, ret);
    return 1;
}

int StaticMesh3D_Lua::GetBakeLighting(lua_State* L)
{
    StaticMesh3D* comp = CHECK_STATIC_MESH_3D(L, 1);

    bool ret = comp->GetBakeLighting();

    lua_pushboolean(L, ret);
    return 1;
}

void StaticMesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        STATIC_MESH_3D_LUA_NAME,
        STATIC_MESH_3D_LUA_FLAG,
        MESH_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, SetStaticMesh);

    REGISTER_TABLE_FUNC(L, mtIndex, GetStaticMesh);

    REGISTER_TABLE_FUNC(L, mtIndex, SetUseTriangleCollision);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, SetUseTriangleCollision, "EnableTriangleCollision");

    REGISTER_TABLE_FUNC(L, mtIndex, GetUseTriangleCollision);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GetUseTriangleCollision, "IsTriangleCollisionEnabled");

    REGISTER_TABLE_FUNC(L, mtIndex, GetBakeLighting);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);

}
