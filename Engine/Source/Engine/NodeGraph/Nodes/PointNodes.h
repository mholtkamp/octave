#pragma once

#include "NodeGraph/GraphNode.h"

// =============================================================================
// Point Generation Nodes
// =============================================================================

class CreatePointsGridNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CreatePointsGridNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Create Points Grid"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class ScatterPointsOnMeshNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ScatterPointsOnMeshNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Scatter Points On Mesh"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class PointsFromSplineNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PointsFromSplineNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Points From Spline"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class CreatePointsLineNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CreatePointsLineNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Create Points Line"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class RandomPointsInBoxNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomPointsInBoxNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Random Points In Box"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class MergePointsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MergePointsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Merge Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Point Attribute Nodes
// =============================================================================

class SetPointAttributeFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPointAttributeFloatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Set Point Attribute (Float)"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetPointAttributeVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPointAttributeVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Set Point Attribute (Vector)"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetPointAttributeColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPointAttributeColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Set Point Attribute (Color)"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetPointAttributeFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetPointAttributeFloatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Point Attribute (Float)"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetPointAttributeVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetPointAttributeVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Point Attribute (Vector)"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetPointAttributeColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetPointAttributeColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Point Attribute (Color)"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetPointCountNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetPointCountNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Point Count"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetPointPositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetPointPositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Point Position"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetPointPositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPointPositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Set Point Position"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Point Iteration / Transform Nodes
// =============================================================================

class ForEachPointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ForEachPointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "For Each Point"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetPointInLoopNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPointInLoopNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Point In Loop"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class FilterPointsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FilterPointsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Filter Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class TransformPointsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TransformPointsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Transform Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class RandomizeAttributeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomizeAttributeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Randomize Attribute"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Point Output / Bridge Nodes
// =============================================================================

class CopyToPointsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CopyToPointsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Copy To Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class InstantiateAtPointsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InstantiateAtPointsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Instantiate At Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class PointCloudToProceduralOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PointCloudToProceduralOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Points To Output"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Point Utility Nodes
// =============================================================================

class PointCloudFirstPointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PointCloudFirstPointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "First Point"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class PointCloudLastPointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PointCloudLastPointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Last Point"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class RemovePointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RemovePointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Remove Point"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class ReversePointsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ReversePointsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Reverse Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SortPointsByAttributeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SortPointsByAttributeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Sort Points"; }
    virtual const char* GetNodeCategory() const override { return "Point"; }
    virtual glm::vec4 GetNodeColor() const override;
};
