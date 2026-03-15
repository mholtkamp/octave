#pragma once

#include "NodeGraph/GraphNode.h"

// --- Set Gizmo Color ---
class GizmoSetColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoSetColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Gizmo Color"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Set Gizmo Matrix ---
class GizmoSetMatrixNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoSetMatrixNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Gizmo Matrix"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Reset Gizmo State ---
class GizmoResetStateNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoResetStateNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Reset Gizmo State"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Draw Cube ---
class GizmoDrawCubeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoDrawCubeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Draw Cube"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Draw Wire Cube ---
class GizmoDrawWireCubeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoDrawWireCubeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Draw Wire Cube"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Draw Sphere ---
class GizmoDrawSphereNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoDrawSphereNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Draw Sphere"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Draw Wire Sphere ---
class GizmoDrawWireSphereNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoDrawWireSphereNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Draw Wire Sphere"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Draw Line ---
class GizmoDrawLineNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoDrawLineNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Draw Line"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Draw Ray ---
class GizmoDrawRayNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GizmoDrawRayNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Draw Ray"; }
    virtual const char* GetNodeCategory() const override { return "Gizmo"; }
    virtual glm::vec4 GetNodeColor() const override;
};
