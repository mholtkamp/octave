#pragma once

#include <string>
#include <vector>

#include "Object.h"
#include "Assertion.h"
#include "EngineTypes.h"
#include "Property.h"
#include "Stream.h"
#include "Factory.h"
#include "Maths.h"
#include "NetDatum.h"
#include "NetFunc.h"
#include "ScriptUtils.h"
#include "ScriptAutoReg.h"
#include "Signals.h"
#include "Assets/StaticMesh.h"

#include "Bullet/btBulletCollisionCommon.h"

#include <unordered_map>
#include <unordered_set>

class Node;
class Scene;
class World;
class Script;

#define DECLARE_NODE(Class, Parent) \
        DECLARE_FACTORY(Class, Node); \
        DECLARE_OBJECT(Class, Parent); \
        DECLARE_SCRIPT_LINK(Class, Parent, Node) \
        typedef Parent Super;

#define DEFINE_NODE(Class, Parent) \
        DEFINE_FACTORY(Class, Node); \
        DEFINE_OBJECT(Class); \
        DEFINE_SCRIPT_LINK(Class, Parent, Node);

typedef std::unordered_map<std::string, NetFunc> NetFuncMap;

// Can also use a lambda for Traverse() and ForEach() functions
typedef bool(*NodeTraversalFP)(Node*);

#if 0
struct NodeNetData
{
    std::vector<NetDatum> mReplicatedData;
    NetId mNetId = INVALID_NET_ID;
    NetHostId mOwningHost = INVALID_HOST_ID;
    bool mReplicate = false;
    bool mReplicateTransform = false;
    bool mForceReplicate = false;
};
#endif

class Node : public Object
{
public:

    DECLARE_FACTORY_MANAGER(Node);
    DECLARE_FACTORY(Node, Node);
    DECLARE_OBJECT(Node, Object);
    DECLARE_SCRIPT_LINK_BASE(Node);

    static NodePtr Construct(const std::string& name);
    static NodePtr Construct(TypeId typeId);
    static void Destruct(Node* node);

    Node();
    virtual ~Node();

    virtual void Create();
    virtual void Destroy();
    void DestroyDeferred();
    void Doom(); // Alias for DestroyDeferred

    virtual void SaveStream(Stream& stream, Platform platorm);
    virtual void LoadStream(Stream& stream, Platform platorm, uint32_t version);

    virtual void Copy(Node* srcNode, bool recurse);
    virtual void Render(PipelineConfig pipelineConfig);

    virtual void Start();
    virtual void Stop();
    virtual void PrepareTick(std::vector<NodePtrWeak>& outTickNodes, bool game);
    virtual void Tick(float deltaTime);
    virtual void EditorTick(float deltaTime);
    virtual void Render();
    virtual VertexType GetVertexType() const;

    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void GatherReplicatedData(std::vector<NetDatum>& outData);
    virtual void GatherNetFuncs(std::vector<NetFunc>& outFuncs);

    void GatherPropertyOverrides(std::vector<Property>& outOverrides);
    void ApplyPropertyOverrides(const std::vector<Property>& overs);

    virtual void BeginOverlap(Primitive3D* thisComp, Primitive3D* otherComp);
    virtual void EndOverlap(Primitive3D* thisComp, Primitive3D* otherComp);
    virtual void OnCollision(
        Primitive3D* thisComp,
        Primitive3D* otherComp,
        glm::vec3 impactPoint,
        glm::vec3 impactNormal,
        btPersistentManifold* manifold);

    NodeId GetNodeId() const;

    void EmitSignal(const std::string& name, const std::vector<Datum>& args);
    void ConnectSignal(const std::string& name, Node* listener, SignalHandlerFP func);
    void ConnectSignal(const std::string& name, Node* listener, const ScriptFunc& func);
    void DisconnectSignal(const std::string& name, Node* listener);

    void RenderShadow();
    void RenderSelected(bool renderChildren);

    Node* CreateChild(TypeId nodeType);
    Node* CreateChild(const char* typeName);
    Node* CreateChildClone(Node* srcNode, bool recurse);
    // Clone note: useLinkedScene defaults to false 
    NodePtr Clone(bool recurse, bool instantiateLinkedScene = true);
    void DestroyAllChildren();

    Node* GetRoot();
    bool IsWorldRoot() const;

    template<class NodeClass>
    NodeClass* CreateChild()
    {
        return (NodeClass*)CreateChild(NodeClass::GetStaticType());
    }

    template<class NodeClass>
    NodeClass* CreateChild(const char* name)
    {
        NodeClass* ret = (NodeClass*)CreateChild(NodeClass::GetStaticType());
        ret->SetName(name);
        return ret;
    }

