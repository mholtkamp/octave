#pragma once

#include <string>
#include <vector>

#include "RTTI.h"
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
        DECLARE_RTTI(Class, Parent); \
        DECLARE_SCRIPT_LINK(Class, Parent, Node) \
        typedef Parent Super;

#define DEFINE_NODE(Class, Parent) \
        DEFINE_FACTORY(Class, Node); \
        DEFINE_RTTI(Class); \
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
    ReplicationRate mReplicationRate = ReplicationRate::High;
};
#endif

class Node : public RTTI
{
public:

    DECLARE_FACTORY_MANAGER(Node);
    DECLARE_FACTORY(Node, Node);
    DECLARE_RTTI(Node, RTTI);
    DECLARE_SCRIPT_LINK_BASE(Node);

    static Node* Construct(const std::string& name);
    static Node* Construct(TypeId typeId);
    static void Destruct(Node* node);

    Node();
    virtual ~Node();

    virtual void Create();
    virtual void Destroy();
    
    virtual void SaveStream(Stream& stream, Platform platorm);
    virtual void LoadStream(Stream& stream, Platform platorm, uint32_t version);

    virtual void Copy(Node* srcNode, bool recurse);
    virtual void Render(PipelineConfig pipelineConfig);

    virtual void Start();
    virtual void Stop();
    virtual void RecursiveTick(float deltaTime, bool game);
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

    void RenderShadow();
    void RenderSelected(bool renderChildren);

    Node* CreateChild(TypeId nodeType);
    Node* CreateChild(const char* typeName);
    Node* CreateChildClone(Node* srcNode, bool recurse);
    // Clone note: useLinkedScene defaults to false 
    Node* Clone(bool recurse, bool instantiateLinkedScene = true);
    void DestroyChild(Node* node);
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

    void SetPendingDestroy(bool pendingDestroy);
    bool IsPendingDestroy() const;
    void FlushPendingDestroys();

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
    void SetOwningHost(NetHostId hostId);

    void SetReplicate(bool replicate);
    bool IsReplicated() const;
    void SetReplicateTransform(bool repTransform);
    bool IsTransformReplicated() const;

    void ForceReplication();
    void ClearForcedReplication();
    bool NeedsForcedReplication();

    ReplicationRate GetReplicationRate() const;
    //void SetReplicationRate(ReplicationRate rate);

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

    virtual const char* GetTypeName() const;
    virtual DrawData GetDrawData();

    virtual bool IsNode3D() const;
    virtual bool IsWidget() const;
    virtual bool IsPrimitive3D() const;
    virtual bool IsLight3D() const;

    Node* GetParent();
    const Node* GetParent() const;
    const std::vector<Node*>& GetChildren() const;

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
    bool IsSceneLinked() const;
    bool IsForeign() const;

    bool HasAuthority() const;
    bool IsOwned() const;

    int GetUserdataRef() const;
    void SetUserdataRef(int ref);

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

    static void RegisterNetFuncs(Node* node);

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
    static NodeClass* Construct()
    {
        return (NodeClass*)Construct(NodeClass::GetStaticType());
    }

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    static bool OnRep_OwningHost(Datum* datum, uint32_t index, const void* newValue);

    void TickCommon(float deltaTime);

    virtual void SetParent(Node* parent);
    void ValidateUniqueChildName(Node* newChild);

    void SendNetFunc(NetFunc* func, uint32_t numParams, const Datum** params);

    static std::unordered_map<TypeId, NetFuncMap> sTypeNetFuncMap;

    std::string mName;

    World* mWorld = nullptr;
    Node* mParent = nullptr;
    std::vector<Node*> mChildren;
    std::unordered_map<std::string, Node*> mChildNameMap;
    std::string mScriptFile;

    bool mActive = true;
    bool mVisible = true;
    bool mTransient = false;
    bool mDefault = false;

    // Merged from Actor
    SceneRef mScene;
    std::vector<std::string> mTags;
    uint32_t mHitCheckId = 0;

    bool mHasStarted = false;
    bool mPendingDestroy = false;
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
    ReplicationRate mReplicationRate = ReplicationRate::High;

    Script* mScript = nullptr;
    int mUserdataRef = LUA_REFNIL;
    //NodeNetData* mNetData = nullptr;

#if EDITOR
public:
    // TODO-NODE: Either remove mExposeVariable, or make sure it works with Scenes.
    // Could consider moving this up to Node if it would be useful from avoiding FindChild() calls in Start().
    bool ShouldExposeVariable() const;
    void SetExposeVariable(bool expose);

protected:

    bool mExposeVariable = false;
#endif
};
