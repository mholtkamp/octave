#pragma once

#include <string>
#include <vector>
#include "Assertion.h"

#include "RTTI.h"
#include "EngineTypes.h"
#include "Property.h"
#include "Stream.h"
#include "Factory.h"

class Actor;
class World;

#define DECLARE_NODE(Base, Parent) DECLARE_FACTORY(Base, Node); DECLARE_RTTI(Base, Parent);
#define DEFINE_NODE(Base) DEFINE_FACTORY(Base, Node); DEFINE_RTTI(Base);

struct NodeScriptData
{
    std::string mFileName;
    std::string mClassName;
    std::string mTableName;
    std::vector<Property> mScriptProps;
    std::vector<ScriptNetDatum> mReplicatedData;
    bool mTickEnabled = false;
    bool mHandleBeginOverlap = false;
    bool mHandleEndOverlap = false;
    bool mHandleOnCollision = false;
};

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
    virtual void Tick(float deltaTime);
    virtual void EditorTick(float deltaTime);

    virtual void GatherProperties(std::vector<Property>& outProps);
    virtual void GatherReplicatedData(std::vector<NetDatum>& outData);
    virtual void GatherNetFuncs(std::vector<NetFunc>& outFuncs);

    void SetName(const std::string& newName);
    const std::string& GetName() const;
    void SetActive(bool active);
    bool IsActive() const;
    void SetVisible(bool visible);
    bool IsVisible() const;
    void SetTransient(bool transient);
    virtual bool IsTransient() const;

    void SetDefault(bool isDefault);
    bool IsDefault() const;

    World* GetWorld();

    virtual const char* GetTypeName() const;
    virtual DrawData GetDrawData();

    virtual bool IsTransformNode() const;
    virtual bool IsPrimitiveNode() const;
    virtual bool IsLightNode() const;

    Node* GetParent();
    const std::vector<Node*>& GetChildren() const;

    virtual void Attach(Node* parent, bool keepWorldTransform = false);
    void AddChild(Node* child);
    void RemoveChild(Node* child);
    void RemoveChild(int32_t index);

    int32_t GetChildIndex(const char* childName);
    Node* GetChild(const char* childName);
    Node* GetChild(int32_t index);
    uint32_t GetNumChildren() const;
    int32_t FindParentNodeIndex() const;

protected:

    std::string mName;

    World* mWorld = nullptr;
    Node* mParent = nullptr;
    std::vector<Node*> mChildren;

    bool mActive = true;
    bool mVisible = true;
    bool mTransient = false;
    bool mDefault = false;

    // Merged from Actor
    SceneRef mSceneSource;
    std::vector<std::string> mTags;
    uint32_t mHitCheckId;

    bool mHasStarted = false;
    bool mPendingDestroy = false;
    bool mTickEnabled = true;
    bool mTransient = false;
    bool mPersistent = false;
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

    NodeScriptData* mScriptData = nullptr;
    //NodeNetData* mNetData = nullptr;

};
