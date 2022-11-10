#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"
#include "Clock.h"
#include "Utilities.h"

#include "System/System.h"

#include "LuaBindings/Engine_Lua.h"
#include "LuaBindings/Stream_Lua.h"

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

int Engine_Lua::GetGameDeltaTime(lua_State* L)
{
    float ret = GetEngineState()->mGameDeltaTime;

    lua_pushnumber(L, ret);
    return 1;
}

int Engine_Lua::GetRealDeltaTime(lua_State* L)
{
    float ret = GetEngineState()->mRealDeltaTime;

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

int Engine_Lua::SetBreakOnScriptError(lua_State* L)
{
    bool enableBreak = CHECK_BOOLEAN(L, 1);

    ScriptUtils::SetBreakOnScriptError(enableBreak);

    return 0;
}

int Engine_Lua::IsPlayingInEditor(lua_State* L)
{
    bool ret = ::IsPlayingInEditor();

    lua_pushboolean(L, ret);
    return 1;
}

int Engine_Lua::ReloadAllScripts(lua_State* L)
{
    ::ReloadAllScripts();

    return 0;
}

int Engine_Lua::SetPaused(lua_State* L)
{
    bool value = CHECK_BOOLEAN(L, 1);

    ::SetPaused(value);
    return 0;
}

int Engine_Lua::IsPaused(lua_State* L)
{
    bool ret = ::IsPaused();

    lua_pushboolean(L, ret);
    return 1;
}

int Engine_Lua::FrameStep(lua_State* L)
{
    ::FrameStep();
    return 0;
}

int Engine_Lua::SetTimeDilation(lua_State* L)
{
    float value = CHECK_NUMBER(L, 1);

    ::SetTimeDilation(value);
    return 0;
}

int Engine_Lua::GetTimeDilation(lua_State* L)
{
    float ret = ::GetTimeDilation();

    lua_pushnumber(L, ret);
    return 1;
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

    lua_pushcfunction(L, Engine_Lua::GetGameDeltaTime);
    lua_pushvalue(L, -1);
    lua_setfield(L, tableIdx, "GetGameDeltaTime");
    lua_setfield(L, tableIdx, "GetDeltaTime");

    lua_pushcfunction(L, Engine_Lua::GetRealDeltaTime);
    lua_setfield(L, tableIdx, "GetRealDeltaTime");

    lua_pushcfunction(L, Engine_Lua::GetPlatform);
    lua_setfield(L, tableIdx, "GetPlatform");

    lua_pushcfunction(L, Engine_Lua::IsEditor);
    lua_setfield(L, tableIdx, "IsEditor");

    lua_pushcfunction(L, Engine_Lua::Break);
    lua_setfield(L, tableIdx, "Break");

    lua_pushcfunction(L, Engine_Lua::SetBreakOnScriptError);
    lua_setfield(L, tableIdx, "SetBreakOnScriptError");

    lua_pushcfunction(L, Engine_Lua::IsPlayingInEditor);
    lua_setfield(L, tableIdx, "IsPlayingInEditor");

    lua_pushcfunction(L, Engine_Lua::ReloadAllScripts);
    lua_setfield(L, tableIdx, "ReloadAllScripts");

    lua_pushcfunction(L, Engine_Lua::SetPaused);
    lua_setfield(L, tableIdx, "SetPaused");

    lua_pushcfunction(L, Engine_Lua::IsPaused);
    lua_setfield(L, tableIdx, "IsPaused");

    lua_pushcfunction(L, Engine_Lua::FrameStep);
    lua_setfield(L, tableIdx, "FrameStep");

    lua_pushcfunction(L, Engine_Lua::SetTimeDilation);
    lua_setfield(L, tableIdx, "SetTimeDilation");

    lua_pushcfunction(L, Engine_Lua::GetTimeDilation);
    lua_setfield(L, tableIdx, "GetTimeDilation");

    lua_setglobal(L, "Engine");

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
