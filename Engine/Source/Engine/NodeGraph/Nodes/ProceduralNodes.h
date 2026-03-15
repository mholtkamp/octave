#pragma once

#include "NodeGraph/GraphNode.h"

// --- Procedural Output ---
class ProceduralOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ProceduralOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Procedural Output"; }
    virtual const char* GetNodeCategory() const override { return "Procedural"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Noise ---
class NoiseNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NoiseNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Noise"; }
    virtual const char* GetNodeCategory() const override { return "Procedural"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Voronoi ---
class VoronoiNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VoronoiNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Voronoi"; }
    virtual const char* GetNodeCategory() const override { return "Procedural"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Gradient ---
class GradientNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GradientNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Gradient"; }
    virtual const char* GetNodeCategory() const override { return "Procedural"; }
    virtual glm::vec4 GetNodeColor() const override;
};
