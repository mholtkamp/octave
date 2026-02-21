#pragma once

#include "NodeGraph/GraphNode.h"

// --- Add ---
class AddNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AddNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Add"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Subtract ---
class SubtractNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SubtractNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Subtract"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Multiply ---
class MultiplyNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MultiplyNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Multiply"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Divide ---
class DivideNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DivideNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Divide"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Lerp ---
class LerpNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(LerpNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Lerp"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Clamp ---
class ClampNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ClampNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Clamp"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Abs ---
class AbsNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AbsNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Abs"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Dot Product ---
class DotProductNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DotProductNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Dot Product"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// -- Remap Value ---
class RemapValueNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RemapValueNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Remap Value"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// -- Sin ---
class SinNode : public GraphNode
{
    public:
    DECLARE_GRAPH_NODE(SinNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Sin"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vector Distance ---
class VectorDistanceNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorDistanceNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vector Distance"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Random Point Near Vector ---
class RandomPointNearVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomPointNearVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Random Point Near Vector"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Trigonometry Nodes
// =============================================================================

// --- Cos ---
class CosNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CosNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Cos"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Tan ---
class TanNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TanNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Tan"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- ASin ---
class ASinNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ASinNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "ASin"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- ACos ---
class ACosNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ACosNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "ACos"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- ATan ---
class ATanNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ATanNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "ATan"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- ATan2 ---
class ATan2Node : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ATan2Node, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "ATan2"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Fundamental Math Nodes
// =============================================================================

// --- Sqrt ---
class SqrtNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SqrtNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Sqrt"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Power ---
class PowerNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PowerNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Power"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Floor ---
class FloorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FloorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Floor"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Ceil ---
class CeilNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CeilNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Ceil"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Round ---
class RoundNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RoundNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Round"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Fmod ---
class FmodNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FmodNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Fmod"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Min ---
class MinNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MinNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Min"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Max ---
class MaxNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MaxNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Max"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Negate ---
class NegateNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NegateNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Negate"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Sign ---
class SignNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SignNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Sign"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Frac ---
class FracNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FracNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Frac"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- One Minus ---
class OneMinusNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(OneMinusNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "One Minus"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Vector Operation Nodes
// =============================================================================

// --- Cross Product ---
class CrossProductNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CrossProductNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Cross Product"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Normalize ---
class NormalizeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NormalizeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Normalize"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Vector Length ---
class VectorLengthNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorLengthNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Vector Length"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Reflect ---
class ReflectNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ReflectNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Reflect"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Rotate Vector ---
class RotateVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RotateVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Rotate Vector"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Interpolation Nodes
// =============================================================================

// --- Smoothstep ---
class SmoothstepNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SmoothstepNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Smoothstep"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Inverse Lerp ---
class InverseLerpNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InverseLerpNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Inverse Lerp"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Step ---
class StepNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StepNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Step"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Miscellaneous Math Nodes
// =============================================================================

// --- Log ---
class LogNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(LogNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Log"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Log2 ---
class Log2Node : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(Log2Node, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Log2"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Exp ---
class ExpNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ExpNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Exp"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Random ---
class RandomNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Random"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Random Range ---
class RandomRangeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomRangeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Random Range"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- PI ---
class PINode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PINode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "PI"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Comparison Nodes
// =============================================================================

// --- Equal ---
class EqualNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(EqualNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Equal"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Not Equal ---
class NotEqualNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NotEqualNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Not Equal"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Greater ---
class GreaterNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GreaterNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Greater"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Less ---
class LessNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(LessNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Less"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Greater Equal ---
class GreaterEqualNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GreaterEqualNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Greater Equal"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Less Equal ---
class LessEqualNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(LessEqualNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Less Equal"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Boolean Logic Nodes
// =============================================================================

// --- And ---
class AndNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AndNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "And"; }
    virtual const char* GetNodeCategory() const override { return "Logic"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Or ---
class OrNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(OrNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Or"; }
    virtual const char* GetNodeCategory() const override { return "Logic"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Not ---
class NotNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(NotNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Not"; }
    virtual const char* GetNodeCategory() const override { return "Logic"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Xor ---
class XorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(XorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Xor"; }
    virtual const char* GetNodeCategory() const override { return "Logic"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Selection / Validation Nodes
// =============================================================================

// --- Select ---
class SelectNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SelectNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Select"; }
    virtual const char* GetNodeCategory() const override { return "Math"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Is Valid ---
class IsValidNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IsValidNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Is Valid"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Is Null ---
class IsNullNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IsNullNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Is Null"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Is Nearly Equal ---
class IsNearlyEqualNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IsNearlyEqualNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Is Nearly Equal"; }
    virtual const char* GetNodeCategory() const override { return "Comparison"; }
    virtual glm::vec4 GetNodeColor() const override;
};