#include "ScriptAutoReg.h"

std::vector<AutoRegData>& GetGlobalAutoRegArray()
{
    static std::vector<AutoRegData> sAutoRegScriptFuncs;
    return sAutoRegScriptFuncs;
}

void InitAutoRegScripts()
{
    lua_State* L = GetLua();

    std::vector<AutoRegData>& autoRegArray = GetGlobalAutoRegArray();

    for (AutoRegData& data : autoRegArray)
    {
        if (data.mTableName != nullptr &&
            data.mTableName[0] != '\0')
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

    autoRegArray.clear();
    autoRegArray.shrink_to_fit();
}

