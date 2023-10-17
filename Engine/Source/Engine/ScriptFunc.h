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
    ScriptFunc(const Datum& datum);

    bool operator==(const ScriptFunc& other) const;
    bool operator!=(const ScriptFunc& other) const;

    void Call(uint32_t numParams = 0, Datum* params = nullptr) const;
    Datum CallR(uint32_t numParams = 0, Datum* params = nullptr) const;

    void Push(lua_State* L) const;

    bool IsValid() const;

    static void CreateRefTable();

protected:

    void RegisterRef(lua_State* L, int arg);
    void UnregisterRef();
    void CopyRef(int ref);

    int mRef = LUA_REFNIL;
};