    bool IsDestroyed() const;
    bool IsPendingDestroy() const;
    bool IsDoomed() const;

    bool HasStarted() const;

    void EnableTick(bool enable);
    bool IsTickEnabled() const;

    virtual void SetWorld(World* world);
    World* GetWorld();

    void SetScene(Scene* scene);
    Scene* GetScene();

    std::vector<NetDatum>& GetReplicatedData();

    void SetNetId(NetId id);
    NetId GetNetId() const;

    NetHostId GetOwningHost() const;
    void SetOwningHost(NetHostId hostId, bool setAsPawn = false);

    void SetReplicate(bool replicate);
    bool IsReplicated() const;
    void SetReplicateTransform(bool repTransform);
    bool IsTransformReplicated() const;

    void ForceReplication();
    void ClearForcedReplication();
    bool NeedsForcedReplication();

    virtual bool CheckNetRelevance(Node* playerNode);
    bool IsAlwaysRelevant() const;
    void SetAlwaysRelevant(bool alwaysRelevant);

    bool HasTag(const std::string& tag);
    void AddTag(const std::string& tag);
    void RemoveTag(const std::string& tag);

    void SetName(const std::string& newName);
    const std::string& GetName() const;
    virtual void SetActive(bool active);
    bool IsActive(bool recurse = false) const;
    virtual void SetVisible(bool visible);
    bool IsVisible(bool recurse = false) const;
    void SetTransient(bool transient);
    virtual bool IsTransient() const;

    void SetDefault(bool isDefault);
    bool IsDefault() const;

    void SetUserdataCreated(bool created);
    bool IsUserdataCreated() const;

    virtual const char* GetTypeName() const;
    virtual DrawData GetDrawData();

    virtual bool IsNode3D() const;
    virtual bool IsWidget() const;
    virtual bool IsPrimitive3D() const;
    virtual bool IsLight3D() const;

    Node* GetParent();
    const Node* GetParent() const;
    const std::vector<NodePtr>& GetChildren() const;

    virtual void Attach(Node* parent, bool keepWorldTransform = false, int32_t index = -1);
    void Detach(bool keepWorldTransform = false);
    void AddChild(Node* child, int32_t index = -1);
    void RemoveChild(Node* child);
    void RemoveChild(int32_t index);

    int32_t FindChildIndex(const std::string& name) const;
    int32_t FindChildIndex(Node* child) const;
    Node* FindChild(const std::string& name, bool recurse) const;
    Node* FindChildWithTag(const std::string& name, bool recurse) const;
    Node* FindDescendant(const std::string& name);
    Node* FindAncestor(const std::string& name);
    bool HasAncestor(Node* node);
    Node* GetChild(int32_t index) const;
    Node* GetChildByType(TypeId type) const;
    uint32_t GetNumChildren() const;
    int32_t FindParentNodeIndex() const;

    void SetHitCheckId(uint32_t id);
    uint32_t GetHitCheckId() const;

    bool IsLateTickEnabled() const;
    void EnableLateTick(bool enable);

    Script* GetScript();
    void SetScriptFile(const std::string& fileName);

    bool DoChildrenHaveUniqueNames() const;
    void BreakSceneLink();
    bool IsSceneLinked(bool ignoreInPie = true) const;
    bool IsForeign() const;

    bool HasAuthority() const;
    bool IsOwned() const;
    bool IsLocallyControlled() const;

    NetFunc* FindNetFunc(const char* name);
    NetFunc* FindNetFunc(uint16_t index);

