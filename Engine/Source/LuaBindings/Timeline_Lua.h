#pragma once

#include "EngineTypes.h"
#include "Log.h"

#include "Assets/Timeline.h"

#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#if LUA_ENABLED

#define TIMELINE_LUA_NAME "Timeline"
#define TIMELINE_LUA_FLAG "cfTimeline"
#define CHECK_TIMELINE(L, arg) CheckAssetLuaType<Timeline>(L, arg, TIMELINE_LUA_NAME, TIMELINE_LUA_FLAG)

struct Timeline_Lua
{
    static int GetDuration(lua_State* L);
    static int IsLooping(lua_State* L);
    static int GetPlayRate(lua_State* L);
    static int GetNumTracks(lua_State* L);

    static void Bind();
};

#endif
