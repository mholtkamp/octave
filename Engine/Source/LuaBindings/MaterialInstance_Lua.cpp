#include "LuaBindings/MaterialInstance_Lua.h"
#include "LuaBindings/Material_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int MaterialInstance_Lua::CreateNew(lua_State* L)
{
    Material* srcMat = nullptr;
    if (!lua_isnone(L, 1)) { srcMat = CHECK_MATERIAL(L, 1); }

    MaterialInstance* ret = MaterialInstance::New(srcMat);

    Asset_Lua::Create(L, ret);
    return 1;
}


void MaterialInstance_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MATERIAL_INSTANCE_LUA_NAME,
        MATERIAL_INSTANCE_LUA_FLAG,
        MATERIAL_LUA_NAME);

    lua_pushcfunction(L, CreateNew);
    lua_setfield(L, mtIndex, "Create");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
