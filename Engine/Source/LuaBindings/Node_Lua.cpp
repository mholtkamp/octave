#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/Node.h"
#include "Assets/Scene.h"

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

        // TODO-NODE: How do we extend userdata so that Lua scripts can inherit from C++ directly.
        // We want a script to be able to call world:FindNode('Genie'):Teleport() where the node has a Genie scirpt
        // where Teleport is defined. I THINK a decent solution would be to make the Script's table instance the uservalue for the userdata.
        // And then Node_Lua::Index() and NewIndex() will need to query the uservalue if it has one (after all other metatables have failed __index).

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

int Node_Lua::Attach(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    Node* newParent = nullptr;
    bool keepWorldTransform = false;

    if (!lua_isnil(L, 2))
    {
        newParent = CHECK_NODE(L, 2);
    }
    if (!lua_isnone(L, 3))
    {
        keepWorldTransform = CHECK_BOOLEAN(L, 3);
    }

    node->Attach(newParent, keepWorldTransform);

    return 0;
}

int Node_Lua::Detach(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool keepWorldTransform = false;

    if (!lua_isnone(L, 2))
    {
        keepWorldTransform = CHECK_BOOLEAN(L, 2);
    }

    node->Detach(keepWorldTransform);

    return 0;
}

int Node_Lua::GetParent(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    Node* parent = node->GetParent();

    Node_Lua::Create(L, parent);
    return 1;
}

int Node_Lua::GetChild(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    Node* child = nullptr;

    if (lua_isinteger(L, 2))
    {
        int32_t childIndex = (int32_t)lua_tointeger(L, 2) - 1;
        child = node->GetChild(childIndex);
    }
    else
    {
        const char* childName = CHECK_STRING(L, 2);
        child = node->GetChild(childName);
    }

    Node_Lua::Create(L, child);
    return 1;
}

int Node_Lua::GetChildByType(lua_State* L)
{
    // TODO-NODE: Test to see if this function works as expected.
    Node* node = CHECK_NODE(L, 1);
    const char* typeName = CHECK_STRING(L, 2);

    Node* ret = nullptr;

    for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
    {
        Node* child = node->GetChild(i);
        if (strncmp(node->GetClassName(), typeName, MAX_PATH_SIZE) == 0)
        {
            ret = child;
            break;
        }
    }

    Node_Lua::Create(L, ret);
    return 1;
}

int Node_Lua::GetNumChildren(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    uint32_t numChildren = node->GetNumChildren();

    lua_pushinteger(L, numChildren);
    return 1;
}

int Node_Lua::AddChild(lua_State* L)
{
    // TODO-NODE: Check if this works.
    Node* node = CHECK_NODE(L, 1);
    Node* newChild = CHECK_NODE(L, 2);

    newChild->Attach(node);

    return 0;
}

int Node_Lua::RemoveChild(lua_State* L)
{
    // TODO-NODE: Check if this works.
    Node* node = CHECK_NODE(L, 1);
    Node* child = nullptr;

    if (lua_isinteger(L, 2))
    {
        int32_t childIndex = (int32_t)lua_tointeger(L, 2) - 1;
        child = node->GetChild(childIndex);
    }
    else
    {
        const char* childName = CHECK_STRING(L, 2);
        child = node->GetChild(childName);
    }

    if (child != nullptr)
    {
        child->Attach(nullptr);
    }

    return 0;
}

int Node_Lua::CreateChildNode(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* nodeClass = CHECK_STRING(L, 2);

    Node* newChild = node->CreateChildNode(nodeClass);

    Node_Lua::Create(L, newChild);
    return 1;
}

int Node_Lua::CloneChildNode(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    Node* srcNode = CHECK_NODE(L, 2);

    Node* newChild = node->CloneChildNode(srcNode);

    Node_Lua::Create(L, newChild);
    return 1;
}

int Node_Lua::DestroyChildNode(lua_State* L)
{
    // TODO-NODE: Check to see if this works.
    Node* node = CHECK_NODE(L, 1);

    Node* child = nullptr;

    if (lua_isinteger(L, 2))
    {
        int32_t childIndex = (int32_t)lua_tointeger(L, 2) - 1;
        child = node->GetChild(childIndex);
    }
    else if (lua_isstring(L, 2))
    {
        const char* childName = CHECK_STRING(L, 2);
        child = node->GetChild(childName);
    }
    else
    {
        child = CHECK_NODE(L, 2);
    }

    node->DestroyChildNode(child);

    return 0;
}

int Node_Lua::DestroyAllChildren(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    node->DestroyAllChildren();

    return 0;
}

int Node_Lua::Start(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    if (!node->HasStarted())
    {
        node->Start();
    }

    return 0;
}

int Node_Lua::HasStarted(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool ret = node->HasStarted();

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::SetPendingDestroy(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool destroy = CHECK_BOOLEAN(L, 2);

    node->SetPendingDestroy(destroy);

    return 0;
}

int Node_Lua::IsPendingDestroy(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool destroy = node->IsPendingDestroy();

    lua_pushboolean(L, destroy);
    return 1;
}

int Node_Lua::EnableTick(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool enable = CHECK_BOOLEAN(L, 2);

    node->EnableTick(enable);

    return 0;
}

int Node_Lua::IsTickEnabled(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool tickEnabled = node->IsTickEnabled();

    lua_pushboolean(L, tickEnabled);
    return 1;
}

int Node_Lua::GetScene(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    Scene* scene = node->GetScene();

    Asset_Lua::Create(L, scene);
    return 1;
}

int Node_Lua::GetNetId(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    NetId netId = node->GetNetId();

    lua_pushinteger(L, (int)netId);
    return 1;
}

int Node_Lua::GetOwningHost(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    NetHostId netHostId = node->GetOwningHost();

    lua_pushinteger(L, (int)netHostId);
    return 1;
}

int Node_Lua::SetOwningHost(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    int netHostId = CHECK_INTEGER(L, 2);

    node->SetOwningHost(netHostId);

    return 0;
}

int Node_Lua::SetReplicate(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool replicate = CHECK_BOOLEAN(L, 2);

    node->SetReplicate(replicate);

    return 0;
}

int Node_Lua::IsReplicated(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool replicated = node->IsReplicated();

    lua_pushboolean(L, replicated);
    return 1;
}

int Node_Lua::ForceReplication(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    node->ForceReplication();

    return 0;
}

int Node_Lua::HasTag(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    bool ret = node->HasTag(tag);

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::AddTag(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    node->AddTag(tag);

    return 0;
}

int Node_Lua::RemoveTag(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* tag = CHECK_STRING(L, 2);

    node->RemoveTag(tag);

    return 0;
}

int Node_Lua::SetPersistent(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    node->SetPersitent(value);

    return 0;
}

int Node_Lua::IsPersistent(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool ret = node->IsPersistent();

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::HasAuthority(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool ret = node->HasAuthority();

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::IsOwned(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool ret = node->IsOwned();

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::InvokeNetFunc(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* funcName = CHECK_STRING(L, 2);

    uint32_t numParams = lua_gettop(L) - 2;

    if (numParams > 8)
    {
        LogError("Too many params for net func. Truncating to 8 params.");
        numParams = 8;
    }

    if (numParams >= 1)
    {
        std::vector<Datum> params;
        params.resize(numParams);

        for (uint32_t i = 0; i < numParams; ++i)
        {
            // First param starts at index 3
            params[i] = LuaObjectToDatum(L, 3 + i);
        }

        node->InvokeNetFunc(funcName, params);
    }
    else
    {
        node->InvokeNetFunc(funcName);
    }

    return 0;
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
