#pragma once

#include "Utilities.h"
#include "Log.h"

#define DECLARE_SCRIPT_LINK(Type, ParentType, TopType) \
    static bool sRegisteredScriptFuncs_##Type; \
    virtual void RegisterScriptFuncs(lua_State* L) override;

#define DEFINE_SCRIPT_LINK(Type, ParentType, TopType) \
        bool Type::sRegisteredScriptFuncs_##Type = false; \
        static std::vector<AutoRegData> sAutoRegs_##Type; \
        void Type::RegisterScriptFuncs(lua_State* L) { \
            if (Type::sRegisteredScriptFuncs_##Type) { return; } \
            ParentType::RegisterScriptFuncs(L); \
            int mtIndex = CreateClassMetatable(#Type, "cf" #Type, #ParentType); \
            TopType::BindCommonLuaFuncs(L, mtIndex); \
            for (AutoRegData& data : sAutoRegs_##Type) { \
                lua_pushstring(L, data.mFuncName); \
                lua_pushcfunction(L, data.mFunc); \
                lua_rawset(L, mtIndex); \
            } \
            lua_pop(L, 1); \
            sAutoRegs_##Type.clear(); \
            sAutoRegs_##Type.shrink_to_fit(); \
            Type::sRegisteredScriptFuncs_##Type = true; \
        }

#define DECLARE_SCRIPT_LINK_BASE(Base) \
    virtual void RegisterScriptFuncs(lua_State* L); \
    static void BindCommonLuaFuncs(lua_State* L, int mtIndex);

#define DEFINE_SCRIPT_LINK_BASE(Base) \
    void Base::RegisterScriptFuncs(lua_State* L){ } \
    void Base::BindCommonLuaFuncs(lua_State* L, int mtIndex) { Base##_Lua::BindCommon(L, mtIndex); }

#define REGISTER_SCRIPT_FUNCS() \
    lua_State* L = GetLua(); \
    if (L != nullptr) \
    { \
        RegisterScriptFuncs(L); \
    }

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

std::vector<AutoRegData>& GetGlobalAutoRegArray();

void InitAutoRegScripts();
