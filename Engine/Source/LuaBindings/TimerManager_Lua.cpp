#include "LuaBindings/TimerManager_Lua.h"
#include "TimerManager.h"
#include "LuaUtils.h"

#if LUA_ENABLED

int TimerManager_Lua::SetTimer(lua_State* L)
{
    int32_t id = 0;
    CHECK_FUNCTION(L, 1);
    ScriptFunc scriptFunc(L, 1);
    float time = CHECK_NUMBER(L, 2);
    bool loop = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

    id = GetTimerManager()->SetTimer(scriptFunc, time, loop);

    lua_pushinteger(L, id);
    return 1;
}

int TimerManager_Lua::ClearAllTimers(lua_State* L)
{
    GetTimerManager()->ClearAllTimers();
    return 0;
}

int TimerManager_Lua::ClearTimer(lua_State* L)
{
    int32_t id = CHECK_INTEGER(L, 1);

    GetTimerManager()->ClearTimer(id);

    return 0;
}

int TimerManager_Lua::PauseTimer(lua_State* L)
{
    int32_t id = CHECK_INTEGER(L, 1);

    GetTimerManager()->PauseTimer(id);

    return 0;
}

int TimerManager_Lua::ResumeTimer(lua_State* L)
{
    int32_t id = CHECK_INTEGER(L, 1);

    GetTimerManager()->ResumeTimer(id);

    return 0;
}

int TimerManager_Lua::ResetTimer(lua_State* L)
{
    int32_t id = CHECK_INTEGER(L, 1);

    GetTimerManager()->ResetTimer(id);

    return 0;
}

int TimerManager_Lua::GetTimeRemaining(lua_State* L)
{
    int32_t id = CHECK_INTEGER(L, 1);

    float ret = GetTimerManager()->GetTimeRemaining(id);

    lua_pushnumber(L, ret);
    return 1;
}

void TimerManager_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, tableIdx, SetTimer);

    REGISTER_TABLE_FUNC(L, tableIdx, ClearAllTimers);

    REGISTER_TABLE_FUNC(L, tableIdx, ClearTimer);

    REGISTER_TABLE_FUNC(L, tableIdx, PauseTimer);

    REGISTER_TABLE_FUNC(L, tableIdx, ResumeTimer);

    REGISTER_TABLE_FUNC(L, tableIdx, ResetTimer);

    REGISTER_TABLE_FUNC(L, tableIdx, GetTimeRemaining);

    lua_setglobal(L, TIMER_MANAGER_LUA_NAME);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif