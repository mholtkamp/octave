#pragma once

#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/TimelinePlayer.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TIMELINE_PLAYER_LUA_NAME "TimelinePlayer"
#define TIMELINE_PLAYER_LUA_FLAG "cfTimelinePlayer"
#define CHECK_TIMELINE_PLAYER(L, arg) static_cast<TimelinePlayer*>(CheckNodeLuaType(L, arg, TIMELINE_PLAYER_LUA_NAME, TIMELINE_PLAYER_LUA_FLAG));

struct TimelinePlayer_Lua
{
    static int Play(lua_State* L);
    static int Pause(lua_State* L);
    static int Stop(lua_State* L);
    static int SetTime(lua_State* L);
    static int GetTime(lua_State* L);
    static int GetDuration(lua_State* L);
    static int IsPlaying(lua_State* L);
    static int IsPaused(lua_State* L);
    static int SetTimeline(lua_State* L);
    static int GetTimeline(lua_State* L);
    static int SetPlayOnStart(lua_State* L);
    static int GetPlayOnStart(lua_State* L);

    static void Bind();
};

#endif
