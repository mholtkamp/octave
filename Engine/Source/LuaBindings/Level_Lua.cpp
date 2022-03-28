#include "LuaBindings/Level_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

void Level_Lua::Bind()
{
    lua_State* L = GetLua();
    CreateClassMetatable(
        LEVEL_LUA_NAME,
        LEVEL_LUA_FLAG,
        ASSET_LUA_NAME);

    lua_pop(L, 1);
    assert(lua_gettop(L) == 0);
}

#endif
