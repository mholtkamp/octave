#pragma once

#include "EngineTypes.h"

#if LUA_ENABLED

#define TIMER_MANAGER_LUA_NAME "TimerManager"

struct TimerManager_Lua
{
    static int SetTimer(lua_State* L);
    static int ClearAllTimers(lua_State* L);
    static int ClearTimer(lua_State* L);
    static int PauseTimer(lua_State* L);
    static int ResumeTimer(lua_State* L);
    static int ResetTimer(lua_State* L);
    static int GetTimeRemaining(lua_State* L);

    static void Bind();
};

#endif
