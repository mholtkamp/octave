#pragma once

#include "NodeGraph/GraphNode.h"

// --- Animation Output ---
class AnimationOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AnimationOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Animation Output"; }
    virtual const char* GetNodeCategory() const override { return "Animation"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Anim Clip ---
class AnimClipNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AnimClipNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Anim Clip"; }
    virtual const char* GetNodeCategory() const override { return "Animation"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Blend ---
class BlendNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(BlendNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Blend"; }
    virtual const char* GetNodeCategory() const override { return "Animation"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Anim Speed ---
class AnimSpeedNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AnimSpeedNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Anim Speed"; }
    virtual const char* GetNodeCategory() const override { return "Animation"; }
    virtual glm::vec4 GetNodeColor() const override;
};
