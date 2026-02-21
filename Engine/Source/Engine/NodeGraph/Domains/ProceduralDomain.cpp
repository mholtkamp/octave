#include "NodeGraph/Domains/ProceduralDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/ProceduralNodes.h"

void ProceduralDomain::RegisterNodeTypes()
{
    // Math nodes
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractNode::GetStaticType(), "Subtract", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DivideNode::GetStaticType(), "Divide", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpNode::GetStaticType(), "Lerp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ClampNode::GetStaticType(), "Clamp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(AbsNode::GetStaticType(), "Abs", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DotProductNode::GetStaticType(), "Dot Product", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RemapValueNode::GetStaticType(), "Remap Value", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SinNode::GetStaticType(), "Sin", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));


    // Value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(IntConstantNode::GetStaticType(), "Integer", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(VectorConstantNode::GetStaticType(), "Vector", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(ColorConstantNode::GetStaticType(), "Color", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(TimeNode::GetStaticType(), "Time", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));

    // Utility nodes
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(DebugLogNode::GetStaticType(), "Debug Log", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Procedural nodes
    AddNodeType(ProceduralOutputNode::GetStaticType(), "Procedural Output", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
    AddNodeType(NoiseNode::GetStaticType(), "Noise", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
    AddNodeType(VoronoiNode::GetStaticType(), "Voronoi", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
    AddNodeType(GradientNode::GetStaticType(), "Gradient", "Procedural", glm::vec4(0.8f, 0.5f, 0.1f, 1.0f));
}

TypeId ProceduralDomain::GetDefaultOutputNodeType() const
{
    return ProceduralOutputNode::GetStaticType();
}

void ProceduralDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the ProceduralOutputNode's input pins contain the final procedural parameters.
}
