#include "NodeGraph/Nodes/GizmoNodes.h"
#include "Gizmos.h"
#include "Nodes/3D/Node3d.h"

FORCE_LINK_DEF(GizmoNodes);

static const glm::vec4 kGizmoNodeColor = glm::vec4(0.85f, 0.4f, 0.9f, 1.0f);

// =============================================================================
// GizmoSetColorNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoSetColorNode);

void GizmoSetColorNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoSetColorNode::Evaluate()
{
    glm::vec4 color = GetInputValue(1).GetColor();
    Gizmos::SetColor(color);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoSetColorNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoSetMatrixNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoSetMatrixNode);

void GizmoSetMatrixNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoSetMatrixNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            Gizmos::SetMatrix(node3d->GetTransform());
        }
    }
    else
    {
        Gizmos::SetMatrix(glm::mat4(1.0f));
    }
    TriggerExecutionPin(0);
}

glm::vec4 GizmoSetMatrixNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoResetStateNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoResetStateNode);

void GizmoResetStateNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoResetStateNode::Evaluate()
{
    Gizmos::ResetState();
    TriggerExecutionPin(0);
}

glm::vec4 GizmoResetStateNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoDrawCubeNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoDrawCubeNode);

void GizmoDrawCubeNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Center", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Size", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoDrawCubeNode::Evaluate()
{
    glm::vec3 center = GetInputValue(1).GetVector();
    glm::vec3 size = GetInputValue(2).GetVector();
    Gizmos::DrawCube(center, size);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoDrawCubeNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoDrawWireCubeNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoDrawWireCubeNode);

void GizmoDrawWireCubeNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Center", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Size", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoDrawWireCubeNode::Evaluate()
{
    glm::vec3 center = GetInputValue(1).GetVector();
    glm::vec3 size = GetInputValue(2).GetVector();
    Gizmos::DrawWireCube(center, size);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoDrawWireCubeNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoDrawSphereNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoDrawSphereNode);

void GizmoDrawSphereNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Center", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Radius", DatumType::Float, Datum(1.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoDrawSphereNode::Evaluate()
{
    glm::vec3 center = GetInputValue(1).GetVector();
    float radius = GetInputValue(2).GetFloat();
    Gizmos::DrawSphere(center, radius);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoDrawSphereNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoDrawWireSphereNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoDrawWireSphereNode);

void GizmoDrawWireSphereNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Center", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Radius", DatumType::Float, Datum(1.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoDrawWireSphereNode::Evaluate()
{
    glm::vec3 center = GetInputValue(1).GetVector();
    float radius = GetInputValue(2).GetFloat();
    Gizmos::DrawWireSphere(center, radius);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoDrawWireSphereNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoDrawLineNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoDrawLineNode);

void GizmoDrawLineNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("From", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("To", DatumType::Vector, Datum(glm::vec3(0.0f, 1.0f, 0.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoDrawLineNode::Evaluate()
{
    glm::vec3 from = GetInputValue(1).GetVector();
    glm::vec3 to = GetInputValue(2).GetVector();
    Gizmos::DrawLine(from, to);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoDrawLineNode::GetNodeColor() const { return kGizmoNodeColor; }

// =============================================================================
// GizmoDrawRayNode
// =============================================================================
DEFINE_GRAPH_NODE(GizmoDrawRayNode);

void GizmoDrawRayNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Origin", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Direction", DatumType::Vector, Datum(glm::vec3(0.0f, 1.0f, 0.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void GizmoDrawRayNode::Evaluate()
{
    glm::vec3 origin = GetInputValue(1).GetVector();
    glm::vec3 direction = GetInputValue(2).GetVector();
    Gizmos::DrawRay(origin, direction);
    TriggerExecutionPin(0);
}

glm::vec4 GizmoDrawRayNode::GetNodeColor() const { return kGizmoNodeColor; }
