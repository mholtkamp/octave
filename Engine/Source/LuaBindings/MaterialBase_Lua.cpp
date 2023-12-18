#include "LuaBindings/MaterialBase_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

void MaterialBase_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        MATERIAL_BASE_LUA_NAME,
        MATERIAL_BASE_LUA_FLAG,
        MATERIAL_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
