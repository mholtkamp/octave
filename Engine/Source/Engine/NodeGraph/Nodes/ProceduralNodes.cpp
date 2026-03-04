#include "NodeGraph/Nodes/ProceduralNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(ProceduralNodes);

static const glm::vec4 kProceduralNodeColor = glm::vec4(0.8f, 0.5f, 0.1f, 1.0f);

// =============================================================================
// ProceduralOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(ProceduralOutputNode);

void ProceduralOutputNode::SetupPins()
{
    AddInputPin("Height", DatumType::Float, Datum(0.0f));
    AddInputPin("Density", DatumType::Float, Datum(1.0f));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddInputPin("Mask", DatumType::Float, Datum(1.0f));
}

void ProceduralOutputNode::Evaluate()
{
    // Sink node - collects procedural generation parameters.
}

glm::vec4 ProceduralOutputNode::GetNodeColor() const { return kProceduralNodeColor; }

// =============================================================================
// NoiseNode
// =============================================================================
DEFINE_GRAPH_NODE(NoiseNode);

void NoiseNode::SetupPins()
{
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scale", DatumType::Float, Datum(1.0f));
    AddInputPin("Octaves", DatumType::Integer, Datum(4));
    AddOutputPin("Value", DatumType::Float);
}

void NoiseNode::Evaluate()
{
    glm::vec3 pos = GetInputValue(0).GetVector();
    float scale = GetInputValue(1).GetFloat();

    // Simple hash-based noise placeholder
    glm::vec3 scaled = pos * scale;
    float value = glm::fract(glm::sin(glm::dot(scaled, glm::vec3(12.9898f, 78.233f, 45.164f))) * 43758.5453f);
    SetOutputValue(0, Datum(value));
}

glm::vec4 NoiseNode::GetNodeColor() const { return kProceduralNodeColor; }

// =============================================================================
// VoronoiNode
// =============================================================================
DEFINE_GRAPH_NODE(VoronoiNode);

void VoronoiNode::SetupPins()
{
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scale", DatumType::Float, Datum(1.0f));
    AddOutputPin("Distance", DatumType::Float);
    AddOutputPin("Cell ID", DatumType::Integer);
}

void VoronoiNode::Evaluate()
{
    glm::vec3 pos = GetInputValue(0).GetVector();
    float scale = GetInputValue(1).GetFloat();

    // Simple voronoi placeholder - returns distance to nearest cell center
    glm::vec3 scaled = pos * scale;
    glm::vec3 cell = glm::floor(scaled);
    glm::vec3 frac = glm::fract(scaled);
    float dist = glm::length(frac - glm::vec3(0.5f));
    int32_t cellId = (int32_t)(cell.x * 73856093.0f + cell.y * 19349663.0f + cell.z * 83492791.0f) % 1000;

    SetOutputValue(0, Datum(dist));
    SetOutputValue(1, Datum(cellId));
}

glm::vec4 VoronoiNode::GetNodeColor() const { return kProceduralNodeColor; }

// =============================================================================
// GradientNode
// =============================================================================
DEFINE_GRAPH_NODE(GradientNode);

void GradientNode::SetupPins()
{
    AddInputPin("Position", DatumType::Float, Datum(0.5f));
    AddInputPin("Start", DatumType::Color, Datum(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
    AddInputPin("End", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Color", DatumType::Color);
}

void GradientNode::Evaluate()
{
    float t = glm::clamp(GetInputValue(0).GetFloat(), 0.0f, 1.0f);
    glm::vec4 start = GetInputValue(1).GetColor();
    glm::vec4 end = GetInputValue(2).GetColor();
    SetOutputValue(0, Datum(glm::mix(start, end, t)));
}

glm::vec4 GradientNode::GetNodeColor() const { return kProceduralNodeColor; }
