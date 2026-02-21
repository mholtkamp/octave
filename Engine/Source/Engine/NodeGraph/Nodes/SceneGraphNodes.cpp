#include "NodeGraph/Nodes/SceneGraphNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(SceneGraphNodes);

static const glm::vec4 kSceneGraphNodeColor = glm::vec4(0.1f, 0.6f, 0.6f, 1.0f);

// =============================================================================
// SceneGraphOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(SceneGraphOutputNode);

void SceneGraphOutputNode::SetupPins()
{
    AddInputPin("Transform", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Active", DatumType::Bool, Datum(true));
}

void SceneGraphOutputNode::Evaluate()
{
    // Sink node - collects scene graph output parameters.
}

glm::vec4 SceneGraphOutputNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetTransformNode
// =============================================================================
DEFINE_GRAPH_NODE(GetTransformNode);

void GetTransformNode::SetupPins()
{
    AddOutputPin("Position", DatumType::Vector);
    AddOutputPin("Rotation", DatumType::Vector);
    AddOutputPin("Scale", DatumType::Vector);
}

void GetTransformNode::Evaluate()
{
    // Placeholder - actual transform data comes from the scene graph.
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
    SetOutputValue(1, Datum(glm::vec3(0.0f)));
    SetOutputValue(2, Datum(glm::vec3(1.0f)));
}

glm::vec4 GetTransformNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetTransformNode
// =============================================================================
DEFINE_GRAPH_NODE(SetTransformNode);

void SetTransformNode::SetupPins()
{
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Rotation", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scale", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddOutputPin("Transform", DatumType::Vector);
}

void SetTransformNode::Evaluate()
{
    glm::vec3 pos = GetInputValue(0).GetVector();
    SetOutputValue(0, Datum(pos));
}

glm::vec4 SetTransformNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// FindNodeNode
// =============================================================================
DEFINE_GRAPH_NODE(FindNodeNode);

void FindNodeNode::SetupPins()
{
    AddOutputPin("Found", DatumType::Bool);
}

void FindNodeNode::Evaluate()
{
    // Placeholder - actual node search comes from the scene graph system.
    SetOutputValue(0, Datum(false));
}

glm::vec4 FindNodeNode::GetNodeColor() const { return kSceneGraphNodeColor; }
