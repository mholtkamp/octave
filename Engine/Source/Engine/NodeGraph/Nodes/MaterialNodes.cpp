#include "NodeGraph/Nodes/MaterialNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(MaterialNodes);

static const glm::vec4 kMaterialNodeColor = glm::vec4(0.7f, 0.2f, 0.2f, 1.0f);

// =============================================================================
// MaterialOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(MaterialOutputNode);

void MaterialOutputNode::SetupPins()
{
    // Match MaterialLite parameters 1:1
    AddInputPin("Color",          DatumType::Color,    Datum(glm::vec4(1.0f)));           // pin 0
    AddInputPin("Opacity",        DatumType::Float,    Datum(1.0f));                      // pin 1
    AddInputPin("Mask Cutoff",    DatumType::Float,    Datum(0.5f));                      // pin 2
    AddInputPin("Emission",       DatumType::Float,    Datum(0.0f));                      // pin 3
    AddInputPin("Specular",       DatumType::Float,    Datum(0.0f));                      // pin 4
    AddInputPin("Shininess",      DatumType::Float,    Datum(32.0f));                     // pin 5
    AddInputPin("Wrap Lighting",  DatumType::Float,    Datum(0.0f));                      // pin 6
    AddInputPin("Fresnel Color",  DatumType::Color,    Datum(glm::vec4(1, 0, 0, 0)));     // pin 7
    AddInputPin("Fresnel Power",  DatumType::Float,    Datum(1.0f));                      // pin 8
    AddInputPin("UV Offset 0",    DatumType::Vector2D, Datum(glm::vec2(0.0f)));           // pin 9
    AddInputPin("UV Scale 0",     DatumType::Vector2D, Datum(glm::vec2(1.0f)));           // pin 10
    AddInputPin("UV Offset 1",    DatumType::Vector2D, Datum(glm::vec2(0.0f)));           // pin 11
    AddInputPin("UV Scale 1",     DatumType::Vector2D, Datum(glm::vec2(1.0f)));           // pin 12
}

void MaterialOutputNode::Evaluate()
{
    // Material output is a sink node - it just collects values.
    // The domain reads the input pin values after evaluation.
}

glm::vec4 MaterialOutputNode::GetNodeColor() const { return kMaterialNodeColor; }

// =============================================================================
// TextureSampleNode
// =============================================================================
DEFINE_GRAPH_NODE(TextureSampleNode);

void TextureSampleNode::SetupPins()
{
    AddInputPin("UV", DatumType::Vector2D, Datum(glm::vec2(0.0f)));
    AddOutputPin("Color", DatumType::Color);
    AddOutputPin("R", DatumType::Float);
    AddOutputPin("G", DatumType::Float);
    AddOutputPin("B", DatumType::Float);
    AddOutputPin("A", DatumType::Float);
}

void TextureSampleNode::Evaluate()
{
    // Placeholder - actual texture sampling requires GPU/runtime integration.
    // For now, output white.
    glm::vec4 color = glm::vec4(1.0f);
    SetOutputValue(0, Datum(color));
    SetOutputValue(1, Datum(color.r));
    SetOutputValue(2, Datum(color.g));
    SetOutputValue(3, Datum(color.b));
    SetOutputValue(4, Datum(color.a));
}

glm::vec4 TextureSampleNode::GetNodeColor() const { return kMaterialNodeColor; }

// =============================================================================
// FresnelNode
// =============================================================================
DEFINE_GRAPH_NODE(FresnelNode);

void FresnelNode::SetupPins()
{
    AddInputPin("Exponent", DatumType::Float, Datum(5.0f));
    AddInputPin("Base Value", DatumType::Float, Datum(0.04f));
    AddOutputPin("Result", DatumType::Float);
}

void FresnelNode::Evaluate()
{
    // Placeholder fresnel approximation
    float exponent = GetInputValue(0).GetFloat();
    float baseValue = GetInputValue(1).GetFloat();
    // Without actual view/normal vectors, output base value
    SetOutputValue(0, Datum(baseValue));
}

glm::vec4 FresnelNode::GetNodeColor() const { return kMaterialNodeColor; }

// =============================================================================
// PannerNode
// =============================================================================
DEFINE_GRAPH_NODE(PannerNode);

void PannerNode::SetupPins()
{
    AddInputPin("UV", DatumType::Vector2D, Datum(glm::vec2(0.0f)));
    AddInputPin("Speed X", DatumType::Float, Datum(1.0f));
    AddInputPin("Speed Y", DatumType::Float, Datum(0.0f));
    AddInputPin("Time", DatumType::Float, Datum(0.0f));
    AddOutputPin("UV", DatumType::Vector2D);
}

void PannerNode::Evaluate()
{
    glm::vec2 uv = GetInputValue(0).GetVector2D();
    float speedX = GetInputValue(1).GetFloat();
    float speedY = GetInputValue(2).GetFloat();
    float time = GetInputValue(3).GetFloat();

    glm::vec2 result = uv + glm::vec2(speedX, speedY) * time;
    SetOutputValue(0, Datum(result));
}

glm::vec4 PannerNode::GetNodeColor() const { return kMaterialNodeColor; }

// =============================================================================
// NormalMapNode
// =============================================================================
DEFINE_GRAPH_NODE(NormalMapNode);

void NormalMapNode::SetupPins()
{
    AddInputPin("Normal", DatumType::Vector, Datum(glm::vec3(0.5f, 0.5f, 1.0f)));
    AddInputPin("Strength", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Vector);
}

void NormalMapNode::Evaluate()
{
    glm::vec3 normal = GetInputValue(0).GetVector();
    float strength = GetInputValue(1).GetFloat();

    // Unpack from 0..1 range to -1..1 range
    glm::vec3 unpacked = normal * 2.0f - 1.0f;
    unpacked.x *= strength;
    unpacked.y *= strength;
    unpacked = glm::normalize(unpacked);

    SetOutputValue(0, Datum(unpacked));
}

glm::vec4 NormalMapNode::GetNodeColor() const { return kMaterialNodeColor; }
