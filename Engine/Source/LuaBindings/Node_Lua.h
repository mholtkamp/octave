#pragma once

#include "Engine.h"
#include "SmartPointer.h"

#if LUA_ENABLED

#define OCT_CLASS_TABLE_KEY "octClassTable"
#define NODE_WRAPPER_TABLE_NAME "NodeWrapper"

#define NODE_LUA_NAME "Node"
#define NODE_LUA_FLAG "cfNode"
#define CHECK_NODE(L, arg)  CheckNodeWrapper(L, arg);

struct Node_Lua
{
    NodePtr mNode;

    static int Create(lua_State* L, Node* node);
    static int Construct(lua_State* L);
    static int Destruct(lua_State* L);

    static int IsValid(lua_State* L);

    static int GetName(lua_State* L);
    static int SetName(lua_State* L);
    static int SetActive(lua_State* L);
    static int IsActive(lua_State* L);
    static int SetVisible(lua_State* L);
    static int IsVisible(lua_State* L);
    static int GetWorld(lua_State* L);

    static int GetParent(lua_State* L);
    static int Attach(lua_State* L);
    static int Detach(lua_State* L);
    static int GetChild(lua_State* L);
    static int GetChildByType(lua_State* L);
    static int GetNumChildren(lua_State* L);
    static int AddChild(lua_State* L);
    static int RemoveChild(lua_State* L);
    static int FindChild(lua_State* L);
    static int FindChildWithTag(lua_State* L);
    static int FindDescendant(lua_State* L);
    static int FindAncestor(lua_State* L);
    static int HasAncestor(lua_State* L);

    static int GetRoot(lua_State* L);
    static int IsWorldRoot(lua_State* L);

    static int Traverse(lua_State* L);
    static int ForEach(lua_State* L);

    static int CreateChild(lua_State* L);
    static int CreateChildClone(lua_State* L);
    static int Clone(lua_State* L);
    static int DestroyChild(lua_State* L);
    static int DestroyAllChildren(lua_State* L);

    static int Start(lua_State* L);
    static int HasStarted(lua_State* L);

    static int Destroy(lua_State* L);
    static int IsDestroyed(lua_State* L);
    static int EnableTick(lua_State* L);
    static int IsTickEnabled(lua_State* L);

    static int GetScene(lua_State* L);

    static int GetNetId(lua_State* L);
    static int GetOwningHost(lua_State* L);
    static int SetOwningHost(lua_State* L);

    static int SetReplicate(lua_State* L);
    static int IsReplicated(lua_State* L);
    static int SetReplicateTransform(lua_State* L);
    static int IsTransformReplicated(lua_State* L);
    static int ForceReplication(lua_State* L);

    static int HasTag(lua_State* L);
    static int AddTag(lua_State* L);
    static int RemoveTag(lua_State* L);

    static int HasAuthority(lua_State* L);
    static int IsOwned(lua_State* L);

    static int IsLateTickEnabled(lua_State* L);
    static int EnableLateTick(lua_State* L);

    static int IsAlwaysRelevant(lua_State* L);
    static int SetAlwaysRelevant(lua_State* L);

    static int InvokeNetFunc(lua_State* L);

    static int CheckType(lua_State* L);

    static void BindCommon(lua_State* L, int mtIndex);
    static void Bind();
};

#endif
