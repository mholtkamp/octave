#include "NodeGraph/Nodes/MathNodes.h"
#include "Utilities.h"

#include <cstdlib>
#include <cmath>
#include "Maths.h"

FORCE_LINK_DEF(MathNodes);

static const glm::vec4 kMathNodeColor = glm::vec4(0.4f, 0.6f, 0.2f, 1.0f);
static const glm::vec4 kComparisonNodeColor = glm::vec4(0.6f, 0.3f, 0.3f, 1.0f);
static const glm::vec4 kLogicNodeColor = glm::vec4(0.7f, 0.2f, 0.2f, 1.0f);

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


// =============================================================================
// RemapValueNode
// =============================================================================

DEFINE_GRAPH_NODE(RemapValueNode);

void RemapValueNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddInputPin("InMin", DatumType::Float, Datum(0.0f));
    AddInputPin("InMax", DatumType::Float, Datum(1.0f));
    AddInputPin("OutMin", DatumType::Float, Datum(0.0f));
    AddInputPin("OutMax", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void RemapValueNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    float inMin = GetInputValue(1).GetFloat();
    float inMax = GetInputValue(2).GetFloat();
    float outMin = GetInputValue(3).GetFloat();
    float outMax = GetInputValue(4).GetFloat();
    if (inMax != inMin)
    {
        float t = (value - inMin) / (inMax - inMin);
        float remappedValue = outMin + t * (outMax - outMin);
        SetOutputValue(0, Datum(remappedValue));
    }
    else
    {
        SetOutputValue(0, Datum(outMin));
    }
}
glm::vec4 RemapValueNode::GetNodeColor() const { return kMathNodeColor; }




// =============================================================================
// SinNode
// =============================================================================

DEFINE_GRAPH_NODE(SinNode);

void SinNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void SinNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::sin(value)));
}
glm::vec4 SinNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// VectorDistanceNode
// =============================================================================
DEFINE_GRAPH_NODE(VectorDistanceNode);

void VectorDistanceNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Distance", DatumType::Float);
    AddOutputPin("Direction", DatumType::Vector);
}

void VectorDistanceNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    glm::vec3 diff = b - a;
    float dist = glm::length(diff);
    glm::vec3 dir = dist > 0.0001f ? diff / dist : glm::vec3(0.0f);

    SetOutputValue(0, Datum(dist));
    SetOutputValue(1, Datum(dir));
}

glm::vec4 VectorDistanceNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// RandomPointNearVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(RandomPointNearVectorNode);

void RandomPointNearVectorNode::SetupPins()
{
    AddInputPin("Center", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Radius", DatumType::Float, Datum(1.0f));
    AddOutputPin("Position", DatumType::Vector);
}

void RandomPointNearVectorNode::Evaluate()
{
    glm::vec3 center = GetInputValue(0).GetVector();
    float radius = GetInputValue(1).GetFloat();

    float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
    float r = ((float)rand() / RAND_MAX) * radius;
    glm::vec3 result = center;
    result.x += r * cosf(angle);
    result.z += r * sinf(angle);

    SetOutputValue(0, Datum(result));
}

glm::vec4 RandomPointNearVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// CosNode
// =============================================================================
DEFINE_GRAPH_NODE(CosNode);

void CosNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void CosNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::cos(value)));
}

glm::vec4 CosNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// TanNode
// =============================================================================
DEFINE_GRAPH_NODE(TanNode);

void TanNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void TanNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::tan(value)));
}

glm::vec4 TanNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ASinNode
// =============================================================================
DEFINE_GRAPH_NODE(ASinNode);

void ASinNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void ASinNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::asin(glm::clamp(value, -1.0f, 1.0f))));
}

glm::vec4 ASinNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ACosNode
// =============================================================================
DEFINE_GRAPH_NODE(ACosNode);

void ACosNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void ACosNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::acos(glm::clamp(value, -1.0f, 1.0f))));
}

glm::vec4 ACosNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ATanNode
// =============================================================================
DEFINE_GRAPH_NODE(ATanNode);

void ATanNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void ATanNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::atan(value)));
}

glm::vec4 ATanNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ATan2Node
// =============================================================================
DEFINE_GRAPH_NODE(ATan2Node);

void ATan2Node::SetupPins()
{
    AddInputPin("Y", DatumType::Float, Datum(0.0f));
    AddInputPin("X", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void ATan2Node::Evaluate()
{
    float y = GetInputValue(0).GetFloat();
    float x = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(glm::atan(y, x)));
}

glm::vec4 ATan2Node::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// SqrtNode
// =============================================================================
DEFINE_GRAPH_NODE(SqrtNode);

void SqrtNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void SqrtNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::sqrt(glm::max(value, 0.0f))));
}

