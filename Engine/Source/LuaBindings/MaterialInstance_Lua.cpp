#include "LuaBindings/MaterialInstance_Lua.h"
#include "LuaBindings/MaterialBase_Lua.h"
#include "LuaBindings/Material_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#include "Assets/MaterialBase.h"

#if LUA_ENABLED

int MaterialInstance_Lua::CreateNew(lua_State* L)
{
    Material* srcMat = nullptr;
    if (!lua_isnone(L, 1)) { srcMat = CHECK_MATERIAL(L, 1); }

    MaterialInstance* ret = MaterialInstance::New(srcMat);

    Asset_Lua::Create(L, ret);
    return 1;
}

int MaterialInstance_Lua::GetBaseMaterial(lua_State* L)
{
    MaterialInstance* inst = CHECK_MATERIAL_INSTANCE(L, 1);

    MaterialBase* base = inst->GetBaseMaterial();

    Asset_Lua::Create(L, base);
    return 1;
}

int MaterialInstance_Lua::SetBaseMaterial(lua_State* L)
{
    MaterialInstance* inst = CHECK_MATERIAL_INSTANCE(L, 1);
    MaterialBase* base = CHECK_MATERIAL_BASE(L, 2);

    inst->SetBaseMaterial(base);

    return 0;
}

void MaterialInstance_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MATERIAL_INSTANCE_LUA_NAME,
        MATERIAL_INSTANCE_LUA_FLAG,
        MATERIAL_LUA_NAME);

    REGISTER_TABLE_FUNC_EX(L, mtIndex, CreateNew, "Create");

    REGISTER_TABLE_FUNC(L, mtIndex, GetBaseMaterial);

    REGISTER_TABLE_FUNC(L, mtIndex, SetBaseMaterial);

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
