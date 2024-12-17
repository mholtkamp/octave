#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"
#include "Clock.h"
#include "Utilities.h"

#include "System/System.h"

#include "LuaBindings/Engine_Lua.h"
#include "LuaBindings/Stream_Lua.h"
#include "LuaBindings/World_Lua.h"

#if LUA_ENABLED

int Engine_Lua::Quit(lua_State* L)
{
    ::Quit();
    return 0;
}

int Engine_Lua::IsShuttingDown(lua_State* L)
{
    bool ret = ::IsShuttingDown();

    lua_pushboolean(L, ret);
    return 1;
}

int Engine_Lua::GetWorld(lua_State* L)
{
    int32_t worldIndex = 0;
    if (!lua_isnone(L, 1)) { worldIndex = CHECK_INDEX(L, 1); }

    World* world = ::GetWorld(worldIndex);

    World_Lua::Create(L, world);
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

int Engine_Lua::GetGameElapsedTime(lua_State* L)
{
    float ret = GetEngineState()->mRealElapsedTime;

    lua_pushnumber(L, ret);
    return 1;
}

int Engine_Lua::GetRealElapsedTime(lua_State* L)
{
    float ret = GetEngineState()->mRealElapsedTime;

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

int Engine_Lua::Alert(lua_State* L)
{
    const char* msg = CHECK_STRING(L, 1);

    SYS_Alert(msg);

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

int Engine_Lua::IsPlaying(lua_State* L)
{
    bool ret = ::IsPlaying();

    lua_pushboolean(L, ret);
    return 1;
}

int Engine_Lua::ReloadAllScripts(lua_State* L)
{
    bool restartScripts = true;
    if (!lua_isnone(L, 1)) { restartScripts = CHECK_BOOLEAN(L, 1); }

    ::ReloadAllScripts(restartScripts);

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

int Engine_Lua::GarbageCollect(lua_State* L)
{
    ::GarbageCollect();

    return 0;
}

void Engine_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, Quit);

    REGISTER_TABLE_FUNC(L, tableIdx, IsShuttingDown);

    REGISTER_TABLE_FUNC(L, tableIdx, GetWorld);

    REGISTER_TABLE_FUNC(L, tableIdx, GetTime);

    REGISTER_TABLE_FUNC(L, tableIdx, GetGameDeltaTime);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, GetGameDeltaTime, "GetDeltaTime");

    REGISTER_TABLE_FUNC(L, tableIdx, GetRealDeltaTime);

    REGISTER_TABLE_FUNC(L, tableIdx, GetGameElapsedTime);
    REGISTER_TABLE_FUNC_EX(L, tableIdx, GetGameElapsedTime, "GetElapsedTime");

    REGISTER_TABLE_FUNC(L, tableIdx, GetRealElapsedTime);

    REGISTER_TABLE_FUNC(L, tableIdx, GetPlatform);

    REGISTER_TABLE_FUNC(L, tableIdx, IsEditor);

    REGISTER_TABLE_FUNC(L, tableIdx, Break);

    REGISTER_TABLE_FUNC(L, tableIdx, Alert);

    REGISTER_TABLE_FUNC(L, tableIdx, SetBreakOnScriptError);

    REGISTER_TABLE_FUNC(L, tableIdx, IsPlayingInEditor);

    REGISTER_TABLE_FUNC(L, tableIdx, IsPlaying);

    REGISTER_TABLE_FUNC(L, tableIdx, ReloadAllScripts);

    REGISTER_TABLE_FUNC(L, tableIdx, SetPaused);

    REGISTER_TABLE_FUNC(L, tableIdx, IsPaused);

    REGISTER_TABLE_FUNC(L, tableIdx, FrameStep);

    REGISTER_TABLE_FUNC(L, tableIdx, SetTimeDilation);

    REGISTER_TABLE_FUNC(L, tableIdx, GetTimeDilation);

    REGISTER_TABLE_FUNC(L, tableIdx, GarbageCollect);

    lua_setglobal(L, "Engine");

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
