#include "NodeGraph/Nodes/FSMNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(FSMNodes);

static const glm::vec4 kFSMNodeColor = glm::vec4(0.6f, 0.2f, 0.7f, 1.0f);

// =============================================================================
// FSMOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(FSMOutputNode);

void FSMOutputNode::SetupPins()
{
    AddInputPin("State ID", DatumType::Integer, Datum(0));
    AddInputPin("Transition", DatumType::Bool, Datum(false));
}

void FSMOutputNode::Evaluate()
{
    // Sink node - collects FSM state output.
}

glm::vec4 FSMOutputNode::GetNodeColor() const { return kFSMNodeColor; }

// =============================================================================
// StateNode
// =============================================================================
DEFINE_GRAPH_NODE(StateNode);

void StateNode::SetupPins()
{
    AddInputPin("Enter", DatumType::Bool, Datum(false));
    AddOutputPin("Active", DatumType::Bool);
    AddOutputPin("State ID", DatumType::Integer);
}

void StateNode::Evaluate()
{
    bool enter = GetInputValue(0).GetBool();
    SetOutputValue(0, Datum(enter));
    SetOutputValue(1, Datum(enter ? 1 : 0));
}

glm::vec4 StateNode::GetNodeColor() const { return kFSMNodeColor; }

// =============================================================================
// TransitionNode
// =============================================================================
DEFINE_GRAPH_NODE(TransitionNode);

void TransitionNode::SetupPins()
{
    AddInputPin("Condition", DatumType::Bool, Datum(false));
    AddInputPin("From State", DatumType::Integer, Datum(0));
    AddInputPin("To State", DatumType::Integer, Datum(1));
    AddOutputPin("Should Transition", DatumType::Bool);
}

void TransitionNode::Evaluate()
{
    bool condition = GetInputValue(0).GetBool();
    SetOutputValue(0, Datum(condition));
}

glm::vec4 TransitionNode::GetNodeColor() const { return kFSMNodeColor; }

// =============================================================================
// ConditionNode
// =============================================================================
DEFINE_GRAPH_NODE(ConditionNode);

void ConditionNode::SetupPins()
{
    AddInputPin("Value A", DatumType::Float, Datum(0.0f));
    AddInputPin("Value B", DatumType::Float, Datum(0.0f));
    AddOutputPin("Result", DatumType::Bool);
}

void ConditionNode::Evaluate()
{
    float a = GetInputValue(0).GetFloat();
    float b = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(a > b));
}

glm::vec4 ConditionNode::GetNodeColor() const { return kFSMNodeColor; }
