#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"
#include "Clock.h"
#include "Utilities.h"

#include "LuaBindings/Engine_Lua.h"

#if LUA_ENABLED

int Engine_Lua::IsShuttingDown(lua_State* L)
{
    bool ret = ::IsShuttingDown();

    lua_pushboolean(L, ret);
    return 1;
}

int Engine_Lua::GetTime(lua_State* L)
{
    float ret = GetAppClock()->GetTime();

    lua_pushnumber(L, ret);
    return 1;
}

int Engine_Lua::GetDeltaTime(lua_State* L)
{
    float ret = GetAppClock()->DeltaTime();

    lua_pushnumber(L, ret);
    return 1;
}

int Engine_Lua::GetPlatform(lua_State* L)
{
    Platform plat = ::GetPlatform();
    const char* platString = ::GetPlatformString(plat);

    lua_pushstring(L, platString);
    return 1;
}

int Engine_Lua::IsEditor(lua_State* L)
{
#if EDITOR
    lua_pushboolean(L, true);
#else
    lua_pushboolean(L, false);
#endif

    return 1;
}

int Engine_Lua::Break(lua_State* L)
{
#if PLATFORM_WINDOWS && _DEBUG
    DebugBreak();
#endif
    return 0;
}

void Engine_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, Engine_Lua::IsShuttingDown);
    lua_setfield(L, tableIdx, "IsShuttingDown");

    lua_pushcfunction(L, Engine_Lua::GetTime);
    lua_setfield(L, tableIdx, "GetTime");

    lua_pushcfunction(L, Engine_Lua::GetDeltaTime);
    lua_setfield(L, tableIdx, "GetDeltaTime");

    lua_pushcfunction(L, Engine_Lua::GetPlatform);
    lua_setfield(L, tableIdx, "GetPlatform");

    lua_pushcfunction(L, Engine_Lua::IsEditor);
    lua_setfield(L, tableIdx, "IsEditor");

    lua_pushcfunction(L, Engine_Lua::Break);
    lua_setfield(L, tableIdx, "Break");

    lua_setglobal(L, "Engine");

    assert(lua_gettop(L) == 0);
}

#endif
