#pragma once

#include "NodeGraph/GraphNode.h"

// --- Scene Graph Output ---
class SceneGraphOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SceneGraphOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Scene Graph Output"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Get Transform ---
class GetTransformNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetTransformNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Transform"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Set Transform ---
class SetTransformNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetTransformNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Set Transform"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Find Node ---
class FindNodeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindNodeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find Node"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};
