#include "NodeGraph/Nodes/MathNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(MathNodes);

static const glm::vec4 kMathNodeColor = glm::vec4(0.4f, 0.6f, 0.2f, 1.0f);

// =============================================================================
// AddNode
// =============================================================================
DEFINE_GRAPH_NODE(AddNode);

void AddNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void AddNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a + b));
}

glm::vec4 AddNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// SubtractNode
// =============================================================================
DEFINE_GRAPH_NODE(SubtractNode);

void SubtractNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void SubtractNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a - b));
}

glm::vec4 SubtractNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// MultiplyNode
// =============================================================================
DEFINE_GRAPH_NODE(MultiplyNode);

void MultiplyNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void MultiplyNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a * b));
}

glm::vec4 MultiplyNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// DivideNode
// =============================================================================
DEFINE_GRAPH_NODE(DivideNode);

void DivideNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void DivideNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    if (b != 0.0f)
    {
        SetOutputValue(0, Datum(a / b));
    }
    else
    {
        SetOutputValue(0, Datum(0.0f));
    }
}

glm::vec4 DivideNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// LerpNode
// =============================================================================
DEFINE_GRAPH_NODE(LerpNode);

void LerpNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddInputPin("Alpha", DatumType::Float, Datum(0.5f));
    AddOutputPin("Result", DatumType::Float);
}

void LerpNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    float alpha = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::mix(a, b, alpha)));
}

glm::vec4 LerpNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ClampNode
// =============================================================================
DEFINE_GRAPH_NODE(ClampNode);

void ClampNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddInputPin("Min", DatumType::Float, Datum(0.0f));
    AddInputPin("Max", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void ClampNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    float minVal = GetInputValue(1).GetFloat();
    float maxVal = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::clamp(value, minVal, maxVal)));
}

glm::vec4 ClampNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// AbsNode
// =============================================================================
DEFINE_GRAPH_NODE(AbsNode);

void AbsNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void AbsNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::abs(value)));
}

glm::vec4 AbsNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// DotProductNode
// =============================================================================
DEFINE_GRAPH_NODE(DotProductNode);

void DotProductNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Result", DatumType::Float);
}

void DotProductNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    SetOutputValue(0, Datum(glm::dot(a, b)));
}

glm::vec4 DotProductNode::GetNodeColor() const { return kMathNodeColor; }
