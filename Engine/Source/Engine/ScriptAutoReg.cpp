#include "ScriptAutoReg.h"

std::vector<AutoRegData> gAutoRegScriptFuncs;

void InitAutoRegScripts()
{
    lua_State* L = GetLua();

    for (AutoRegData& data : gAutoRegScriptFuncs)
    {
        if (data.mTableName != nullptr &&
            data.mTableName != '\0')
        {
            // Does the table already exist?
            lua_getglobal(L, data.mTableName);

            if (lua_isnil(L, -1))
            {
                // No it doesn't, so create it.
                lua_pop(L, 1); // pop nil
                lua_newtable(L);
                lua_setglobal(L, data.mTableName);

                // Make sure the new table is on the stack for the next part
                lua_getglobal(L, data.mTableName);
            }

            assert(lua_istable(L, -1));
            lua_pushcfunction(L, data.mFunc);
            lua_setfield(L, -2, data.mFuncName);

            // Pop the table
            lua_pop(L, 1);
        }
        else
        {
            // If no tablename is specified, then register the func as a global
            lua_pushcfunction(L, data.mFunc);
            lua_setglobal(L, data.mFuncName);
        }

    }

    gAutoRegScriptFuncs.clear();
    gAutoRegScriptFuncs.shrink_to_fit();
}

