#pragma once

#include "NodeGraph/GraphNode.h"

// --- Shader Output ---
class ShaderOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ShaderOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Shader Output"; }
    virtual const char* GetNodeCategory() const override { return "Shader"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vertex Position ---
class VertexPositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VertexPositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vertex Position"; }
    virtual const char* GetNodeCategory() const override { return "Shader"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vertex Normal ---
class VertexNormalNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VertexNormalNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vertex Normal"; }
    virtual const char* GetNodeCategory() const override { return "Shader"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- World Position ---
class WorldPositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(WorldPositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "World Position"; }
    virtual const char* GetNodeCategory() const override { return "Shader"; }
    virtual glm::vec4 GetNodeColor() const override;
};
