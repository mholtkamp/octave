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
        if (node->GetUserdataRef() != LUA_REFNIL)
        {
            // The user data already exists. Grab it from the registry.
            lua_rawgeti(L, LUA_REGISTRYINDEX, node->GetUserdataRef());
            OCT_ASSERT(lua_isuserdata(L, -1));
        }
        else
        {
            // Create new userdata
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

            // Create a registry reference to the new userdata, and then save it on Node
            // so we can reference it next time. The corresponding unref() call is done in Node::Destroy()
            lua_pushvalue(L, udIndex);
            int ref = luaL_ref(L, LUA_REGISTRYINDEX);
            node->SetUserdataRef(ref);
        }
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

int Node_Lua::GarbageCollect(lua_State* L)
{
    CHECK_NODE(L, 1);
    Node_Lua* nodeLua = (Node_Lua*)lua_touserdata(L, 1);
    nodeLua->~Node_Lua();
    return 0;
}

int Node_Lua::Construct(lua_State* L)
{
    // Allow script to allocate a node.
    const char* className = CHECK_STRING(L, 1);
    
    Node* newNode = Node::Construct(className);

    Node_Lua::Create(L, newNode);
    return 1;
}

int Node_Lua::Index(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    // TODO: Do we want to support integer keys??
    const char* key = CHECK_STRING(L, 2);

    // Check if the Node metatable has the key
    lua_getglobal(L, NODE_LUA_NAME);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);

    if (lua_isnil(L, -1))
    {
        // Otherwise check the uservalue (script)
        lua_getuservalue(L, 1);
        if (lua_istable(L, -1))
        {
            lua_pushstring(L, key);
            lua_rawget(L, -2);
        }
        else
        {
            // Pop getuservalue return (probably nil). Nil should still be on top of stack.
            lua_pop(L, 1);
        }
    }

    return 1;
}

int Node_Lua::NewIndex(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    // TODO: Do we want to support integer keys??
    const char* key = CHECK_STRING(L, 2);

    // First, see if we have a uservalue and assign it to the uservalue table.
    lua_getuservalue(L, 1);
    if (lua_istable(L, -1))
    {
        int uservalueIdx = lua_gettop(L);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_rawset(L, uservalueIdx);
    }
    else
    {
        // If no uservalue, then assign to the table itself. 
        lua_getglobal(L, NODE_LUA_NAME);
        int nodeTableIdx = lua_gettop(L);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_rawset(L, nodeTableIdx);
    }

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
    bool recurse = false;

    if (!lua_isnone(L, 2)) { recurse = CHECK_BOOLEAN(L, 2); }

    bool active = node->IsActive(recurse);

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
    bool recurse = false;

    if (!lua_isnone(L, 2)) { recurse = CHECK_BOOLEAN(L, 2); }

    bool isVisible = node->IsVisible(recurse);

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
    int32_t childIndex = -1;

    if (!lua_isnil(L, 2))
    {
        newParent = CHECK_NODE(L, 2);
    }
    if (!lua_isnone(L, 3))
    {
        keepWorldTransform = CHECK_BOOLEAN(L, 3);
    }
    if (!lua_isnone(L, 4))
    {
        childIndex = CHECK_INTEGER(L, 4);
    }

    node->Attach(newParent, keepWorldTransform, childIndex);

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
    int32_t index = CHECK_INTEGER(L, 2);
    index--; // Lua indices start at 1.

    Node* child = node->GetChild(index);

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
    int32_t index = -1;

    if (!lua_isnone(L, 3)) { index = CHECK_INDEX(L, 3); }

    node->AddChild(newChild, index);

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
        child = node->FindChild(childName, false);
    }

    if (child != nullptr)
    {
        child->Attach(nullptr);
    }

    return 0;
}

int Node_Lua::FindChild(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* name = CHECK_STRING(L, 2);
    bool recurse = false;

    if (!lua_isnone(L, 3)) { recurse = CHECK_BOOLEAN(L, 3); }

    Node* ret = node->FindChild(name, recurse);

    Node_Lua::Create(L, ret);
    return 1;
}

