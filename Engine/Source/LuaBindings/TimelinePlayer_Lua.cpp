#include "LuaBindings/TimelinePlayer_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/LuaUtils.h"

#include "Assets/Timeline.h"

#if LUA_ENABLED

int TimelinePlayer_Lua::Play(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    node->Play();
    return 0;
}

int TimelinePlayer_Lua::Pause(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    node->Pause();
    return 0;
}

int TimelinePlayer_Lua::Stop(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    node->StopPlayback();
    return 0;
}

int TimelinePlayer_Lua::SetTime(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    float time = CHECK_NUMBER(L, 2);
    node->SetTime(time);
    return 0;
}

int TimelinePlayer_Lua::GetTime(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    lua_pushnumber(L, node->GetTime());
    return 1;
}

int TimelinePlayer_Lua::GetDuration(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    lua_pushnumber(L, node->GetDuration());
    return 1;
}

int TimelinePlayer_Lua::IsPlaying(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    lua_pushboolean(L, node->IsPlaying());
    return 1;
}

int TimelinePlayer_Lua::IsPaused(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    lua_pushboolean(L, node->IsPaused());
    return 1;
}

int TimelinePlayer_Lua::SetTimeline(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    Timeline* timeline = nullptr;

    if (!lua_isnil(L, 2))
    {
        Asset* asset = CHECK_ASSET(L, 2);
        timeline = asset ? asset->As<Timeline>() : nullptr;
    }

    node->SetTimeline(timeline);
    return 0;
}

int TimelinePlayer_Lua::GetTimeline(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    Timeline* timeline = node->GetTimeline();
    Asset_Lua::Create(L, timeline);
    return 1;
}

int TimelinePlayer_Lua::SetPlayOnStart(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    bool playOnStart = CHECK_BOOLEAN(L, 2);
    node->SetPlayOnStart(playOnStart);
    return 0;
}

int TimelinePlayer_Lua::GetPlayOnStart(lua_State* L)
{
    TimelinePlayer* node = CHECK_TIMELINE_PLAYER(L, 1);
    lua_pushboolean(L, node->GetPlayOnStart());
    return 1;
}

void TimelinePlayer_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TIMELINE_PLAYER_LUA_NAME,
        TIMELINE_PLAYER_LUA_FLAG,
        NODE_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Play);
    REGISTER_TABLE_FUNC(L, mtIndex, Pause);
    REGISTER_TABLE_FUNC(L, mtIndex, Stop);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTime);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTime);
    REGISTER_TABLE_FUNC(L, mtIndex, GetDuration);
    REGISTER_TABLE_FUNC(L, mtIndex, IsPlaying);
    REGISTER_TABLE_FUNC(L, mtIndex, IsPaused);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTimeline);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTimeline);
    REGISTER_TABLE_FUNC(L, mtIndex, SetPlayOnStart);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPlayOnStart);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
