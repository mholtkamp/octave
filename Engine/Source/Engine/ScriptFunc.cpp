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

void ScriptFunc::Call(uint32_t numParams, Datum* params)
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

Datum ScriptFunc::CallR(uint32_t numParams, Datum* params)
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

bool ScriptFunc::IsValid() const
{
    return (mRef != LUA_REFNIL);
}

void ScriptFunc::RegisterRef(lua_State* L, int arg)
{
    OCT_ASSERT(mRef == LUA_REFNIL);
    OCT_ASSERT(lua_isfunction(L, arg));

    lua_getfield(L, LUA_REGISTRYINDEX, REF_TABLE_NAME);
    OCT_ASSERT(lua_istable(L, -1));

    // Push the function on to the top of stack
    lua_pushvalue(L, arg);

    mRef = luaL_ref(L, -2);

    // Pop ref table
    lua_pop(L, 1);
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
