#include "NodeGraph/Nodes/ValueNodes.h"
#include "Utilities.h"
#include "Clock.h"
#include "Engine.h"
#include "Log.h"

FORCE_LINK_DEF(ValueNodes);

static const glm::vec4 kValueNodeColor = glm::vec4(0.2f, 0.5f, 0.7f, 1.0f);

// =============================================================================
// FloatConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(FloatConstantNode);

void FloatConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Out", DatumType::Float);
}

void FloatConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 FloatConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// IntConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(IntConstantNode);

void IntConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Integer, Datum(0));
    AddOutputPin("Out", DatumType::Integer);
}

void IntConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 IntConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// VectorConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(VectorConstantNode);

void VectorConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Out", DatumType::Vector);
}

void VectorConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 VectorConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// ColorConstantNode
// =============================================================================
DEFINE_GRAPH_NODE(ColorConstantNode);

void ColorConstantNode::SetupPins()
{
    AddInputPin("Value", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Out", DatumType::Color);
}

void ColorConstantNode::Evaluate()
{
    SetOutputValue(0, GetInputValue(0));
}

glm::vec4 ColorConstantNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// TimeNode
// =============================================================================
DEFINE_GRAPH_NODE(TimeNode);

void TimeNode::SetupPins()
{
    AddOutputPin("Time", DatumType::Float);
}

void TimeNode::Evaluate()
{
    float time = GetAppClock() ? GetAppClock()->GetTime() : 0.0f;
    SetOutputValue(0, Datum(time));
}

glm::vec4 TimeNode::GetNodeColor() const { return kValueNodeColor; }

// =============================================================================
// ViewerNode
// =============================================================================
DEFINE_GRAPH_NODE(ViewerNode);

void ViewerNode::SetupPins()
{
    AddInputPin("Float", DatumType::Float, Datum(0.0f));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(0.0f)));
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Integer", DatumType::Integer, Datum(0));
}

void ViewerNode::Evaluate()
{
    // Viewer is a display-only sink node.
    // Input values are propagated by the processor and shown in the editor.
}

glm::vec4 ViewerNode::GetNodeColor() const { return glm::vec4(0.2f, 0.6f, 0.3f, 1.0f); }

// =============================================================================
// DebugLogNode
// =============================================================================
DEFINE_GRAPH_NODE(DebugLogNode);

void DebugLogNode::SetupPins()
{
    AddInputPin("Label", DatumType::String, Datum(std::string("Debug")));
    AddInputPin("Float", DatumType::Float, Datum(0.0f));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(0.0f)));
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Integer", DatumType::Integer, Datum(0));
    AddInputPin("String", DatumType::String, Datum(std::string("")));
}

void DebugLogNode::Evaluate()
{
    const std::string& label = GetInputValue(0).GetString();

    float f = GetInputValue(1).GetFloat();
    glm::vec4 c = GetInputValue(2).GetColor();
    glm::vec3 v = GetInputValue(3).GetVector();
    int32_t i = GetInputValue(4).GetInteger();
    const std::string& s = GetInputValue(5).GetString();

    LogDebug("[%s] Float=%.4f Int=%d Vec=(%.2f, %.2f, %.2f) Color=(%.2f, %.2f, %.2f, %.2f) Str=%s",
        label.c_str(),
        f, i,
        v.x, v.y, v.z,
        c.r, c.g, c.b, c.a,
        s.c_str());
}

glm::vec4 DebugLogNode::GetNodeColor() const { return glm::vec4(0.2f, 0.6f, 0.3f, 1.0f); }