glm::vec4 SqrtNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// PowerNode
// =============================================================================
DEFINE_GRAPH_NODE(PowerNode);

void PowerNode::SetupPins()
{
    AddInputPin("Base", DatumType::Float, Datum(0.0f));
    AddInputPin("Exponent", DatumType::Float, Datum(2.0f));
    AddOutputPin("Result", DatumType::Float);
}

void PowerNode::Evaluate()
{
    float base = GetInputValue(0).GetFloat();
    float exponent = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(glm::pow(base, exponent)));
}

glm::vec4 PowerNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// FloorNode
// =============================================================================
DEFINE_GRAPH_NODE(FloorNode);

void FloorNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void FloorNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::floor(value)));
}

glm::vec4 FloorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// CeilNode
// =============================================================================
DEFINE_GRAPH_NODE(CeilNode);

void CeilNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void CeilNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::ceil(value)));
}

glm::vec4 CeilNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// RoundNode
// =============================================================================
DEFINE_GRAPH_NODE(RoundNode);

void RoundNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void RoundNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::round(value)));
}

glm::vec4 RoundNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// FmodNode
// =============================================================================
DEFINE_GRAPH_NODE(FmodNode);

void FmodNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void FmodNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    if (b != 0.0f)
    {
        SetOutputValue(0, Datum(fmodf(a, b)));
    }
    else
    {
        SetOutputValue(0, Datum(0.0f));
    }
}

glm::vec4 FmodNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// MinNode
// =============================================================================
DEFINE_GRAPH_NODE(MinNode);

void MinNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void MinNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(glm::min(a, b)));
}

glm::vec4 MinNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// MaxNode
// =============================================================================
DEFINE_GRAPH_NODE(MaxNode);

void MaxNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void MaxNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(glm::max(a, b)));
}

glm::vec4 MaxNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// NegateNode
// =============================================================================
DEFINE_GRAPH_NODE(NegateNode);

void NegateNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void NegateNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(-value));
}

glm::vec4 NegateNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// SignNode
// =============================================================================
DEFINE_GRAPH_NODE(SignNode);

void SignNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void SignNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::sign(value)));
}

glm::vec4 SignNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// FracNode
// =============================================================================
DEFINE_GRAPH_NODE(FracNode);

void FracNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void FracNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(glm::fract(value)));
}

glm::vec4 FracNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// OneMinusNode
// =============================================================================
DEFINE_GRAPH_NODE(OneMinusNode);

void OneMinusNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void OneMinusNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(1.0f - value));
}

glm::vec4 OneMinusNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// CrossProductNode
// =============================================================================
DEFINE_GRAPH_NODE(CrossProductNode);

void CrossProductNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void CrossProductNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    SetOutputValue(0, Datum(glm::cross(a, b)));
}

glm::vec4 CrossProductNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// NormalizeNode
// =============================================================================
DEFINE_GRAPH_NODE(NormalizeNode);

void NormalizeNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void NormalizeNode::Evaluate()
{
    glm::vec3 value = GetInputValue(0).GetVector();
    SetOutputValue(0, Datum(Maths::SafeNormalize(value)));
}

glm::vec4 NormalizeNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// VectorLengthNode
// =============================================================================
DEFINE_GRAPH_NODE(VectorLengthNode);

void VectorLengthNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Length", DatumType::Float);
}

void VectorLengthNode::Evaluate()
{
    glm::vec3 value = GetInputValue(0).GetVector();
    SetOutputValue(0, Datum(glm::length(value)));
}

glm::vec4 VectorLengthNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ReflectNode
// =============================================================================
DEFINE_GRAPH_NODE(ReflectNode);

void ReflectNode::SetupPins()
{
    AddInputPin("Direction", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Normal", DatumType::Vector, Datum(glm::vec3(0.0f, 1.0f, 0.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void ReflectNode::Evaluate()
{
    glm::vec3 direction = GetInputValue(0).GetVector();
    glm::vec3 normal = GetInputValue(1).GetVector();
    SetOutputValue(0, Datum(glm::reflect(direction, Maths::SafeNormalize(normal))));
}

glm::vec4 ReflectNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// RotateVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(RotateVectorNode);

void RotateVectorNode::SetupPins()
{
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(1.0f, 0.0f, 0.0f)));
    AddInputPin("Axis", DatumType::Vector, Datum(glm::vec3(0.0f, 1.0f, 0.0f)));
    AddInputPin("Angle", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Vector);
}

