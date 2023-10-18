#include "LuaBindings/Mesh3D_Lua.h"
#include "LuaBindings/Primitive3D_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "AssetManager.h"
#include "Assets/Material.h"
#include "Assets/MaterialInstance.h"

#if LUA_ENABLED

int Mesh3D_Lua::GetMaterial(lua_State* L)
{
    Mesh3D* comp = CHECK_MESH_COMPONENT(L, 1);

    Material* mat = comp->GetMaterial();

    Asset_Lua::Create(L, mat);
    return 1;
}

int Mesh3D_Lua::GetMaterialOverride(lua_State* L)
{
    Mesh3D* comp = CHECK_MESH_COMPONENT(L, 1);

    Material* mat = comp->GetMaterialOverride();

    Asset_Lua::Create(L, mat);
    return 1;
}

int Mesh3D_Lua::SetMaterialOverride(lua_State* L)
{
    Mesh3D* comp = CHECK_MESH_COMPONENT(L, 1);
    Material* material = nullptr;
    if (!lua_isnil(L, 2))
    {
        material = CHECK_MATERIAL(L, 2);
    }

    comp->SetMaterialOverride(material);

    return 0;
}

int Mesh3D_Lua::InstantiateMaterial(lua_State* L)
{
    Mesh3D* comp = CHECK_MESH_COMPONENT(L, 1);

    MaterialInstance* matInst = comp->InstantiateMaterial();

    Asset_Lua::Create(L, matInst);
    return 1;
}

int Mesh3D_Lua::IsBillboard(lua_State* L)
{
    Mesh3D* comp = CHECK_MESH_COMPONENT(L, 1);

    bool ret = comp->IsBillboard();

    lua_pushboolean(L, ret);
    return 1;
}

int Mesh3D_Lua::SetBillboard(lua_State* L)
{
    Mesh3D* comp = CHECK_MESH_COMPONENT(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    comp->SetBillboard(value);

    return 0;
}

void Mesh3D_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MESH_COMPONENT_LUA_NAME,
        MESH_COMPONENT_LUA_FLAG,
        PRIMITIVE_COMPONENT_LUA_NAME);

    Component_Lua::BindCommon(L, mtIndex);

    lua_pushcfunction(L, GetMaterial);
    lua_setfield(L, mtIndex, "GetMaterial");

    lua_pushcfunction(L, GetMaterialOverride);
    lua_setfield(L, mtIndex, "GetMaterialOverride");

    lua_pushcfunction(L, SetMaterialOverride);
    lua_setfield(L, mtIndex, "SetMaterialOverride");

    lua_pushcfunction(L, InstantiateMaterial);
    lua_setfield(L, mtIndex, "InstantiateMaterial");

    lua_pushcfunction(L, IsBillboard);
    lua_setfield(L, mtIndex, "IsBillboard");

    lua_pushcfunction(L, SetBillboard);
    lua_setfield(L, mtIndex, "SetBillboard");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif