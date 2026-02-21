#pragma once

#include "NodeGraph/GraphNode.h"

// --- Float Constant ---
class FloatConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FloatConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Float"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Int Constant ---
class IntConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IntConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Integer"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vector Constant ---
class VectorConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vector"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Color Constant ---
class ColorConstantNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ColorConstantNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Color"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Time ---
class TimeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TimeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Time"; }
    virtual const char* GetNodeCategory() const override { return "Value"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Viewer ---
class ViewerNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ViewerNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Viewer"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Debug Log ---
class DebugLogNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DebugLogNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Debug Log"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};