int Node_Lua::FindChildWithTag(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* tag = CHECK_STRING(L, 2);
    bool recurse = false;

    if (!lua_isnone(L, 3)) { recurse = CHECK_BOOLEAN(L, 3); }

    Node* ret = node->FindChildWithTag(tag, recurse);

    Node_Lua::Create(L, ret);
    return 1;
}

int Node_Lua::FindDescendant(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Node* ret = node->FindDescendant(name);

    Node_Lua::Create(L, ret);
    return 1;
}

int Node_Lua::FindAncestor(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* name = CHECK_STRING(L, 2);

    Node* ret = node->FindAncestor(name);

    Node_Lua::Create(L, ret);
    return 1;
}

int Node_Lua::HasAncestor(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    Node* otherNode = CHECK_NODE(L, 2);

    bool ret = node->HasAncestor(otherNode);

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::GetRoot(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    Node* root = node->GetRoot();

    Node_Lua::Create(L, root);
    return 1;
}

int Node_Lua::IsWorldRoot(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool ret = node->IsWorldRoot();

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::Traverse(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc scriptFunc(L, 2);

    auto callScriptFunc = [&](Node* node) -> bool
    {
        Datum args[1] = { node };
        bool ret = scriptFunc.CallR(1, args);
        return ret;
    };

    node->Traverse(callScriptFunc);

    return 0;
}

int Node_Lua::ForEach(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    CHECK_FUNCTION(L, 2);
    ScriptFunc scriptFunc(L, 2);

    auto callScriptFunc = [&](Node* node) -> bool
    {
        Datum args[1] = { node };
        bool ret = scriptFunc.CallR(1, args);
        return ret;
    };

    bool ret = node->ForEach(callScriptFunc);

    lua_pushboolean(L, ret);
    return ret;
}

int Node_Lua::CreateChild(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    const char* nodeClass = CHECK_STRING(L, 2);

    Node* newChild = node->CreateChild(nodeClass);

    Node_Lua::Create(L, newChild);
    return 1;
}

int Node_Lua::Clone(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool recurse = CHECK_BOOLEAN(L, 2);

    Node* clonedNode = node->Clone(recurse);

    Node_Lua::Create(L, clonedNode);
    return 1;
}

int Node_Lua::CreateChildClone(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    Node* srcNode = CHECK_NODE(L, 2);
    bool recurse = CHECK_BOOLEAN(L, 3);

    Node* newChild = node->CreateChildClone(srcNode, recurse);

    Node_Lua::Create(L, newChild);
    return 1;
}

int Node_Lua::DestroyChild(lua_State* L)
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
        child = node->FindChild(childName, false);
    }
    else
    {
        child = CHECK_NODE(L, 2);
    }

    node->DestroyChild(child);

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

int Node_Lua::Destroy(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    Node::Destruct(node);
    node = nullptr;

    Node_Lua* nodeLua = (Node_Lua*)lua_touserdata(L, 1);
    nodeLua->mNode = nullptr;

    return 0;
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

int Node_Lua::IsLateTickEnabled(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);

    bool ret = node->IsLateTickEnabled();

    lua_pushboolean(L, ret);
    return 1;
}

int Node_Lua::EnableLateTick(lua_State* L)
{
    Node* node = CHECK_NODE(L, 1);
    bool value = CHECK_BOOLEAN(L, 2);

    node->EnableLateTick(value);

    return 0;
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

    std::vector<Datum> params;

    if (numParams > 0)
    {
        params.resize(numParams);

        for (uint32_t i = 0; i < numParams; ++i)
        {
            // First param starts at index 3
            params[i] = LuaObjectToDatum(L, 3 + i);
        }
    }

    node->InvokeNetFunc(funcName, params);

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
    REGISTER_TABLE_FUNC_EX(L, mtIndex, GarbageCollect, "__gc");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Equals, "__eq");
}