void RotateVectorNode::Evaluate()
{
    glm::vec3 vector = GetInputValue(0).GetVector();
    glm::vec3 axis = GetInputValue(1).GetVector();
    float angle = GetInputValue(2).GetFloat();

    glm::vec3 safeAxis = Maths::SafeNormalize(axis);
    if (glm::length(safeAxis) < 0.0001f)
    {
        SetOutputValue(0, Datum(vector));
        return;
    }

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, safeAxis);
    glm::vec3 result = glm::vec3(rotation * glm::vec4(vector, 0.0f));
    SetOutputValue(0, Datum(result));
}

glm::vec4 RotateVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// AddVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(AddVectorNode);

void AddVectorNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void AddVectorNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    SetOutputValue(0, Datum(a + b));
}

glm::vec4 AddVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// SubtractVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(SubtractVectorNode);

void SubtractVectorNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void SubtractVectorNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    SetOutputValue(0, Datum(a - b));
}

glm::vec4 SubtractVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// MultiplyVectorNode (scalar)
// =============================================================================
DEFINE_GRAPH_NODE(MultiplyVectorNode);

void MultiplyVectorNode::SetupPins()
{
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scalar", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Vector);
}

void MultiplyVectorNode::Evaluate()
{
    glm::vec3 v = GetInputValue(0).GetVector();
    float s = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(v * s));
}

glm::vec4 MultiplyVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// DivideVectorNode (scalar)
// =============================================================================
DEFINE_GRAPH_NODE(DivideVectorNode);

void DivideVectorNode::SetupPins()
{
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scalar", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Vector);
}

void DivideVectorNode::Evaluate()
{
    glm::vec3 v = GetInputValue(0).GetVector();
    float s = GetInputValue(1).GetFloat();
    if (s != 0.0f)
    {
        SetOutputValue(0, Datum(v / s));
    }
    else
    {
        SetOutputValue(0, Datum(glm::vec3(0.0f)));
    }
}

glm::vec4 DivideVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// MultiplyVectorComponentNode (component-wise)
// =============================================================================
DEFINE_GRAPH_NODE(MultiplyVectorComponentNode);

void MultiplyVectorComponentNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void MultiplyVectorComponentNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    SetOutputValue(0, Datum(a * b));
}

glm::vec4 MultiplyVectorComponentNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// LerpVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(LerpVectorNode);

void LerpVectorNode::SetupPins()
{
    AddInputPin("A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("B", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddInputPin("Alpha", DatumType::Float, Datum(0.5f));
    AddOutputPin("Result", DatumType::Vector);
}

void LerpVectorNode::Evaluate()
{
    glm::vec3 a = GetInputValue(0).GetVector();
    glm::vec3 b = GetInputValue(1).GetVector();
    float alpha = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::mix(a, b, alpha)));
}

glm::vec4 LerpVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// NegateVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(NegateVectorNode);

void NegateVectorNode::SetupPins()
{
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Result", DatumType::Vector);
}

void NegateVectorNode::Evaluate()
{
    glm::vec3 value = GetInputValue(0).GetVector();
    SetOutputValue(0, Datum(-value));
}

glm::vec4 NegateVectorNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// SmoothstepNode
// =============================================================================
DEFINE_GRAPH_NODE(SmoothstepNode);

void SmoothstepNode::SetupPins()
{
    AddInputPin("Edge0", DatumType::Float, Datum(0.0f));
    AddInputPin("Edge1", DatumType::Float, Datum(1.0f));
    AddInputPin("X", DatumType::Float, Datum(0.5f));
    AddOutputPin("Result", DatumType::Float);
}

void SmoothstepNode::Evaluate()
{
    float edge0 = GetInputValue(0).GetFloat();
    float edge1 = GetInputValue(1).GetFloat();
    float x = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::smoothstep(edge0, edge1, x)));
}

glm::vec4 SmoothstepNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// InverseLerpNode
// =============================================================================
DEFINE_GRAPH_NODE(InverseLerpNode);

void InverseLerpNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(1.0f));
    AddInputPin("Value", DatumType::Float, Datum(0.5f));
    AddOutputPin("Result", DatumType::Float);
}

void InverseLerpNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    float value = GetInputValue(2).GetFloat();
    if (b != a)
    {
        SetOutputValue(0, Datum((value - a) / (b - a)));
    }
    else
    {
        SetOutputValue(0, Datum(0.0f));
    }
}

