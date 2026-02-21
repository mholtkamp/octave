#include "NodeGraph/Domains/SceneGraphDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/SceneGraphNodes.h"

void SceneGraphDomain::RegisterNodeTypes()
{
    // Math nodes
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractNode::GetStaticType(), "Subtract", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(IntConstantNode::GetStaticType(), "Integer", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(VectorConstantNode::GetStaticType(), "Vector", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));

    // Utility nodes
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(DebugLogNode::GetStaticType(), "Debug Log", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Scene Graph nodes
    AddNodeType(SceneGraphOutputNode::GetStaticType(), "Scene Graph Output", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetTransformNode::GetStaticType(), "Get Transform", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetTransformNode::GetStaticType(), "Set Transform", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(FindNodeNode::GetStaticType(), "Find Node", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
}

TypeId SceneGraphDomain::GetDefaultOutputNodeType() const
{
    return SceneGraphOutputNode::GetStaticType();
}

void SceneGraphDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the SceneGraphOutputNode's input pins contain the final scene graph parameters.
}
