#include "LuaBindings/Timeline_Lua.h"
#include "LuaBindings/Asset_Lua.h"

#if LUA_ENABLED

int Timeline_Lua::GetDuration(lua_State* L)
{
    Timeline* timeline = CHECK_TIMELINE(L, 1);
    lua_pushnumber(L, timeline->GetDuration());
    return 1;
}

int Timeline_Lua::IsLooping(lua_State* L)
{
    Timeline* timeline = CHECK_TIMELINE(L, 1);
    lua_pushboolean(L, timeline->IsLooping());
    return 1;
}

int Timeline_Lua::GetPlayRate(lua_State* L)
{
    Timeline* timeline = CHECK_TIMELINE(L, 1);
    lua_pushnumber(L, timeline->GetPlayRate());
    return 1;
}

int Timeline_Lua::GetNumTracks(lua_State* L)
{
    Timeline* timeline = CHECK_TIMELINE(L, 1);
    lua_pushinteger(L, (int)timeline->GetNumTracks());
    return 1;
}

void Timeline_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        TIMELINE_LUA_NAME,
        TIMELINE_LUA_FLAG,
        ASSET_LUA_NAME);

    Asset_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetDuration);
    REGISTER_TABLE_FUNC(L, mtIndex, IsLooping);
    REGISTER_TABLE_FUNC(L, mtIndex, GetPlayRate);
    REGISTER_TABLE_FUNC(L, mtIndex, GetNumTracks);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
