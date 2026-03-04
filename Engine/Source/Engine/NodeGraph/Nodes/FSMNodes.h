#pragma once

#include "NodeGraph/GraphNode.h"

// --- FSM Output ---
class FSMOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FSMOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "FSM Output"; }
    virtual const char* GetNodeCategory() const override { return "FSM"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- State ---
class StateNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StateNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "State"; }
    virtual const char* GetNodeCategory() const override { return "FSM"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Transition ---
class TransitionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TransitionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Transition"; }
    virtual const char* GetNodeCategory() const override { return "FSM"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Condition ---
class ConditionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ConditionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Condition"; }
    virtual const char* GetNodeCategory() const override { return "FSM"; }
    virtual glm::vec4 GetNodeColor() const override;
};
