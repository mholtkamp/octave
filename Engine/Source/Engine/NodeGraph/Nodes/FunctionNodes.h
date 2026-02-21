#pragma once

#include "NodeGraph/GraphNode.h"

#include <string>

class NodeGraphAsset;

static const glm::vec4 kFunctionNodeColor = glm::vec4(0.2f, 0.7f, 0.4f, 1.0f);

// =============================================================================
// FunctionOutputNode
// A sink node for function graphs with dynamic input pins.
// Users add/remove outputs (which are input pins on this node)
// to define what values the function returns.
// =============================================================================
class FunctionOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FunctionOutputNode, GraphNode);

    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Function Output"; }
    virtual const char* GetNodeCategory() const override { return "Function"; }
    virtual glm::vec4 GetNodeColor() const override { return kFunctionNodeColor; }

    virtual void CopyCustomData(const GraphNode* src) override;

    // Dynamic pin management
    void AddOutputField(const std::string& name, DatumType type);
    void RemoveOutputField(uint32_t index);
};

// =============================================================================
// FunctionCallNode
// Calls a named function graph within the same NodeGraphAsset.
// Pins are dynamically built from the function's InputNodes and FunctionOutputNode.
// =============================================================================
class FunctionCallNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FunctionCallNode, GraphNode);

    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override;
    virtual const char* GetNodeCategory() const override { return "Function"; }
    virtual glm::vec4 GetNodeColor() const override { return kFunctionNodeColor; }

    virtual void CopyCustomData(const GraphNode* src) override;

    void SetFunctionName(const std::string& name);
    const std::string& GetFunctionName() const { return mFunctionName; }

    // Rebuild pins from function graph signature
    void RebuildPinsFromFunction(NodeGraphAsset* asset);

    void SetOwnerAsset(NodeGraphAsset* asset) { mOwnerAsset = asset; }
    NodeGraphAsset* GetOwnerAsset() const { return mOwnerAsset; }

protected:
    std::string mFunctionName;
    NodeGraphAsset* mOwnerAsset = nullptr;  // non-owning
};
