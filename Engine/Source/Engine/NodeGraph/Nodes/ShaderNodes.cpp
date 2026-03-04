#include "NodeGraph/Nodes/ShaderNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(ShaderNodes);

static const glm::vec4 kShaderNodeColor = glm::vec4(0.2f, 0.4f, 0.8f, 1.0f);

// =============================================================================
// ShaderOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(ShaderOutputNode);

void ShaderOutputNode::SetupPins()
{
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddInputPin("Alpha", DatumType::Float, Datum(1.0f));
    AddInputPin("Vertex Offset", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("World Normal", DatumType::Vector, Datum(glm::vec3(0.0f, 0.0f, 1.0f)));
}

void ShaderOutputNode::Evaluate()
{
    // Sink node - collects final shader parameters.
}

glm::vec4 ShaderOutputNode::GetNodeColor() const { return kShaderNodeColor; }

// =============================================================================
// VertexPositionNode
// =============================================================================
DEFINE_GRAPH_NODE(VertexPositionNode);

void VertexPositionNode::SetupPins()
{
    AddOutputPin("Position", DatumType::Vector);
}

void VertexPositionNode::Evaluate()
{
    // Placeholder - actual vertex data comes from the rendering pipeline.
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
}

glm::vec4 VertexPositionNode::GetNodeColor() const { return kShaderNodeColor; }

// =============================================================================
// VertexNormalNode
// =============================================================================
DEFINE_GRAPH_NODE(VertexNormalNode);

void VertexNormalNode::SetupPins()
{
    AddOutputPin("Normal", DatumType::Vector);
}

void VertexNormalNode::Evaluate()
{
    // Placeholder - actual vertex normal comes from the rendering pipeline.
    SetOutputValue(0, Datum(glm::vec3(0.0f, 0.0f, 1.0f)));
}

glm::vec4 VertexNormalNode::GetNodeColor() const { return kShaderNodeColor; }

// =============================================================================
// WorldPositionNode
// =============================================================================
DEFINE_GRAPH_NODE(WorldPositionNode);

void WorldPositionNode::SetupPins()
{
    AddOutputPin("Position", DatumType::Vector);
}

void WorldPositionNode::Evaluate()
{
    // Placeholder - actual world position comes from the rendering pipeline.
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
}

glm::vec4 WorldPositionNode::GetNodeColor() const { return kShaderNodeColor; }
