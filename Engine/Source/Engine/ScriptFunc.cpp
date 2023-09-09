#include "ScriptFunc.h"
#include "EngineTypes.h"
#include "ScriptUtils.h"

#define REF_TABLE_NAME "OctaveFunc"

ScriptFunc::ScriptFunc()
{

}

ScriptFunc::~ScriptFunc()
{
    UnregisterRef();
}

ScriptFunc::ScriptFunc(lua_State* L, int arg)
{
    RegisterRef(L, arg);
}

ScriptFunc::ScriptFunc(const ScriptFunc& src)
{
    CopyRef(src.mRef);
}

ScriptFunc& ScriptFunc::operator=(const ScriptFunc& src)
{
    if (this != &src)
    {
        CopyRef(src.mRef);
    }

    return *this;
}

ScriptFunc::ScriptFunc(const Datum& datum)
{
    *this = datum.GetFunction();
}


bool ScriptFunc::operator==(const ScriptFunc& other) const
{
    bool equal = false;

    if (mRef == LUA_REFNIL || other.mRef == LUA_REFNIL)
    {
        // If either ref is nil, then they are only equal if both are nil
        equal = (mRef == LUA_REFNIL && other.mRef == LUA_REFNIL);
    }
    else
    {
        // Otherwise we have to check if these keys reference the same object.
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
            OCT_ASSERT(lua_istable(L, -1));

            int refTable = lua_gettop(L);
            lua_geti(L, refTable, mRef);
            lua_geti(L, refTable, other.mRef);

            equal = (lua_rawequal(L, -1, -2) == 1);
            lua_pop(L, 3);
        }
    }

    return equal;
}

bool ScriptFunc::operator!=(const ScriptFunc& other) const
{
    return !operator==(other);
}

void ScriptFunc::Call(uint32_t numParams, Datum* params) const
{
    lua_State* L = GetLua();
    if (L != nullptr && 
        mRef != LUA_REFNIL)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
        OCT_ASSERT(lua_istable(L, -1));

        // Push function
        lua_geti(L, -1, mRef);

        // Push params
        OCT_ASSERT(numParams == 0 || params != nullptr);
        for (uint32_t i = 0; i < numParams; ++i)
        {
            LuaPushDatum(L, params[i]);
        }

        ScriptUtils::CallLuaFunc(numParams, 0);
    }
}

Datum ScriptFunc::CallR(uint32_t numParams, Datum* params) const
{
    Datum retDatum;

    lua_State* L = GetLua();
    if (L != nullptr && 
        mRef != LUA_REFNIL)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
        OCT_ASSERT(lua_istable(L, -1));

        // Push function
        lua_geti(L, -1, mRef);

        // Push params
        OCT_ASSERT(numParams == 0 || params != nullptr);
        for (uint32_t i = 0; i < numParams; ++i)
        {
            LuaPushDatum(L, params[i]);
        }

        bool success = ScriptUtils::CallLuaFunc(numParams, 1);
        
        if (success)
        {
            LuaObjectToDatum(L, -1, retDatum);
            lua_pop(L, 1);
        }
    }

    return retDatum;
}

void ScriptFunc::Push(lua_State* L) const
{
    // It's very important that this function pushes ONLY the function,
    // not the reference table also.
    if (mRef == LUA_REFNIL)
    {
        lua_pushnil(L);
    }
    else
    {
        lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);  // Stack = +1
        OCT_ASSERT(lua_istable(L, -1));

        lua_geti(L, -1, mRef); // Stack = +2

        int tempRef = luaL_ref(L, LUA_REGISTRYINDEX); // Stack = +1

        // Pop the ref table
        lua_pop(L, 1); // Stack = 0

        // Push the single function
        lua_rawgeti(L, LUA_REGISTRYINDEX, tempRef); // Stack = +1

        // Unref the temp ref
        luaL_unref(L, LUA_REGISTRYINDEX, tempRef);
    }
}

bool ScriptFunc::IsValid() const
{
    return (mRef != LUA_REFNIL);
}

void ScriptFunc::RegisterRef(lua_State* L, int arg)
{
    OCT_ASSERT(mRef == LUA_REFNIL);
    OCT_ASSERT(lua_isfunction(L, arg));

    // Convert arg to positive index
    lua_pushvalue(L, arg);
    int funcIdx = lua_gettop(L);

    lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    int refTableIdx = lua_gettop(L);

    // Push the function on to the top of stack
    lua_pushvalue(L, funcIdx);

    mRef = luaL_ref(L, refTableIdx);

    // Pop ref table and first value copy
    lua_pop(L, 2);
}

void ScriptFunc::UnregisterRef()
{
    if (mRef != LUA_REFNIL)
    {
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            // Get the octave ref table. This should have already been created to store the ref originally.
            lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
            OCT_ASSERT(lua_istable(L, -1));

            luaL_unref(L, -1, mRef);

            // Pop the ref table
            lua_pop(L, 1);
        }

        mRef = LUA_REFNIL;
    }
}

void ScriptFunc::CopyRef(int ref)
{
    // Unregister any previously referenced func
    UnregisterRef();

    lua_State* L = GetLua();

    if (L != nullptr &&
        ref != LUA_REFNIL)
    {
        lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
        OCT_ASSERT(lua_istable(L, -1));
        int refTabIdx = lua_gettop(L);
        lua_geti(L, refTabIdx, ref);
        OCT_ASSERT(lua_isfunction(L, -1));

        // Register another reference to that same function
        mRef = luaL_ref(L, refTabIdx);

        // Pop ref table
        lua_pop(L, 1);
    }
}

void ScriptFunc::CreateRefTable()
{
    lua_State* L = GetLua();
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
}
