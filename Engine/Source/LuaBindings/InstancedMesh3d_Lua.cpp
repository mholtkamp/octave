#include "LuaBindings/InstancedMesh3d_Lua.h"
#include "LuaBindings/StaticMesh3d_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

int InstancedMesh3D_Lua::GetNumInstances(lua_State* L)
{
    InstancedMesh3D* node = CHECK_INSTANCED_MESH_3D(L, 1);

    int32_t ret = node->GetNumInstances();

    lua_pushinteger(L, ret);
    return 1;
}

int InstancedMesh3D_Lua::GetInstanceData(lua_State* L)
{
    InstancedMesh3D* node = CHECK_INSTANCED_MESH_3D(L, 1);
    int32_t index = CHECK_INDEX(L, 2);

    if (index >= 0 && index < int32_t(node->GetNumInstances()))
    {
        const MeshInstanceData& instData = node->GetInstanceData(index);
        Datum dataTable;
        dataTable.SetColorField("position", (glm::vec4(instData.mPosition, 0)));
        dataTable.SetColorField("rotation", (glm::vec4(instData.mRotation, 0)));
        dataTable.SetColorField("scale", (glm::vec4(instData.mScale, 0)));
        LuaPushDatum(L, dataTable);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int InstancedMesh3D_Lua::SetInstanceData(lua_State* L)
{
    InstancedMesh3D* node = CHECK_INSTANCED_MESH_3D(L, 1);
    int32_t index = CHECK_INDEX(L, 2);
    Datum dataTable = LuaObjectToDatum(L, 3);

    if (index >= 0 && index < int32_t(node->GetNumInstances()))
    {
        MeshInstanceData instData;
        instData.mPosition = dataTable.GetColorField("position");
        instData.mRotation = dataTable.GetColorField("rotation");
        instData.mScale = dataTable.GetColorField("scale");

        node->SetInstanceData(index, instData);
    }

    return 0;
}

int InstancedMesh3D_Lua::AddInstanceData(lua_State* L)
{
    InstancedMesh3D* node = CHECK_INSTANCED_MESH_3D(L, 1);
    Datum dataTable = LuaObjectToDatum(L, 2);

    MeshInstanceData instData;
    instData.mPosition = dataTable.GetColorField("position");
    instData.mRotation = dataTable.GetColorField("rotation");
    instData.mScale = dataTable.GetColorField("scale");

    node->AddInstanceData(instData);

    return 0;
}

int InstancedMesh3D_Lua::RemoveInstanceData(lua_State* L)
{
    InstancedMesh3D* node = CHECK_INSTANCED_MESH_3D(L, 1);
    int32_t index = CHECK_INDEX(L, 2);

    node->RemoveInstanceData(index);

    return 0;
}

void InstancedMesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        INSTANCED_MESH_3D_LUA_NAME,
        INSTANCED_MESH_3D_LUA_FLAG,
        STATIC_MESH_3D_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumInstances);
    REGISTER_TABLE_FUNC(L, mtIndex, GetInstanceData);
    REGISTER_TABLE_FUNC(L, mtIndex, SetInstanceData);
    REGISTER_TABLE_FUNC(L, mtIndex, AddInstanceData);
    REGISTER_TABLE_FUNC(L, mtIndex, RemoveInstanceData);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
