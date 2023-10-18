#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/Node.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/World_Lua.h"

#if LUA_ENABLED

int Node_Lua::Create(lua_State* L, Node* node)
{
    if (node != nullptr)
    {
        Node_Lua* nodeLua = (Node_Lua*)lua_newuserdata(L, sizeof(Node_Lua));
        new (nodeLua) Node_Lua();
        nodeLua->mNode = node;

        int udIndex = lua_gettop(L);

        luaL_getmetatable(L, node->GetClassName());
        if (lua_isnil(L, -1))
        {
            LogWarning("Could not find object's metatable, so the top-level metatable will be used.");

            // Could not find this type's metatable, so just use Node
            lua_pop(L, 1);
            luaL_getmetatable(L, NODE_LUA_NAME);
        }

        OCT_ASSERT(lua_istable(L, -1));
        lua_setmetatable(L, udIndex);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Node_Lua::Destroy(lua_State* L)
{
    CHECK_NODE(L, 1);
    Node_Lua* nodeLua = (Node_Lua*)lua_touserdata(L, 1);
    nodeLua->~Node_Lua();
    return 0;
}

int Node_Lua::IsValid(lua_State* L)
{
#if LUA_SAFE_NODE
    Node_Lua* luaObj = static_cast<Node_Lua*>(CheckHierarchyLuaType<Node_Lua>(L, 1, NODE_LUA_NAME, NODE_LUA_FLAG));

    bool ret = (luaObj->mNode.Get() != nullptr);

    lua_pushboolean(L, ret);
    return 1;
#else
    lua_pushboolean(L, true);
    return 1;
#endif
}

int Node_Lua::GetName(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    const std::string& name = node->GetName();

    lua_pushstring(L, name.c_str());
    return 1;
}

int Node_Lua::SetName(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* name = CHECK_STRING(L, 2);

    node->SetName(name);

    return 0;
}

int Node_Lua::SetActive(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool active = CHECK_BOOLEAN(L, 2);

    node->SetActive(active);

    return 0;
}

int Node_Lua::IsActive(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool active = node->IsActive();

    lua_pushboolean(L, active);
    return 1;
}

int Node_Lua::SetVisible(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool visible = CHECK_BOOLEAN(L, 2);

    node->SetVisible(visible);

    return 0;
}

int Node_Lua::IsVisible(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool isVisible = node->IsVisible();

    lua_pushboolean(L, isVisible);
    return 1;
}

int Node_Lua::GetWorld(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    World* world = node->GetWorld();

    World_Lua::Create(L, world);
    return 1;
}

int Node_Lua::Equals(lua_State* L)
{
    Node* nodeA = CHECK_NODE(L, 1);
    Node* nodeB = nullptr;

    if (lua_isuserdata(L, 2))
    {
        nodeB = CHECK_NODE(L, 2);
    }

    bool ret = (nodeA == nodeB);

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::CheckType(lua_State* L)
{
    bool ret = false;
    CHECK_NODE(L, 1);
    const char* typeName = CHECK_STRING(L, 2);

    if (lua_getmetatable(L, 1))
    {
        char classFlag[64];
        snprintf(classFlag, 64, "cf%s", typeName);
        lua_getfield(L, 1, classFlag);

        if (!lua_isnil(L, -1))
        {
            ret = true;
        }
    }

    return ret;
}

void Node_Lua::BindCommon(lua_State* L, int mtIndex)
{
    lua_pushcfunction(L, Destroy);
    lua_setfield(L, mtIndex, "__gc");

    lua_pushcfunction(L, Equals);
    lua_setfield(L, mtIndex, "__eq");
}

void Node_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        NODE_LUA_NAME,
        NODE_LUA_FLAG,
        nullptr);

    BindCommon(L, mtIndex);

    lua_pushcfunction(L, IsValid);
    lua_setfield(L, mtIndex, "IsValid");

    lua_pushcfunction(L, GetOwner);
    lua_setfield(L, mtIndex, "GetOwner");

    lua_pushcfunction(L, GetName);
    lua_setfield(L, mtIndex, "GetName");

    lua_pushcfunction(L, SetName);
    lua_setfield(L, mtIndex, "SetName");

    lua_pushcfunction(L, SetActive);
    lua_setfield(L, mtIndex, "SetActive");

    lua_pushcfunction(L, IsActive);
    lua_setfield(L, mtIndex, "IsActive");

    lua_pushcfunction(L, SetVisible);
    lua_setfield(L, mtIndex, "SetVisible");

    lua_pushcfunction(L, IsVisible);
    lua_setfield(L, mtIndex, "IsVisible");

    lua_pushcfunction(L, GetWorld);
    lua_setfield(L, mtIndex, "GetWorld");

    lua_pushcfunction(L, Equals);
    lua_setfield(L, mtIndex, "Equals");

    lua_pushcfunction(L, CheckType);
    lua_pushcfunction(L, CheckType);
    lua_pushcfunction(L, CheckType);
    lua_setfield(L, mtIndex, "CheckType");
    lua_setfield(L, mtIndex, "Is");
    lua_setfield(L, mtIndex, "IsA");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
