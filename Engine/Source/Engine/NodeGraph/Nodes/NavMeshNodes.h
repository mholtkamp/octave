#pragma once

#include "NodeGraph/GraphNode.h"

// =============================================================================
// NavMesh Query Nodes (Pure Data)
// =============================================================================

class FindNavPathNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindNavPathNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find Nav Path"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class FindRandomNavPointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindRandomNavPointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find Random Nav Point"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class FindClosestNavPointNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindClosestNavPointNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find Closest Nav Point"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class IsAutoNavRebuildNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IsAutoNavRebuildNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Is Auto Nav Rebuild"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// NavMesh Action Nodes (Flow)
// =============================================================================

class BuildNavDataNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(BuildNavDataNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Build Nav Data"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class EnableAutoNavRebuildNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(EnableAutoNavRebuildNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Enable Auto Nav Rebuild"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class InvalidateNavMeshNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InvalidateNavMeshNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Invalidate Nav Mesh"; }
    virtual const char* GetNodeCategory() const override { return "NavMesh"; }
    virtual glm::vec4 GetNodeColor() const override;
};
