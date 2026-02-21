#include "NodeGraph/Domains/MaterialDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/MaterialNodes.h"
#include "NodeGraph/Nodes/InputNodes.h"

void MaterialDomain::RegisterNodeTypes()
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

    // Material nodes
    AddNodeType(MaterialOutputNode::GetStaticType(), "Material Output", "Material", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(TextureSampleNode::GetStaticType(), "Texture Sample", "Material", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(FresnelNode::GetStaticType(), "Fresnel", "Material", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(PannerNode::GetStaticType(), "Panner", "Material", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(NormalMapNode::GetStaticType(), "Normal Map", "Material", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
}

TypeId MaterialDomain::GetDefaultOutputNodeType() const
{
    return MaterialOutputNode::GetStaticType();
}

void MaterialDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // No-op: the actual writeback is done by MaterialLite::ApplyGraphToParams()
    // on the owning material side, called by the editor preview or NodeGraphPlayer.
}