glm::vec4 InverseLerpNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// StepNode
// =============================================================================
DEFINE_GRAPH_NODE(StepNode);

void StepNode::SetupPins()
{
    AddInputPin("Edge", DatumType::Float, Datum(0.5f));
    AddInputPin("X", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void StepNode::Evaluate()
{
    float edge = GetInputValue(0).GetFloat();
    float x = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(x < edge ? 0.0f : 1.0f));
}

glm::vec4 StepNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// LogNode
// =============================================================================
DEFINE_GRAPH_NODE(LogNode);

void LogNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void LogNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    if (value > 0.0f)
    {
        SetOutputValue(0, Datum(logf(value)));
    }
    else
    {
        SetOutputValue(0, Datum(0.0f));
    }
}

glm::vec4 LogNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// Log2Node
// =============================================================================
DEFINE_GRAPH_NODE(Log2Node);

void Log2Node::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(1.0f));
    AddOutputPin("Result", DatumType::Float);
}

void Log2Node::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    if (value > 0.0f)
    {
        SetOutputValue(0, Datum(log2f(value)));
    }
    else
    {
        SetOutputValue(0, Datum(0.0f));
    }
}

glm::vec4 Log2Node::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// ExpNode
// =============================================================================
DEFINE_GRAPH_NODE(ExpNode);

void ExpNode::SetupPins()
{
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void ExpNode::Evaluate()
{
    float value = GetInputValue(0).GetFloat();
    SetOutputValue(0, Datum(expf(value)));
}

glm::vec4 ExpNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// RandomNode
// =============================================================================
DEFINE_GRAPH_NODE(RandomNode);

void RandomNode::SetupPins()
{
    AddOutputPin("Value", DatumType::Float);
}

void RandomNode::Evaluate()
{
    SetOutputValue(0, Datum((float)rand() / (float)RAND_MAX));
}

glm::vec4 RandomNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// RandomRangeNode
// =============================================================================
DEFINE_GRAPH_NODE(RandomRangeNode);

void RandomRangeNode::SetupPins()
{
    AddInputPin("Min", DatumType::Float, Datum(0.0f));
    AddInputPin("Max", DatumType::Float, Datum(1.0f));
    AddOutputPin("Value", DatumType::Float);
}

void RandomRangeNode::Evaluate()
{
    float min = GetInputValue(0).GetFloat();
    float max = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(min + ((float)rand() / (float)RAND_MAX) * (max - min)));
}

glm::vec4 RandomRangeNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// PINode
// =============================================================================
DEFINE_GRAPH_NODE(PINode);

void PINode::SetupPins()
{
    AddOutputPin("Value", DatumType::Float);
}

void PINode::Evaluate()
{
    SetOutputValue(0, Datum(3.14159265358979323846f));
}

glm::vec4 PINode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// EqualNode
// =============================================================================
DEFINE_GRAPH_NODE(EqualNode);

void EqualNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void EqualNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a == b));
}

glm::vec4 EqualNode::GetNodeColor() const { return kComparisonNodeColor; }

// =============================================================================
// NotEqualNode
// =============================================================================
DEFINE_GRAPH_NODE(NotEqualNode);

void NotEqualNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void NotEqualNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a != b));
}

glm::vec4 NotEqualNode::GetNodeColor() const { return kComparisonNodeColor; }

// =============================================================================
// GreaterNode
// =============================================================================
DEFINE_GRAPH_NODE(GreaterNode);

void GreaterNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void GreaterNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a > b));
}

glm::vec4 GreaterNode::GetNodeColor() const { return kComparisonNodeColor; }

// =============================================================================
// LessNode
// =============================================================================
DEFINE_GRAPH_NODE(LessNode);

void LessNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void LessNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a < b));
}

glm::vec4 LessNode::GetNodeColor() const { return kComparisonNodeColor; }

// =============================================================================
// GreaterEqualNode
// =============================================================================
DEFINE_GRAPH_NODE(GreaterEqualNode);

void GreaterEqualNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void GreaterEqualNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a >= b));
}

glm::vec4 GreaterEqualNode::GetNodeColor() const { return kComparisonNodeColor; }

// =============================================================================
// LessEqualNode
// =============================================================================
DEFINE_GRAPH_NODE(LessEqualNode);

void LessEqualNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void LessEqualNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a <= b));
}

glm::vec4 LessEqualNode::GetNodeColor() const { return kComparisonNodeColor; }

