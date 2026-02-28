#include "NodeGraph/Domains/AnimationDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/AnimationNodes.h"

void AnimationDomain::RegisterNodeTypes()
{
    // Math nodes
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractNode::GetStaticType(), "Subtract", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DivideNode::GetStaticType(), "Divide", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpNode::GetStaticType(), "Lerp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ClampNode::GetStaticType(), "Clamp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RemapValueNode::GetStaticType(), "Remap Value", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SinNode::GetStaticType(), "Sin", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(IntConstantNode::GetStaticType(), "Integer", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(TimeNode::GetStaticType(), "Time", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));

    // Utility nodes
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(DebugLogNode::GetStaticType(), "Debug Log", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Animation nodes
    AddNodeType(AnimationOutputNode::GetStaticType(), "Animation Output", "Animation", glm::vec4(0.2f, 0.7f, 0.3f, 1.0f));
    AddNodeType(AnimClipNode::GetStaticType(), "Anim Clip", "Animation", glm::vec4(0.2f, 0.7f, 0.3f, 1.0f));
    AddNodeType(BlendNode::GetStaticType(), "Blend", "Animation", glm::vec4(0.2f, 0.7f, 0.3f, 1.0f));
    AddNodeType(AnimSpeedNode::GetStaticType(), "Anim Speed", "Animation", glm::vec4(0.2f, 0.7f, 0.3f, 1.0f));
}

TypeId AnimationDomain::GetDefaultOutputNodeType() const
{
    return AnimationOutputNode::GetStaticType();
}

void AnimationDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the AnimationOutputNode's input pins contain the final animation parameters.
}
