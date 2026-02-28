#pragma once

#include "NodeGraph/GraphNode.h"

#include <string>

class NodeGraphAsset;

static const glm::vec4 kVariableNodeColor = glm::vec4(0.3f, 0.6f, 0.9f, 1.0f);

// =============================================================================
// GetVariableNode
// Pure data node that reads a named variable's runtime value.
// =============================================================================
class GetVariableNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetVariableNode, GraphNode);

    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override;
    virtual const char* GetNodeCategory() const override { return "Variable"; }
    virtual glm::vec4 GetNodeColor() const override { return kVariableNodeColor; }

    virtual void CopyCustomData(const GraphNode* src) override;

    void RebuildPinFromVariable(NodeGraphAsset* asset);

    void SetVariableName(const std::string& name) { mVariableName = name; }
    const std::string& GetVariableName() const { return mVariableName; }

    void SetOwnerAsset(NodeGraphAsset* asset) { mOwnerAsset = asset; }
    NodeGraphAsset* GetOwnerAsset() const { return mOwnerAsset; }

    DatumType GetCachedType() const { return mCachedType; }

protected:
    std::string mVariableName;
    NodeGraphAsset* mOwnerAsset = nullptr;  // non-owning
    DatumType mCachedType = DatumType::Float;
    mutable std::string mDisplayName;
};

// =============================================================================
// SetVariableNode
// Flow node that writes a value to a named variable's runtime value.
// =============================================================================
class SetVariableNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetVariableNode, GraphNode);

    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override;
    virtual const char* GetNodeCategory() const override { return "Variable"; }
    virtual glm::vec4 GetNodeColor() const override { return kVariableNodeColor; }
    virtual bool IsFlowNode() const override { return true; }

    virtual void CopyCustomData(const GraphNode* src) override;

    void RebuildPinsFromVariable(NodeGraphAsset* asset);

    void SetVariableName(const std::string& name) { mVariableName = name; }
    const std::string& GetVariableName() const { return mVariableName; }

    void SetOwnerAsset(NodeGraphAsset* asset) { mOwnerAsset = asset; }
    NodeGraphAsset* GetOwnerAsset() const { return mOwnerAsset; }

    DatumType GetCachedType() const { return mCachedType; }

protected:
    std::string mVariableName;
    NodeGraphAsset* mOwnerAsset = nullptr;  // non-owning
    DatumType mCachedType = DatumType::Float;
    mutable std::string mDisplayName;
};