// =============================================================================
// AndNode
// =============================================================================
DEFINE_GRAPH_NODE(AndNode);

void AndNode::SetupPins()
{
    AddInputPin("A", DatumType::Bool, Datum(false));
    AddInputPin("B", DatumType::Bool, Datum(false));
    AddOutputPin("Result", DatumType::Bool);
}

void AndNode::Evaluate()
{
    bool a = GetInputValue(0).GetBool();
    bool b = GetInputValue(1).GetBool();
    SetOutputValue(0, Datum(a && b));
}

glm::vec4 AndNode::GetNodeColor() const { return kLogicNodeColor; }

// =============================================================================
// OrNode
// =============================================================================
DEFINE_GRAPH_NODE(OrNode);

void OrNode::SetupPins()
{
    AddInputPin("A", DatumType::Bool, Datum(false));
    AddInputPin("B", DatumType::Bool, Datum(false));
    AddOutputPin("Result", DatumType::Bool);
}

void OrNode::Evaluate()
{
    bool a = GetInputValue(0).GetBool();
    bool b = GetInputValue(1).GetBool();
    SetOutputValue(0, Datum(a || b));
}

glm::vec4 OrNode::GetNodeColor() const { return kLogicNodeColor; }

// =============================================================================
// NotNode
// =============================================================================
DEFINE_GRAPH_NODE(NotNode);

void NotNode::SetupPins()
{
    AddInputPin("Value", DatumType::Bool, Datum(false));
    AddOutputPin("Result", DatumType::Bool);
}

void NotNode::Evaluate()
{
    bool value = GetInputValue(0).GetBool();
    SetOutputValue(0, Datum(!value));
}

glm::vec4 NotNode::GetNodeColor() const { return kLogicNodeColor; }

// =============================================================================
// XorNode
// =============================================================================
DEFINE_GRAPH_NODE(XorNode);

void XorNode::SetupPins()
{
    AddInputPin("A", DatumType::Bool, Datum(false));
    AddInputPin("B", DatumType::Bool, Datum(false));
    AddOutputPin("Result", DatumType::Bool);
}

void XorNode::Evaluate()
{
    bool a = GetInputValue(0).GetBool();
    bool b = GetInputValue(1).GetBool();
    SetOutputValue(0, Datum(a != b));
}

glm::vec4 XorNode::GetNodeColor() const { return kLogicNodeColor; }

// =============================================================================
// SelectNode
// =============================================================================
DEFINE_GRAPH_NODE(SelectNode);

void SelectNode::SetupPins()
{
    AddInputPin("Condition", DatumType::Bool, Datum(true));
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Float);
}

void SelectNode::Evaluate()
{
    bool condition = GetInputValue(0).GetBool();
    float a = GetInputValue(1).GetFloat();
    float b = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(condition ? a : b));
}

glm::vec4 SelectNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// IsValidNode
// =============================================================================
DEFINE_GRAPH_NODE(IsValidNode);

void IsValidNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddOutputPin("Valid", DatumType::Bool);
}

void IsValidNode::Evaluate()
{
    SetOutputValue(0, Datum(GetInputValue(0).GetNode().Get() != nullptr));
}

glm::vec4 IsValidNode::GetNodeColor() const { return kMathNodeColor; }

// =============================================================================
// IsNullNode
// =============================================================================
DEFINE_GRAPH_NODE(IsNullNode);

void IsNullNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddOutputPin("Is Null", DatumType::Bool);
}

void IsNullNode::Evaluate()
{
    SetOutputValue(0, Datum(GetInputValue(0).GetNode().Get() == nullptr));
}

glm::vec4 IsNullNode::GetNodeColor() const { return glm::vec4(0.2f, 0.6f, 0.3f, 1.0f); }

// =============================================================================
// IsNearlyEqualNode
// =============================================================================
DEFINE_GRAPH_NODE(IsNearlyEqualNode);

void IsNearlyEqualNode::SetupPins()
{
    AddInputPin("A", DatumType::Float, Datum(0.0f));
    AddInputPin("B", DatumType::Float, Datum(0.0f));
    AddInputPin("Epsilon", DatumType::Float, Datum(0.0001f));
    AddOutputPin("Result", DatumType::Bool);
}

void IsNearlyEqualNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    float epsilon = GetInputValue(2).GetFloat();
    SetOutputValue(0, Datum(glm::abs(a - b) <= epsilon));
}

glm::vec4 IsNearlyEqualNode::GetNodeColor() const { return kComparisonNodeColor; }
