#pragma once

#include "Nodes/Node.h"
#include "AssetRef.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphProcessor.h"

class NodeGraphAsset;

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

    // Output reading (after evaluation)
    float GetOutputFloat(uint32_t pinIndex) const;
    int32_t GetOutputInt(uint32_t pinIndex) const;
    bool GetOutputBool(uint32_t pinIndex) const;
    glm::vec3 GetOutputVector(uint32_t pinIndex) const;
    glm::vec4 GetOutputColor(uint32_t pinIndex) const;

    // Settings
    void SetPlayOnStart(bool v) { mPlayOnStart = v; }
    bool GetPlayOnStart() const { return mPlayOnStart; }

    // Event system
    float GetDeltaTime() const { return mDeltaTime; }
    bool HasStartFired() const { return mStartFired; }

    NodeGraph* GetRuntimeGraph() { return mRuntimeGraph; }

protected:

    void EnsureRuntimeGraph();
    void FireEvent(const char* eventName);

    AssetRef mNodeGraphAsset;
    NodeGraph* mRuntimeGraph = nullptr;
    GraphProcessor mProcessor;
    bool mPlaying = false;
    bool mPaused = false;
    bool mPlayOnStart = false;
    bool mStartFired = false;
    float mDeltaTime = 0.0f;
};
