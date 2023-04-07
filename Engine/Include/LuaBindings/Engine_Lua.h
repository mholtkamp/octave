#pragma once

#include "Engine.h"
#if LUA_ENABLED

#define ENGINE_LUA_NAME "Engine"

struct Engine_Lua
{
    static int Quit(lua_State* L);
    static int IsShuttingDown(lua_State* L);
    static int GetTime(lua_State* L);
    static int GetGameDeltaTime(lua_State* L);
    static int GetRealDeltaTime(lua_State* L);
    static int GetElapsedTime(lua_State* L);
    static int GetPlatform(lua_State* L);
    static int IsEditor(lua_State* L);
    static int Break(lua_State* L);
    static int SetBreakOnScriptError(lua_State* L);
    static int IsPlayingInEditor(lua_State* L);
    static int IsPlaying(lua_State* L);
    static int ReloadAllScripts(lua_State* L);
    static int SetPaused(lua_State* L);
    static int IsPaused(lua_State* L);
    static int FrameStep(lua_State* L);
    static int SetTimeDilation(lua_State* L);
    static int GetTimeDilation(lua_State* L);

    static void Bind();
};

#endif