    void InvokeNetFunc(const char* name);
    void InvokeNetFunc(const char* name, Datum param0);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6);
    void InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6, Datum param7);
    void InvokeNetFunc(const char* name, const std::vector<Datum>& params);

    static void ProcessPendingDestroys();

    static void RegisterNetFuncs(Node* node);

    const WeakPtr<Node>& GetSelfPtr() const { return mSelf; }

    template<typename T>
    T* FindChild(const std::string& name, bool recurse)
    {
        T* ret = nullptr;
        Node* child = FindChild(name, recurse);
        if (child != nullptr)
        {
            ret = child->As<T>();
        }

        return ret;
    }

    template<typename T>
    void Traverse(T func, bool inverted = false)
    {
        bool traverseChildren = true;
        
        if (!inverted)
        {
            traverseChildren = func(this);
        }

        if (traverseChildren)
        {
            if (inverted)
            {
                for (int32_t i = int32_t(mChildren.size()) - 1; i >= 0; --i)
                {
                    mChildren[i]->Traverse(func, inverted);
                }
            }
            else
            {
                for (uint32_t i = 0; i < mChildren.size(); ++i)
                {
                    mChildren[i]->Traverse(func, inverted);
                }
            }
        }

        if (inverted)
        {
            func(this);
        }
    }

    template<typename T>
    bool ForEach(T func, bool inverted = false)
    {
        bool cont = true;

        if (!inverted)
        {
            cont = func(this);
        }

        if (cont)
        {
            if (inverted)
            {
                for (int32_t i = int32_t(mChildren.size()) - 1; i >= 0; --i)
                {
                    cont = mChildren[i]->ForEach(func, inverted);

                    if (!cont)
                    {
                        break;
                    }
                }
            }
            else
            {
                for (uint32_t i = 0; i < mChildren.size(); ++i)
                {
                    cont = mChildren[i]->ForEach(func, inverted);

                    if (!cont)
                    {
                        break;
                    }
                }
            }
        }

        if (cont && inverted)
        {
            cont = func(this);
        }

        return cont;
    }

    template<class NodeClass>
    static SharedPtr<NodeClass> Construct()
    {
        // Code copied across other Construct() functions
        NodeClass* newNode = (NodeClass*)Node::CreateInstance(NodeClass::GetStaticType());
        SharedPtr<NodeClass> newNodePtr;
        newNodePtr.Set(newNode, nullptr);
        newNodePtr.SetDeleter([](NodeClass* node) { node->Destroy(); });
        newNodePtr->mSelf = PtrStaticCast<Node>(newNodePtr);
        newNodePtr->Create();
        return newNodePtr;
    }

protected:


    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    static bool OnRep_OwningHost(Datum* datum, uint32_t index, const void* newValue);

    void TickCommon(float deltaTime);

    virtual void SetParent(Node* parent);
    void ValidateUniqueChildName(Node* newChild);

    void SendNetFunc(NetFunc* func, uint32_t numParams, const Datum** params);

    static std::unordered_map<TypeId, NetFuncMap> sTypeNetFuncMap;
    static std::unordered_set<NodePtrWeak> sPendingDestroySet;
    static NodeId sNextNodeId;

    std::string mName;

    World* mWorld = nullptr;
    NodePtrWeak mParent;
    NodePtrWeak mSelf;
    std::vector<NodePtr> mChildren;
    std::unordered_map<std::string, Node*> mChildNameMap;
    std::unordered_map<std::string, Signal> mSignalMap;
    std::string mScriptFile;

    bool mActive = true;
    bool mVisible = true;
    bool mTransient = false;
    bool mDefault = false;
    bool mUserdataCreated = false;

    // Merged from Actor
    SceneRef mScene;
    std::vector<std::string> mTags;
    NodeId mNodeId = INVALID_NODE_ID;
    uint32_t mHitCheckId = 0;

    bool mHasStarted = false;
    bool mDestroyed = false;
    bool mTickEnabled = true;
    bool mLateTick = false;

    // Network Data
    // This is only about 44 bytes, so right now, we will keep this data as direct members of Node.
    // But if we need to save data, then consider allocating a NodeNetData struct only if replicated.
    // Leaving these as direct members will improve cache performance too.
    std::vector<NetDatum> mReplicatedData;
    NetId mNetId = INVALID_NET_ID;
    NetHostId mOwningHost = INVALID_HOST_ID;
    bool mReplicate = false;
    bool mReplicateTransform = false;
    bool mForceReplicate = false;
    bool mAlwaysRelevant = true;

    Script* mScript = nullptr;
    //NodeNetData* mNetData = nullptr;

#if EDITOR
public:
    // TODO-NODE: Either remove mExposeVariable, or make sure it works with Scenes.
    // Could consider moving this up to Node if it would be useful from avoiding FindChild() calls in Start().
    bool ShouldExposeVariable() const;
    void SetExposeVariable(bool expose);
    bool AreAllChildrenHiddenInTree() const;

    bool mExposeVariable = false;
    bool mHiddenInTree = false;
#endif
};

// Eventually, if we move the mSelf pointer into Object, we will have to
// move these functions also (and make them take Object* param)
template<typename T = Node>
SharedPtr<T> ResolvePtr(Node* node)
{
    NodePtr nodePtr = node ? node->GetSelfPtr().Lock() : nullptr;
    return PtrStaticCast<T>(nodePtr);
}

template<typename T = Node>
WeakPtr<T> ResolveWeakPtr(Node* node)
{
    return PtrStaticCast<T>(node ? node->GetSelfPtr() : nullptr);
}
