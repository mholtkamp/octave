#include "LuaBindings/Level_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

void Level_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex =CreateClassMetatable(
        LEVEL_LUA_NAME,
        LEVEL_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
