#include "Log.h"
#include "Engine.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Log_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int Log_Lua::Debug(lua_State* L)
{
    const char* string = CHECK_STRING(L, 1);

    LogDebug("%s", string);

    return 0;
}

int Log_Lua::Warning(lua_State* L)
{
    const char* string = CHECK_STRING(L, 1);

    LogWarning("%s", string);

    return 0;
}

int Log_Lua::Error(lua_State* L)
{
    const char* string = CHECK_STRING(L, 1);

    LogError("%s", string);

    return 0;
}

int Log_Lua::Console(lua_State* L)
{
    const char* string = CHECK_STRING(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    LogConsole(color, "%s", string);

    return 0;
}

void Log_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, Debug);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, Debug, "Info");

    REGISTER_TABLE_FUNC(L, tableIdx, Warning);

    REGISTER_TABLE_FUNC(L, tableIdx, Error);

    REGISTER_TABLE_FUNC(L, tableIdx, Console);

    lua_setglobal(L, LOG_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
