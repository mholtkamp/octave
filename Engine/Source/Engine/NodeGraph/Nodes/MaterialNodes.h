#pragma once

#include "NodeGraph/GraphNode.h"

// --- Material Output ---
class MaterialOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MaterialOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Material Output"; }
    virtual const char* GetNodeCategory() const override { return "Material"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Texture Sample ---
class TextureSampleNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TextureSampleNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Texture Sample"; }
    virtual const char* GetNodeCategory() const override { return "Material"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Fresnel ---
class FresnelNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FresnelNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Fresnel"; }
    virtual const char* GetNodeCategory() const override { return "Material"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Panner ---
class PannerNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PannerNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Panner"; }
    virtual const char* GetNodeCategory() const override { return "Material"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Normal Map ---
class NormalMapNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NormalMapNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Normal Map"; }
    virtual const char* GetNodeCategory() const override { return "Material"; }
    virtual glm::vec4 GetNodeColor() const override;
};
