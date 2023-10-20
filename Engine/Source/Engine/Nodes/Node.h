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
class World;

#define DECLARE_NODE(Base, Parent) \
        DECLARE_FACTORY(Base, Node); \
        DECLARE_RTTI(Base, Parent); \
        DECLARE_SCRIPT_LINK(Base, Parent, Node);

#define DEFINE_NODE(Base, Parent) \
        DEFINE_FACTORY(Base, Node); \
        DEFINE_RTTI(Base); \
        DEFINE_SCRIPT_LINK(Base, Parent, Node);

typedef std::unordered_map<std::string, NetFunc> NetFuncMap;

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

    static Node* CreateNew(const std::string& name);
    static Node* CreateNew(TypeId typeId);

    Node();
    virtual ~Node();

    virtual void Create();
    virtual void Destroy();
    
    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream);

    virtual void Copy(Node* srcNode);
    virtual void Render(PipelineId pipelineId);

    virtual void Start();
    virtual void Stop();
    virtual void RecursiveTick(float deltaTime, bool game);
    virtual void Tick(float deltaTime);
    virtual void EditorTick(float deltaTime);

    virtual void GatherProperties(std::vector<Property>& outProps);
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
    void DestroyChild(Node* node);
    void DestroyAllChildren();

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
    void SetActive(bool active);
    bool IsActive(bool recurse = false) const;
    void SetVisible(bool visible);
    bool IsVisible(bool recurse = false) const;
    void SetTransient(bool transient);
    virtual bool IsTransient() const;

    void SetDefault(bool isDefault);
    bool IsDefault() const;

    World* GetWorld();

    virtual const char* GetTypeName() const;
    virtual DrawData GetDrawData();

    virtual bool IsNode3D() const;
    virtual bool IsWidget() const;
    virtual bool IsPrimitive3D() const;
    virtual bool IsLight3D() const;

    Node* GetParent();
    const Node* GetParent() const;
    const std::vector<Node*>& GetChildren() const;

    virtual void Attach(Node* parent, bool keepWorldTransform = false);
    void Detach(bool keepWorldTransform = false);
    void AddChild(Node* child, int32_t index = -1);
    void RemoveChild(Node* child);
    void RemoveChild(int32_t index);

    int32_t FindChildIndex(const std::string& name) const;
    Node* FindChild(const std::string& name, bool recurse) const;
    Node* FindDescendant(const std::string& name);
    Node* FindAncestor(const std::string& name);
    bool HasAncestor(Node* node);
    Node* GetChild(int32_t index) const;
    Node* GetChildByType(TypeId type) const;
    uint32_t GetNumChildren() const;
    int32_t FindParentNodeIndex() const;

    void SetHitCheckId(uint32_t id);
    uint32_t GetHitCheckId() const;

    Script* GetScript();

    bool DoChildrenHaveUniqueNames() const;

    bool HasAuthority() const;
    bool IsOwned() const;

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

protected:

    virtual void SetParent(Node* parent);

    void SendNetFunc(NetFunc* func, uint32_t numParams, Datum** params);

    static std::unordered_map<TypeId, NetFuncMap> sTypeNetFuncMap;

    std::string mName;

    World* mWorld = nullptr;
    Node* mParent = nullptr;
    std::vector<Node*> mChildren;

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
    bool mTransient = false;
    bool mVisible = true;

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
    //NodeNetData* mNetData = nullptr;

};
