#pragma once

#include "Utilities.h"
#include "Log.h"

#define DECLARE_SCRIPT_LINK(Type, ParentType, TopType) \
    virtual void RegisterScriptFuncs(lua_State* L) override;

#define DEFINE_SCRIPT_LINK(Type, ParentType, TopType) \
        static std::vector<AutoRegData> sAutoRegs_##Type; \
        void Type::RegisterScriptFuncs(lua_State* L) { \
            if (AreScriptFuncsRegistered(Type::GetStaticType())) { return; } \
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
            SetScriptFuncsRegistered(Type::GetStaticType()); \
        }

#define DECLARE_SCRIPT_LINK_BASE(Base) \
    virtual void RegisterScriptFuncs(lua_State* L); \
    static void BindCommonLuaFuncs(lua_State* L, int mtIndex); \
    static bool AreScriptFuncsRegistered(TypeId type); \
    static void SetScriptFuncsRegistered(TypeId type); \
    static std::unordered_set<TypeId> sScriptRegisteredSet;

#define DEFINE_SCRIPT_LINK_BASE(Base) \
    std::unordered_set<TypeId> Base::sScriptRegisteredSet; \
    void Base::RegisterScriptFuncs(lua_State* L){ } \
    void Base::BindCommonLuaFuncs(lua_State* L, int mtIndex) { Base##_Lua::BindCommon(L, mtIndex); } \
    bool Base::AreScriptFuncsRegistered(TypeId type) \
    { \
        OCT_ASSERT(type != INVALID_TYPE_ID); \
        bool registered = (sScriptRegisteredSet.find(type) != sScriptRegisteredSet.end()); \
        return registered; \
    } \
    void Base::SetScriptFuncsRegistered(TypeId type) \
    { \
        OCT_ASSERT(sScriptRegisteredSet.find(type) == sScriptRegisteredSet.end()); \
        sScriptRegisteredSet.insert(type); \
    }

#define REGISTER_SCRIPT_FUNCS() \
    lua_State* L = GetLua(); \
    if (L != nullptr) \
    { \
        if (!AreScriptFuncsRegistered(GetType())) \
        { \
            RegisterScriptFuncs(L); \
        } \
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
