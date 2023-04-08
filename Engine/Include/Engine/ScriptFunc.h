#pragma once

#include "EngineTypes.h"
#include "Datum.h"

class ScriptComponent;

template<typename T>
struct ScriptableFP
{
    T mFuncPointer = nullptr;
    mutable std::string mScriptTableName;
    mutable std::string mScriptFuncName;
};

class ScriptFunc
{
public:
    ScriptFunc();
    ~ScriptFunc();

    ScriptFunc(lua_State* L, int arg);
    ScriptFunc(const ScriptFunc& src);
    ScriptFunc& operator=(const ScriptFunc& src);

    void Call(uint32_t numParams = 0, Datum* params = nullptr);
    Datum CallR(uint32_t numParams = 0, Datum* params = nullptr);

    //void PushRef() const;

    static void CreateRefTable();

protected:

    void RegisterRef(lua_State* L, int arg);
    void UnregisterRef();
    void CopyRef(int ref);

    int mRef = LUA_REFNIL;
};
