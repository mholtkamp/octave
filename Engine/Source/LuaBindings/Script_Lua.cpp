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
    // TODO
    return 0;
}


void Script_Lua::Bind()
{
    lua_State* L = GetLua();

    lua_newtable(L);
    int tableIdx = lua_gettop(L);

    lua_pushcfunction(L, Script_Lua::Load);
    lua_setfield(L, tableIdx, "Load");

    lua_pushcfunction(L, Script_Lua::Run);
    lua_setfield(L, tableIdx, "Run");

    lua_pushcfunction(L, Script_Lua::Inherit);
    lua_setfield(L, tableIdx, "Inherit");

    lua_setglobal(L, SCRIPT_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
