#pragma once

#include "Utilities.h"
#include "LuaBindings/LuaTypeCheck.h"

struct AutoRegData
{
    AutoRegData(const char* tableName, const char* funcName, lua_CFunction func) :
        mTableName(tableName),
        mFuncName(funcName),
        mFunc(func)
    {

    }

    const char* mTableName;
    const char* mFuncName;
    lua_CFunction mFunc;
};

struct ScriptFuncAutoReg
{
    ScriptFuncAutoReg(std::vector<AutoRegData>& autoRegs, const char* tableName, const char* funcName, lua_CFunction func)
    {
        autoRegs.emplace_back(tableName, funcName, func);
    }
};

extern std::vector<AutoRegData> gAutoRegScriptFuncs;

void InitAutoRegScripts();