void Node_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        NODE_LUA_NAME,
        NODE_LUA_FLAG,
        nullptr);

    BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, Construct);
    REGISTER_TABLE_FUNC(L, mtIndex, Construct); // Alias

    REGISTER_TABLE_FUNC(L, mtIndex, IsValid);

    REGISTER_TABLE_FUNC(L, mtIndex, GetName);

    REGISTER_TABLE_FUNC(L, mtIndex, SetName);

    REGISTER_TABLE_FUNC(L, mtIndex, SetActive);

    REGISTER_TABLE_FUNC(L, mtIndex, IsActive);

    REGISTER_TABLE_FUNC(L, mtIndex, SetVisible);

    REGISTER_TABLE_FUNC(L, mtIndex, IsVisible);

    REGISTER_TABLE_FUNC(L, mtIndex, GetWorld);

    REGISTER_TABLE_FUNC(L, mtIndex, Equals);

    REGISTER_TABLE_FUNC(L, mtIndex, GetParent);

    REGISTER_TABLE_FUNC(L, mtIndex, Attach);

    REGISTER_TABLE_FUNC(L, mtIndex, Detach);

    REGISTER_TABLE_FUNC(L, mtIndex, GetChild);

    REGISTER_TABLE_FUNC(L, mtIndex, GetChildByType);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumChildren);

    REGISTER_TABLE_FUNC(L, mtIndex, AddChild);

    REGISTER_TABLE_FUNC(L, mtIndex, RemoveChild);

    REGISTER_TABLE_FUNC(L, mtIndex, FindChild);

    REGISTER_TABLE_FUNC(L, mtIndex, FindChildWithTag);

    REGISTER_TABLE_FUNC(L, mtIndex, FindDescendant);

    REGISTER_TABLE_FUNC(L, mtIndex, FindAncestor);

    REGISTER_TABLE_FUNC(L, mtIndex, HasAncestor);

    REGISTER_TABLE_FUNC(L, mtIndex, GetRoot);
    
    REGISTER_TABLE_FUNC(L, mtIndex, IsWorldRoot);

    REGISTER_TABLE_FUNC(L, mtIndex, Traverse);

    REGISTER_TABLE_FUNC(L, mtIndex, ForEach);

    REGISTER_TABLE_FUNC(L, mtIndex, CreateChild);

    REGISTER_TABLE_FUNC(L, mtIndex, CreateChildClone);

    REGISTER_TABLE_FUNC(L, mtIndex, Clone);

    REGISTER_TABLE_FUNC(L, mtIndex, DestroyChild);

    REGISTER_TABLE_FUNC(L, mtIndex, DestroyAllChildren);

    REGISTER_TABLE_FUNC(L, mtIndex, Start);

    REGISTER_TABLE_FUNC(L, mtIndex, HasStarted);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPendingDestroy);

    REGISTER_TABLE_FUNC(L, mtIndex, IsPendingDestroy);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableTick);

    REGISTER_TABLE_FUNC(L, mtIndex, IsTickEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, GetScene);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNetId);

    REGISTER_TABLE_FUNC(L, mtIndex, GetOwningHost);

    REGISTER_TABLE_FUNC(L, mtIndex, SetOwningHost);

    REGISTER_TABLE_FUNC(L, mtIndex, SetReplicate);

    REGISTER_TABLE_FUNC(L, mtIndex, IsReplicated);

    REGISTER_TABLE_FUNC(L, mtIndex, ForceReplication);

    REGISTER_TABLE_FUNC(L, mtIndex, HasTag);

    REGISTER_TABLE_FUNC(L, mtIndex, AddTag);

    REGISTER_TABLE_FUNC(L, mtIndex, RemoveTag);

    REGISTER_TABLE_FUNC(L, mtIndex, HasAuthority);

    REGISTER_TABLE_FUNC(L, mtIndex, IsOwned);

    REGISTER_TABLE_FUNC(L, mtIndex, IsLateTickEnabled);

    REGISTER_TABLE_FUNC(L, mtIndex, EnableLateTick);

    REGISTER_TABLE_FUNC(L, mtIndex, InvokeNetFunc);

    REGISTER_TABLE_FUNC(L, mtIndex, CheckType);
    REGISTER_TABLE_FUNC_EX(L, mtIndex, CheckType, "Is");
    REGISTER_TABLE_FUNC_EX(L, mtIndex, CheckType, "IsA");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, Index, "__index");

    REGISTER_TABLE_FUNC_EX(L, mtIndex, NewIndex, "__newindex");

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
