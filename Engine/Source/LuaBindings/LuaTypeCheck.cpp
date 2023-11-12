#include "LuaBindings/LuaTypeCheck.h"

#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

Node* CheckNodeWrapper(lua_State* L, int arg)
{
    luaL_checkudata(L, 1, NODE_WRAPPER_TABLE_NAME);
    Node_Lua* nodeLua = (Node_Lua*)lua_touserdata(L, 1);

    if (nodeLua->mNode == nullptr)
    {
        luaL_error(L, "Attempting to use destroyed node at arg %d", arg);
    }

    return nodeLua->mNode;
}

Node* CheckNodeLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
#if LUA_SAFE_NODE
    Node* ret = nullptr;
    Node_Lua* luaObj = static_cast<Node_Lua*>(CheckHierarchyLuaType<Node_Lua>(L, arg, className, classFlag));

    if (luaObj != nullptr)
    {
        ret = luaObj->mNode.Get();
        if (ret == nullptr)
        {
            luaL_error(L, "Attempting to use destroyed node at arg %d", arg);
        }
    }

    return ret;
#else
    Node_Lua* nodeLua = CheckHierarchyLuaType<Node_Lua>(L, arg, className, classFlag);

    if (nodeLua->mNode == nullptr)
    {
        luaL_error(L, "Attempting to use destroyed node at arg %d", arg);
    }

    return nodeLua->mNode;
#endif
}

RTTI* CheckRttiLuaType(lua_State* L, int arg)
{
    RTTI* rtti = nullptr;
    luaL_checktype(L, arg, LUA_TUSERDATA);

    // Only nodes support script-to-native function calls.
    bool isNode = (lua_getfield(L, arg, "cfNode") != LUA_TNIL);
    lua_pop(L, 1);

    if (isNode)
    {
        rtti = ((Node_Lua*)lua_touserdata(L, arg))->mNode;
    }
    else
    {
        luaL_error(L, "Error: Arg #%d: Expected Node", arg);
    }

    return rtti;
}

Asset* CheckAssetOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    return lua_isnil(L, arg) ? nullptr : CheckAssetLuaType<Asset>(L, arg, className, classFlag);
}

Node* CheckNodeOrNilLuaType(lua_State* L, int arg, const char* className, const char* classFlag)
{
    return lua_isnil(L, arg) ? nullptr : CheckNodeLuaType(L, arg, className, classFlag);
}

RTTI* CheckRttiOrNilLuaType(lua_State* L, int arg)
{
    return lua_isnil(L, arg) ? nullptr : CheckRttiLuaType(L, arg);
}

const char* CheckTableName(lua_State* L, int arg)
{
    const char* tableName = "";

    if (lua_istable(L, arg))
    {
        lua_getfield(L, arg, "tableName");

        // Script tables must have tableName assigned to them.
        // This is the name that should be used to access it from global space.
        luaL_checktype(L, -1, LUA_TSTRING);
        tableName = lua_tostring(L, -1);

        lua_pop(L, 1);
    }
    else if (lua_isstring(L, arg))
    {
        tableName = lua_tostring(L, arg);
    }

    return tableName;
}

bool CheckClassFlag(lua_State* L, int arg, const char* flag)
{
    bool isClass = false;

    if (lua_type(L, arg) == LUA_TUSERDATA)
    {
        isClass = (lua_getfield(L, arg, flag) != LUA_TNIL);
        lua_pop(L, 1);
    }

    return isClass;
}

#endif