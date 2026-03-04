#pragma once

#include "NodeGraph/GraphNode.h"

// =============================================================================
// Spline Query Nodes
// =============================================================================

class SplinePositionAtNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplinePositionAtNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Spline Position At"; }
    virtual const char* GetNodeCategory() const override { return "Spline"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SplineTangentAtNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplineTangentAtNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Spline Tangent At"; }
    virtual const char* GetNodeCategory() const override { return "Spline"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SplinePointCountNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplinePointCountNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Spline Point Count"; }
    virtual const char* GetNodeCategory() const override { return "Spline"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SplineGetPointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplineGetPointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Spline Get Point"; }
    virtual const char* GetNodeCategory() const override { return "Spline"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SplineNearestPercentNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplineNearestPercentNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Spline Nearest Percent"; }
    virtual const char* GetNodeCategory() const override { return "Spline"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SplineLengthNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SplineLengthNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Spline Length"; }
    virtual const char* GetNodeCategory() const override { return "Spline"; }
    virtual glm::vec4 GetNodeColor() const override;
};
