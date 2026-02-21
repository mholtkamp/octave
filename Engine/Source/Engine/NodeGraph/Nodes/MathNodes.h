#pragma once

#include "NodeGraph/GraphNode.h"

// --- Add ---
class AddNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AddNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Add"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Subtract ---
class SubtractNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SubtractNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Subtract"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Multiply ---
class MultiplyNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MultiplyNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Multiply"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Divide ---
class DivideNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DivideNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Divide"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Lerp ---
class LerpNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(LerpNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Lerp"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Clamp ---
class ClampNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ClampNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Clamp"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Abs ---
class AbsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AbsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Abs"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Dot Product ---
class DotProductNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DotProductNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Dot Product"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};
