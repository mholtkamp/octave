#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"
#include "Utilities.h"
#include "ScriptUtils.h"

#include "LuaBindings/Script_Lua.h"

#if LUA_ENABLED

int Script_Lua::Load(lua_State* L)
{
    const char* filename = CHECK_STRING(L, 1);

    std::string className = ScriptUtils::GetClassNameFromFileName(filename);

    if (!ScriptUtils::IsScriptLoaded(className))
    {
        ScriptUtils::LoadScriptFile(filename, className);
    }

    return 0;
}

int Script_Lua::Run(lua_State* L)
{
    const char* filename = CHECK_STRING(L, 1);

    ScriptUtils::RunScript(filename);

    return 0;
}

int Script_Lua::Inherit(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE); // Derived Table
    luaL_checktype(L, 2, LUA_TTABLE); // Parent Table

    // Set index to same table for both derived and parent
    lua_pushvalue(L, 1);
    lua_setfield(L, 1, "__index");
    lua_pushvalue(L, 2);
    lua_setfield(L, 2, "__index");

    // Assign the parent table as the metatable for the child table
    lua_pushvalue(L, 2); // Not sure if needed
    lua_setmetatable(L, 1);

    return 0;
}

int Script_Lua::New(lua_State* L)
{
    // Create a table that inherits from a parent table.
    luaL_checktype(L, 1, LUA_TTABLE); // Parent Table

    // Create a new table which will be an "instance" of parent type
    lua_newtable(L);
    int newTableIdx = lua_gettop(L);

    lua_pushvalue(L, 1);
    lua_setfield(L, 1, "__index");
    lua_pushvalue(L, newTableIdx);
    lua_setfield(L, newTableIdx, "__index");

    // Assign metatable
    lua_pushvalue(L, 1);
    lua_setmetatable(L, newTableIdx);

    return 1;
}

int Script_Lua::GarbageCollect(lua_State* L)
{
    ScriptUtils::GarbageCollect();

    return 0;
}

int Script_Lua::LoadDirectory(lua_State* L)
{
    const char* dirStr = CHECK_STRING(L, 1);
    bool recurse = true;
    if (!lua_isnone(L, 2)) { recurse = CHECK_BOOLEAN(L, 2); }

    ScriptUtils::LoadScriptDirectory(dirStr, recurse);

    return 0;
}

void Script_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, Script_Lua::Load);
    lua_setfield(L, tableIdx, "Load");
    lua_pushcfunction(L, Script_Lua::Load);
    lua_setfield(L, tableIdx, "Require");

    lua_pushcfunction(L, Script_Lua::Run);
    lua_setfield(L, tableIdx, "Run");

    lua_pushcfunction(L, Script_Lua::Inherit);
    lua_setfield(L, tableIdx, "Inherit");

    lua_pushcfunction(L, Script_Lua::New);
    lua_setfield(L, tableIdx, "New");

    lua_pushcfunction(L, Script_Lua::GarbageCollect);
    lua_setfield(L, tableIdx, "GarbageCollect");

    lua_pushcfunction(L, Script_Lua::LoadDirectory);
    lua_setfield(L, tableIdx, "LoadDirectory");

    lua_setglobal(L, SCRIPT_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
