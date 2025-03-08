#include "LuaBindings/Signal_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "Log.h"

#if LUA_ENABLED

int Signal_Lua::Create(lua_State* L)
{
    Signal_Lua* newObject = (Signal_Lua*)lua_newuserdata(L, sizeof(Signal_Lua));
    new (newObject) Signal_Lua();
    luaL_getmetatable(L, SIGNAL_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    return 1;
}

int Signal_Lua::Destroy(lua_State* L)
{
    CHECK_SIGNAL(L, 1);
    Signal_Lua* obj = (Signal_Lua*)lua_touserdata(L, 1);
    obj->~Signal_Lua();
    return 0;
}

int Signal_Lua::Emit(lua_State* L)
{
    Signal& signal = CHECK_SIGNAL(L, 1);

    std::vector<Datum> args;
    int numArgs = lua_gettop(L);
    args.resize(numArgs);

    for (int32_t i = 1; i <= numArgs; ++i)
    {
        args.push_back(LuaObjectToDatum(L, i));
    }

    signal.Emit(args);

    return 0;
}

int Signal_Lua::Connect(lua_State* L)
{
    Signal& signal = CHECK_SIGNAL(L, 1);
    Node* listenerNode = CHECK_NODE(L, 2);
    CHECK_FUNCTION(L, 3);
    ScriptFunc listenerFunc(L, 3);

    signal.Connect(listenerNode, listenerFunc);

    return 0;
}

int Signal_Lua::Disconnect(lua_State* L)
{
    Signal& signal = CHECK_SIGNAL(L, 1);
    Node* listenerNode = CHECK_NODE(L, 2);

    signal.Disconnect(listenerNode);

    return 0;
}

void Signal_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, SIGNAL_LUA_NAME);
    int mtIndex = lua_gettop(L);

    REGISTER_TABLE_FUNC(L, mtIndex, Create);

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Destroy, "__gc");

    REGISTER_TABLE_FUNC(L, mtIndex, Emit);

    REGISTER_TABLE_FUNC(L, mtIndex, Connect);

    REGISTER_TABLE_FUNC(L, mtIndex, Disconnect);

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_setglobal(L, SIGNAL_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
