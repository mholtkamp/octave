#pragma once

#include "Nodes/Node.h"
#include "AssetRef.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphProcessor.h"

#include <vector>
#include <map>

class NodeGraphAsset;
class Primitive3D;

struct OverlapEventData
{
    Node* mOtherNode = nullptr;
};

struct CollisionEventData
{
    Node* mOtherNode = nullptr;
    glm::vec3 mHitPosition = glm::vec3(0.0f);
    glm::vec3 mHitNormal = glm::vec3(0.0f);
};

struct CollisionPairKey
{
    Node* mThisComp = nullptr;
    Node* mOtherComp = nullptr;

    bool operator<(const CollisionPairKey& other) const
    {
        if (mThisComp != other.mThisComp)
            return mThisComp < other.mThisComp;
        return mOtherComp < other.mOtherComp;
    }
};

class NodeGraphPlayer : public Node
{
public:

    DECLARE_NODE(NodeGraphPlayer, Node);

    NodeGraphPlayer();
    virtual ~NodeGraphPlayer();

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Tick(float deltaTime) override;
    virtual void Start() override;
    virtual void Stop() override;

    // Physics event overrides
    virtual void BeginOverlap(Primitive3D* thisComp, Primitive3D* otherComp) override;
    virtual void EndOverlap(Primitive3D* thisComp, Primitive3D* otherComp) override;
    virtual void OnCollision(
        Primitive3D* thisComp,
        Primitive3D* otherComp,
        glm::vec3 impactPoint,
        glm::vec3 impactNormal,
        btPersistentManifold* manifold) override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual const char* GetTypeName() const override;

    // Playback API
    void Play();
    void Pause();
    void StopGraph();
    void Reset();
    bool IsPlaying() const;
    bool IsPaused() const;

    // Asset
    void SetNodeGraphAsset(NodeGraphAsset* asset);
    NodeGraphAsset* GetNodeGraphAsset() const;

    // Input setters (delegate to mRuntimeGraph)
    bool SetInputFloat(const char* name, float value);
    bool SetInputInt(const char* name, int32_t value);
    bool SetInputBool(const char* name, bool value);
    bool SetInputString(const char* name, const char* value);
    bool SetInputVector(const char* name, const glm::vec3& value);
    bool SetInputColor(const char* name, const glm::vec4& value);
    bool SetInputByte(const char* name, uint8_t value);
    bool SetInputAsset(const char* name, Asset* asset);
    bool SetInputVector2D(const char* name, const glm::vec2& value);
    bool SetInputShort(const char* name, int16_t value);
    bool SetInputNode(const char* name, Node* value);
    bool SetInputNode3D(const char* name, Node* value);
    bool SetInputWidget(const char* name, Node* value);
    bool SetInputText(const char* name, Node* value);
    bool SetInputQuad(const char* name, Node* value);
    bool SetInputAudio3D(const char* name, Node* value);
    bool SetInputScene(const char* name, Asset* asset);
    bool SetInputSpline3D(const char* name, Node* value);

    // Output reading (after evaluation)
    float GetOutputFloat(uint32_t pinIndex) const;
    int32_t GetOutputInt(uint32_t pinIndex) const;
    bool GetOutputBool(uint32_t pinIndex) const;
    std::string GetOutputString(uint32_t pinIndex) const;
    glm::vec2 GetOutputVector2D(uint32_t pinIndex) const;
    glm::vec3 GetOutputVector(uint32_t pinIndex) const;
    glm::vec4 GetOutputColor(uint32_t pinIndex) const;
    uint8_t GetOutputByte(uint32_t pinIndex) const;
    int16_t GetOutputShort(uint32_t pinIndex) const;
    Asset* GetOutputAsset(uint32_t pinIndex) const;
    Node* GetOutputNode(uint32_t pinIndex) const;

    // Settings
    void SetPlayOnStart(bool v) { mPlayOnStart = v; }
    bool GetPlayOnStart() const { return mPlayOnStart; }

    // Event system
    float GetDeltaTime() const { return mDeltaTime; }
    bool HasStartFired() const { return mStartFired; }

    // Physics event accessors (used by event graph nodes during Evaluate)
    const OverlapEventData& GetCurrentOverlapEventData() const { return mCurrentOverlapEventData; }
    const CollisionEventData& GetCurrentCollisionEventData() const { return mCurrentCollisionEventData; }

    NodeGraph* GetRuntimeGraph() { return mRuntimeGraph; }

protected:

    void EnsureRuntimeGraph();
    void FireEvent(const char* eventName);
    void ProcessPhysicsEvents();

    AssetRef mNodeGraphAsset;
    NodeGraph* mRuntimeGraph = nullptr;
    GraphProcessor mProcessor;
    bool mPlaying = false;
    bool mPaused = false;
    bool mPlayOnStart = false;
    bool mStartFired = false;
    float mDeltaTime = 0.0f;

    // Physics event queues
    std::vector<OverlapEventData> mBeginOverlapQueue;
    std::vector<OverlapEventData> mEndOverlapQueue;
    std::map<CollisionPairKey, CollisionEventData> mCurrentCollisions;
    std::map<CollisionPairKey, CollisionEventData> mPreviousCollisions;

    // Current event context (set during ProcessPhysicsEvents, read by event graph nodes)
    OverlapEventData mCurrentOverlapEventData;
    CollisionEventData mCurrentCollisionEventData;
};
