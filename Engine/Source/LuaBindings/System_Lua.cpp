#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"
#include "Clock.h"
#include "Utilities.h"

#include "System/System.h"

#include "LuaBindings/System_Lua.h"
#include "LuaBindings/Stream_Lua.h"

#if LUA_ENABLED

int System_Lua::WriteSave(lua_State* L)
{
    const char* saveName = CHECK_STRING(L, 1);
    Stream& stream = CHECK_STREAM(L, 2);

    SYS_WriteSave(saveName, stream);

    return 0;
}

int System_Lua::ReadSave(lua_State* L)
{
    const char* saveName = CHECK_STRING(L, 1);
    Stream& stream = CHECK_STREAM(L, 2);

    SYS_ReadSave(saveName, stream);

    return 0;
}

int System_Lua::DoesSaveExist(lua_State* L)
{
    const char* saveName = CHECK_STRING(L, 1);

    bool ret = SYS_DoesSaveExist(saveName);

    lua_pushboolean(L, ret);
    return 1;
}

int System_Lua::DeleteSave(lua_State* L)
{
    const char* saveName = CHECK_STRING(L, 1);

    SYS_DeleteSave(saveName);

    return 0;
}

int System_Lua::UnmountMemoryCard(lua_State* L)
{
    SYS_UnmountMemoryCard();

    return 0;
}

void System_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, WriteSave);
    lua_setfield(L, tableIdx, "WriteSave");

    lua_pushcfunction(L, ReadSave);
    lua_setfield(L, tableIdx, "ReadSave");

    lua_pushcfunction(L, DoesSaveExist);
    lua_setfield(L, tableIdx, "DoesSaveExist");

    lua_pushcfunction(L, DeleteSave);
    lua_setfield(L, tableIdx, "DeleteSave");

    lua_pushcfunction(L, UnmountMemoryCard);
    lua_setfield(L, tableIdx, "UnmountMemoryCard");

    lua_setglobal(L, "System");

    assert(lua_gettop(L) == 0);
}

#endif
