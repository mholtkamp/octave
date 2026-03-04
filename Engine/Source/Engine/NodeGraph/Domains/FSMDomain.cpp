#include "NodeGraph/Domains/FSMDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/FSMNodes.h"

void FSMDomain::RegisterNodeTypes()
{
    // Value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(IntConstantNode::GetStaticType(), "Integer", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));

    // Utility nodes
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(DebugLogNode::GetStaticType(), "Debug Log", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // FSM nodes
    AddNodeType(FSMOutputNode::GetStaticType(), "FSM Output", "FSM", glm::vec4(0.6f, 0.2f, 0.7f, 1.0f));
    AddNodeType(StateNode::GetStaticType(), "State", "FSM", glm::vec4(0.6f, 0.2f, 0.7f, 1.0f));
    AddNodeType(TransitionNode::GetStaticType(), "Transition", "FSM", glm::vec4(0.6f, 0.2f, 0.7f, 1.0f));
    AddNodeType(ConditionNode::GetStaticType(), "Condition", "FSM", glm::vec4(0.6f, 0.2f, 0.7f, 1.0f));
}

TypeId FSMDomain::GetDefaultOutputNodeType() const
{
    return FSMOutputNode::GetStaticType();
}

void FSMDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the FSMOutputNode's input pins contain the final FSM state.
}
