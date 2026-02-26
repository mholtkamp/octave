#include "NodeGraph/Domains/SceneGraphDomain.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/SceneGraphNodes.h"
#include "NodeGraph/Nodes/PointNodes.h"
#include "NodeGraph/Nodes/SplineNodes.h"
#include "NodeGraph/Nodes/GizmoNodes.h"

void SceneGraphDomain::RegisterNodeTypes()
{
    // Event nodes
    AddNodeType(StartEventNode::GetStaticType(), "Start", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(TickEventNode::GetStaticType(), "Tick", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(StopEventNode::GetStaticType(), "Stop", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(BeginOverlapEventNode::GetStaticType(), "On Overlap Begin", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(EndOverlapEventNode::GetStaticType(), "On Overlap End", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(CollisionBeginEventNode::GetStaticType(), "On Collision Begin", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
    AddNodeType(CollisionEndEventNode::GetStaticType(), "On Collision End", "Event", glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));

    // Math nodes
    AddNodeType(AddNode::GetStaticType(), "Add", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractNode::GetStaticType(), "Subtract", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyNode::GetStaticType(), "Multiply", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RemapValueNode::GetStaticType(), "Remap Value", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SinNode::GetStaticType(), "Sin", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(VectorDistanceNode::GetStaticType(), "Vector Distance", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RandomPointNearVectorNode::GetStaticType(), "Random Point Near Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Value nodes
    AddNodeType(FloatConstantNode::GetStaticType(), "Float", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(IntConstantNode::GetStaticType(), "Integer", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(VectorConstantNode::GetStaticType(), "Vector", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(TimeNode::GetStaticType(), "Time", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));
    AddNodeType(TimeNode::GetStaticType(), "Time", "Value", glm::vec4(0.2f, 0.5f, 0.7f, 1.0f));

    // Utility nodes
    AddNodeType(ViewerNode::GetStaticType(), "Viewer", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(DebugLogNode::GetStaticType(), "Debug Log", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(GraphOutputNode::GetStaticType(), "Graph Output", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Scene Graph nodes (existing)
    AddNodeType(SceneGraphOutputNode::GetStaticType(), "Scene Graph Output", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetTransformNode::GetStaticType(), "Get Transform", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetTransformNode::GetStaticType(), "Set Transform", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(FindNodeNode::GetStaticType(), "Find Node", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetNode3DName::GetStaticType(), "Get Node3D Name", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetNode3DClass::GetStaticType(), "Get Node3D Class", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Scene Graph - Node Reference
    AddNodeType(GetSelfNode::GetStaticType(), "Get Self", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetParentGraphNode::GetStaticType(), "Get Parent", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(FindInSceneNode::GetStaticType(), "Find In Scene", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(FindChildGraphNode::GetStaticType(), "Find Child", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Scene Graph - Transform Get
    AddNodeType(GetPositionNode::GetStaticType(), "Get Position", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetRotationNode::GetStaticType(), "Get Rotation", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetScaleNode::GetStaticType(), "Get Scale", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Scene Graph - Transform Set
    AddNodeType(SetPositionNode::GetStaticType(), "Set Position", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetRotationNode::GetStaticType(), "Set Rotation", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetScaleNode::GetStaticType(), "Set Scale", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetVisibilityNode::GetStaticType(), "Set Visibility", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetActiveGraphNode::GetStaticType(), "Set Active", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Scene Graph - Distance
    AddNodeType(DistanceBetweenNodesNode::GetStaticType(), "Distance Between Nodes", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(RandomPointNearNodeNode::GetStaticType(), "Random Point Near Node", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Scene Graph - Instantiation
    AddNodeType(InstantiateSceneNode::GetStaticType(), "Instantiate Scene", "Scene Graph", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Attribute nodes - Set
    AddNodeType(SetAttributeIntNode::GetStaticType(), "Set Attribute (Int)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetAttributeFloatNode::GetStaticType(), "Set Attribute (Float)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetAttributeVectorNode::GetStaticType(), "Set Attribute (Vector)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetAttributeStringNode::GetStaticType(), "Set Attribute (String)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetAttributeBoolNode::GetStaticType(), "Set Attribute (Bool)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(SetAttributeColorNode::GetStaticType(), "Set Attribute (Color)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Attribute nodes - Get
    AddNodeType(GetAttributeIntNode::GetStaticType(), "Get Attribute (Int)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetAttributeFloatNode::GetStaticType(), "Get Attribute (Float)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetAttributeVectorNode::GetStaticType(), "Get Attribute (Vector)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetAttributeStringNode::GetStaticType(), "Get Attribute (String)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetAttributeBoolNode::GetStaticType(), "Get Attribute (Bool)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));
    AddNodeType(GetAttributeColorNode::GetStaticType(), "Get Attribute (Color)", "Attribute", glm::vec4(0.1f, 0.6f, 0.6f, 1.0f));

    // Widget nodes
    AddNodeType(SetTextNode::GetStaticType(), "Set Text", "Widget", glm::vec4(0.6f, 0.2f, 0.8f, 1.0f));
    AddNodeType(GetTextNode::GetStaticType(), "Get Text", "Widget", glm::vec4(0.6f, 0.2f, 0.8f, 1.0f));
    AddNodeType(SetSpriteNode::GetStaticType(), "Set Sprite", "Widget", glm::vec4(0.6f, 0.2f, 0.8f, 1.0f));
    AddNodeType(SetSizeNode::GetStaticType(), "Set Size", "Widget", glm::vec4(0.6f, 0.2f, 0.8f, 1.0f));
    AddNodeType(GetSizeNode::GetStaticType(), "Get Size", "Widget", glm::vec4(0.6f, 0.2f, 0.8f, 1.0f));
    AddNodeType(SetWidgetColorNode::GetStaticType(), "Set Widget Color", "Widget", glm::vec4(0.6f, 0.2f, 0.8f, 1.0f));

    // Audio nodes
    AddNodeType(SetAudioClipNode::GetStaticType(), "Set Audio Clip", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    AddNodeType(SetAudioTimeNode::GetStaticType(), "Set Audio Time", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    AddNodeType(SetVolumeNode::GetStaticType(), "Set Volume", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    AddNodeType(SetPitchNode::GetStaticType(), "Set Pitch", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    AddNodeType(PlayAudioNode::GetStaticType(), "Play Audio", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    AddNodeType(StopAudioNode::GetStaticType(), "Stop Audio", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));
    AddNodeType(PauseAudioNode::GetStaticType(), "Pause Audio", "Audio", glm::vec4(0.2f, 0.8f, 0.3f, 1.0f));

    // Animation nodes
    AddNodeType(PlayAnimationNode::GetStaticType(), "Play Animation", "Animation", glm::vec4(0.8f, 0.5f, 0.2f, 1.0f));
    AddNodeType(StopAnimationNode::GetStaticType(), "Stop Animation", "Animation", glm::vec4(0.8f, 0.5f, 0.2f, 1.0f));

    // Flow Control nodes
    AddNodeType(ForLoopNode::GetStaticType(), "For Loop", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(ForEachLoopNode::GetStaticType(), "For Each", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(WaitTillNode::GetStaticType(), "Wait", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(BranchNode::GetStaticType(), "Branch", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(SequenceNode::GetStaticType(), "Sequence", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(WhileLoopNode::GetStaticType(), "While Loop", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(DoOnceNode::GetStaticType(), "Do Once", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(FlipFlopNode::GetStaticType(), "Flip Flop", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(GateNode::GetStaticType(), "Gate", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(DoNNode::GetStaticType(), "Do N", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    AddNodeType(MultiGateNode::GetStaticType(), "Multi Gate", "Flow Control", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    // Math nodes - Trigonometry
    AddNodeType(CosNode::GetStaticType(), "Cos", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(TanNode::GetStaticType(), "Tan", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ASinNode::GetStaticType(), "ASin", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ACosNode::GetStaticType(), "ACos", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ATanNode::GetStaticType(), "ATan", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ATan2Node::GetStaticType(), "ATan2", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Math nodes - Fundamentals
    AddNodeType(SqrtNode::GetStaticType(), "Sqrt", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(PowerNode::GetStaticType(), "Power", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(FloorNode::GetStaticType(), "Floor", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(CeilNode::GetStaticType(), "Ceil", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RoundNode::GetStaticType(), "Round", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(FmodNode::GetStaticType(), "Fmod", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MinNode::GetStaticType(), "Min", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MaxNode::GetStaticType(), "Max", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(NegateNode::GetStaticType(), "Negate", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SignNode::GetStaticType(), "Sign", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(FracNode::GetStaticType(), "Frac", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(OneMinusNode::GetStaticType(), "One Minus", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Math nodes - Vector Arithmetic
    AddNodeType(AddVectorNode::GetStaticType(), "Add Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(SubtractVectorNode::GetStaticType(), "Subtract Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyVectorNode::GetStaticType(), "Multiply Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(DivideVectorNode::GetStaticType(), "Divide Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(MultiplyVectorComponentNode::GetStaticType(), "Multiply Vector Component", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(LerpVectorNode::GetStaticType(), "Lerp Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(NegateVectorNode::GetStaticType(), "Negate Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Math nodes - Vector Operations
    AddNodeType(CrossProductNode::GetStaticType(), "Cross Product", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(NormalizeNode::GetStaticType(), "Normalize", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(VectorLengthNode::GetStaticType(), "Vector Length", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ReflectNode::GetStaticType(), "Reflect", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RotateVectorNode::GetStaticType(), "Rotate Vector", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Math nodes - Interpolation
    AddNodeType(SmoothstepNode::GetStaticType(), "Smoothstep", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(InverseLerpNode::GetStaticType(), "Inverse Lerp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(StepNode::GetStaticType(), "Step", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Math nodes - Miscellaneous
    AddNodeType(LogNode::GetStaticType(), "Log", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(Log2Node::GetStaticType(), "Log2", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(ExpNode::GetStaticType(), "Exp", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RandomNode::GetStaticType(), "Random", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(RandomRangeNode::GetStaticType(), "Random Range", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(PINode::GetStaticType(), "PI", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));

    // Comparison nodes
    AddNodeType(EqualNode::GetStaticType(), "Equal", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));
    AddNodeType(NotEqualNode::GetStaticType(), "Not Equal", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));
    AddNodeType(GreaterNode::GetStaticType(), "Greater", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));
    AddNodeType(LessNode::GetStaticType(), "Less", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));
    AddNodeType(GreaterEqualNode::GetStaticType(), "Greater Equal", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));
    AddNodeType(LessEqualNode::GetStaticType(), "Less Equal", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));
    AddNodeType(IsNearlyEqualNode::GetStaticType(), "Is Nearly Equal", "Comparison", glm::vec4(0.6f, 0.3f, 0.3f, 1.0f));

    // Logic nodes
    AddNodeType(AndNode::GetStaticType(), "And", "Logic", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(OrNode::GetStaticType(), "Or", "Logic", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(NotNode::GetStaticType(), "Not", "Logic", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));
    AddNodeType(XorNode::GetStaticType(), "Xor", "Logic", glm::vec4(0.7f, 0.2f, 0.2f, 1.0f));

    // Selection / Validation nodes
    AddNodeType(SelectNode::GetStaticType(), "Select", "Math", glm::vec4(0.4f, 0.6f, 0.2f, 1.0f));
    AddNodeType(IsValidNode::GetStaticType(), "Is Valid", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(IsNullNode::GetStaticType(), "Is Null", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(NodeToNode3DNode::GetStaticType(), "Node To Node3D", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(Node3DToNodeNode::GetStaticType(), "Node3D To Node", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(NodeToTextNode::GetStaticType(), "Node To Text", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(Node3DToTextNode::GetStaticType(), "Node3D To Text", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Conversion nodes - To String
    AddNodeType(FloatToStringNode::GetStaticType(), "Float To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(IntToStringNode::GetStaticType(), "Int To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(BoolToStringNode::GetStaticType(), "Bool To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(VectorToStringNode::GetStaticType(), "Vector To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(ColorToStringNode::GetStaticType(), "Color To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(NodeToStringNode::GetStaticType(), "Node To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(Node3DToStringNode::GetStaticType(), "Node3D To String", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));

    // Conversion nodes - From String
    AddNodeType(StringToFloatNode::GetStaticType(), "String To Float", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToIntNode::GetStaticType(), "String To Int", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToBoolNode::GetStaticType(), "String To Bool", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToVectorNode::GetStaticType(), "String To Vector", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(StringToColorNode::GetStaticType(), "String To Color", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));

    // Conversion nodes - Type
    AddNodeType(VectorToColorNode::GetStaticType(), "Vector To Color", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(ColorToVectorNode::GetStaticType(), "Color To Vector", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(SplitVectorNode::GetStaticType(), "Split Vector", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));
    AddNodeType(SplitColorNode::GetStaticType(), "Split Color", "Conversion", glm::vec4(0.6f, 0.4f, 0.7f, 1.0f));

    // String nodes
    AddNodeType(StringConcatenateNode::GetStaticType(), "String Concatenate", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringLengthNode::GetStaticType(), "String Length", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(SubstringNode::GetStaticType(), "Substring", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringContainsNode::GetStaticType(), "String Contains", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringFindNode::GetStaticType(), "String Find", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringReplaceNode::GetStaticType(), "String Replace", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringSplitNode::GetStaticType(), "String Split", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringToUpperNode::GetStaticType(), "String To Upper", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringToLowerNode::GetStaticType(), "String To Lower", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringTrimNode::GetStaticType(), "String Trim", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringFormatNode::GetStaticType(), "String Format", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringStartsWithNode::GetStaticType(), "String Starts With", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));
    AddNodeType(StringEndsWithNode::GetStaticType(), "String Ends With", "String", glm::vec4(0.7f, 0.3f, 0.5f, 1.0f));

    // Utility nodes (new)
    AddNodeType(MakeVectorNode::GetStaticType(), "Make Vector", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(MakeVector2DNode::GetStaticType(), "Make Vector 2D", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));
    AddNodeType(RerouteNode::GetStaticType(), "Reroute", "Utility", glm::vec4(0.2f, 0.6f, 0.3f, 1.0f));

    // Input nodes
    AddNodeType(InputEventNode::GetStaticType(), "Input Event", "Input", glm::vec4(0.8f, 0.6f, 0.1f, 1.0f));
    AddNodeType(InputDownNode::GetStaticType(), "Input Down", "Input", glm::vec4(0.8f, 0.6f, 0.1f, 1.0f));
    AddNodeType(InputCountNode::GetStaticType(), "Input Count", "Input", glm::vec4(0.8f, 0.6f, 0.1f, 1.0f));
    AddNodeType(GamepadAxisNode::GetStaticType(), "Gamepad Axis", "Input", glm::vec4(0.8f, 0.6f, 0.1f, 1.0f));
    AddNodeType(MousePositionNode::GetStaticType(), "Mouse Position", "Input", glm::vec4(0.8f, 0.6f, 0.1f, 1.0f));
    AddNodeType(ScrollWheelNode::GetStaticType(), "Scroll Wheel", "Input", glm::vec4(0.8f, 0.6f, 0.1f, 1.0f));

    // Point Generation nodes
    static const glm::vec4 kPointColor(0.9f, 0.55f, 0.1f, 1.0f);
    AddNodeType(CreatePointsGridNode::GetStaticType(), "Create Points Grid", "Point", kPointColor);
    AddNodeType(ScatterPointsOnMeshNode::GetStaticType(), "Scatter Points On Mesh", "Point", kPointColor);
    AddNodeType(RandomPointsInBoxNode::GetStaticType(), "Random Points In Box", "Point", kPointColor);
    AddNodeType(PointsFromSplineNode::GetStaticType(), "Points From Spline", "Point", kPointColor);
    AddNodeType(CreatePointsLineNode::GetStaticType(), "Create Points Line", "Point", kPointColor);
    AddNodeType(MergePointsNode::GetStaticType(), "Merge Points", "Point", kPointColor);

    // Point Attribute nodes
    AddNodeType(SetPointAttributeFloatNode::GetStaticType(), "Set Point Attribute (Float)", "Point", kPointColor);
    AddNodeType(SetPointAttributeVectorNode::GetStaticType(), "Set Point Attribute (Vector)", "Point", kPointColor);
    AddNodeType(SetPointAttributeColorNode::GetStaticType(), "Set Point Attribute (Color)", "Point", kPointColor);
    AddNodeType(GetPointAttributeFloatNode::GetStaticType(), "Get Point Attribute (Float)", "Point", kPointColor);
    AddNodeType(GetPointAttributeVectorNode::GetStaticType(), "Get Point Attribute (Vector)", "Point", kPointColor);
    AddNodeType(GetPointAttributeColorNode::GetStaticType(), "Get Point Attribute (Color)", "Point", kPointColor);
    AddNodeType(GetPointCountNode::GetStaticType(), "Get Point Count", "Point", kPointColor);
    AddNodeType(GetPointPositionNode::GetStaticType(), "Get Point Position", "Point", kPointColor);
    AddNodeType(SetPointPositionNode::GetStaticType(), "Set Point Position", "Point", kPointColor);

    // Point Iteration / Transform / Output nodes
    AddNodeType(ForEachPointNode::GetStaticType(), "For Each Point", "Point", kPointColor);
    AddNodeType(FilterPointsNode::GetStaticType(), "Filter Points", "Point", kPointColor);
    AddNodeType(TransformPointsNode::GetStaticType(), "Transform Points", "Point", kPointColor);
    AddNodeType(RandomizeAttributeNode::GetStaticType(), "Randomize Attribute", "Point", kPointColor);
    AddNodeType(CopyToPointsNode::GetStaticType(), "Copy To Points", "Point", kPointColor);
    AddNodeType(InstantiateAtPointsNode::GetStaticType(), "Instantiate At Points", "Point", kPointColor);

    // Spline nodes
    static const glm::vec4 kSplineColor(0.3f, 0.7f, 0.9f, 1.0f);
    AddNodeType(SplinePositionAtNode::GetStaticType(), "Spline Position At", "Spline", kSplineColor);
    AddNodeType(SplineTangentAtNode::GetStaticType(), "Spline Tangent At", "Spline", kSplineColor);
    AddNodeType(SplinePointCountNode::GetStaticType(), "Spline Point Count", "Spline", kSplineColor);
    AddNodeType(SplineGetPointNode::GetStaticType(), "Spline Get Point", "Spline", kSplineColor);
    AddNodeType(SplineNearestPercentNode::GetStaticType(), "Spline Nearest Percent", "Spline", kSplineColor);
    AddNodeType(SplineLengthNode::GetStaticType(), "Spline Length", "Spline", kSplineColor);

    // Gizmo nodes
    static const glm::vec4 kGizmoColor(0.85f, 0.4f, 0.9f, 1.0f);
    AddNodeType(GizmoSetColorNode::GetStaticType(), "Set Gizmo Color", "Gizmo", kGizmoColor);
    AddNodeType(GizmoSetMatrixNode::GetStaticType(), "Set Gizmo Matrix", "Gizmo", kGizmoColor);
    AddNodeType(GizmoResetStateNode::GetStaticType(), "Reset Gizmo State", "Gizmo", kGizmoColor);
    AddNodeType(GizmoDrawCubeNode::GetStaticType(), "Draw Cube", "Gizmo", kGizmoColor);
    AddNodeType(GizmoDrawWireCubeNode::GetStaticType(), "Draw Wire Cube", "Gizmo", kGizmoColor);
    AddNodeType(GizmoDrawSphereNode::GetStaticType(), "Draw Sphere", "Gizmo", kGizmoColor);
    AddNodeType(GizmoDrawWireSphereNode::GetStaticType(), "Draw Wire Sphere", "Gizmo", kGizmoColor);
    AddNodeType(GizmoDrawLineNode::GetStaticType(), "Draw Line", "Gizmo", kGizmoColor);
    AddNodeType(GizmoDrawRayNode::GetStaticType(), "Draw Ray", "Gizmo", kGizmoColor);
}

TypeId SceneGraphDomain::GetDefaultOutputNodeType() const
{
    return GraphOutputNode::GetStaticType();
}

void SceneGraphDomain::OnGraphEvaluated(NodeGraph* graph)
{
    // After evaluation, the SceneGraphOutputNode's input pins contain the final scene graph parameters.
}
