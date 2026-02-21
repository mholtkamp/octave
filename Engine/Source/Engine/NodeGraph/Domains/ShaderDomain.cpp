#include "NodeGraph/Domains/ShaderDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/ShaderNodes.h"

void ShaderDomain::RegisterNodeTypes()
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

    // Shader nodes
    AddNodeType(ShaderOutputNode::GetStaticType(), "Shader Output", "Shader", glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));
    AddNodeType(VertexPositionNode::GetStaticType(), "Vertex Position", "Shader", glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));
    AddNodeType(VertexNormalNode::GetStaticType(), "Vertex Normal", "Shader", glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));
    AddNodeType(WorldPositionNode::GetStaticType(), "World Position", "Shader", glm::vec4(0.2f, 0.4f, 0.8f, 1.0f));
}

TypeId ShaderDomain::GetDefaultOutputNodeType() const
{
    return ShaderOutputNode::GetStaticType();
}

void ShaderDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the ShaderOutputNode's input pins contain the final shader parameters.
}
