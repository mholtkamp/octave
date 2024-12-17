#include "LuaBindings/NodeRef_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "Log.h"

#if LUA_ENABLED

int NodeRef_Lua::Create(lua_State* L)
{
    Node* srcNode = nullptr;

    if (lua_isuserdata(L, 1))
    {
        srcNode = CHECK_NODE(L, 1);
    }


    NodeRef_Lua* newRef = (NodeRef_Lua*)lua_newuserdata(L, sizeof(NodeRef_Lua));
    new (newRef) NodeRef_Lua();
    luaL_getmetatable(L, NODE_REF_LUA_NAME);
    OCT_ASSERT(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    if (srcNode != nullptr)
    {
        newRef->mNodeRef = srcNode;
    }

    return 1;
}

int NodeRef_Lua::Destroy(lua_State* L)
{
    CHECK_NODE_REF(L, 1);
    NodeRef_Lua* refLua = (NodeRef_Lua*)lua_touserdata(L, 1);
    refLua->~NodeRef_Lua();
    return 0;
}

int NodeRef_Lua::Equals(lua_State* L)
{
    NodeRef& refA = CHECK_NODE_REF(L, 1);

    bool ret = false;

    if (lua_isuserdata(L, 2))
    {
        NodeRef& refB = CHECK_NODE_REF(L, 2);
        ret = (refA.Get() == refB.Get());
    }
    else if (lua_isnil(L, 2))
    {
        ret = (refA.Get() == nullptr);
    }

    lua_pushboolean(L, ret);
    return 1;
}

int NodeRef_Lua::Get(lua_State* L)
{
    NodeRef& ref = CHECK_NODE_REF(L, 1);

    Node_Lua::Create(L, ref.Get());
    return 1;
}

int NodeRef_Lua::Set(lua_State* L)
{
    NodeRef& ref = CHECK_NODE_REF(L, 1);

    Node* actor = nullptr;

    if (!lua_isnil(L, 2))
    {
        actor = CHECK_NODE(L, 2);
    }

    ref.Set(actor);

    return 0;
}

void NodeRef_Lua::Bind()
{
    lua_State* L = GetLua();
    OCT_ASSERT(lua_gettop(L) == 0);

    luaL_newmetatable(L, NODE_REF_LUA_NAME);
    int mtIndex = lua_gettop(L);

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Destroy, "__gc");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Equals, "__eq");

    REGISTER_TABLE_FUNC(L, mtIndex, Create);

    REGISTER_TABLE_FUNC(L, mtIndex, Destroy);

    REGISTER_TABLE_FUNC(L, mtIndex, Get);

    REGISTER_TABLE_FUNC(L, mtIndex, Set);

    // Set the __index metamethod to itself
    lua_pushvalue(L, mtIndex);
    lua_setfield(L, mtIndex, "__index");

    lua_setglobal(L, NODE_REF_LUA_NAME);

    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif