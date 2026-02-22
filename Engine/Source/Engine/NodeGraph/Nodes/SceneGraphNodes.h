#pragma once

#include "NodeGraph/GraphNode.h"

// =============================================================================
// Event Entry Nodes
// =============================================================================

class StartEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StartEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "Start"; }
    virtual const char* GetNodeTypeName() const override { return "Start"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class TickEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(TickEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "Tick"; }
    virtual const char* GetNodeTypeName() const override { return "Tick"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class StopEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StopEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "Stop"; }
    virtual const char* GetNodeTypeName() const override { return "Stop"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Physics Event Nodes
// =============================================================================

class BeginOverlapEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(BeginOverlapEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "BeginOverlap"; }
    virtual const char* GetNodeTypeName() const override { return "On Overlap Begin"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class EndOverlapEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(EndOverlapEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "EndOverlap"; }
    virtual const char* GetNodeTypeName() const override { return "On Overlap End"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class CollisionBeginEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CollisionBeginEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "CollisionBegin"; }
    virtual const char* GetNodeTypeName() const override { return "On Collision Begin"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class CollisionEndEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(CollisionEndEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsEventNode() const override { return true; }
    virtual const char* GetEventName() const override { return "CollisionEnd"; }
    virtual const char* GetNodeTypeName() const override { return "On Collision End"; }
    virtual const char* GetNodeCategory() const override { return "Event"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Graph Output Node (flexible sink for script readback)
// =============================================================================

class GraphOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GraphOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Graph Output"; }
    virtual const char* GetNodeCategory() const override { return "Utility"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Existing Scene Graph Nodes
// =============================================================================

// --- Scene Graph Output ---
class SceneGraphOutputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SceneGraphOutputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Scene Graph Output"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Get Transform ---
class GetTransformNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetTransformNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Transform"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Set Transform ---
class SetTransformNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetTransformNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Set Transform"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Find Node ---
class FindNodeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindNodeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find Node"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Node Reference Nodes (pure data)
// =============================================================================

class GetSelfNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetSelfNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Self"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetParentGraphNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetParentGraphNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Parent"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class FindInSceneNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindInSceneNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find In Scene"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class FindChildGraphNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FindChildGraphNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Find Child"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Transform Get (pure data)
// =============================================================================

class GetPositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetPositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Position"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetRotationNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetRotationNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Rotation"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetScaleNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetScaleNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Scale"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Transform Set (exec flow)
// =============================================================================

class SetPositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Position"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetRotationNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetRotationNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Rotation"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetScaleNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetScaleNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Scale"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetVisibilityNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetVisibilityNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Visibility"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetActiveGraphNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetActiveGraphNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Active"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Distance (pure data)
// =============================================================================

class DistanceBetweenNodesNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DistanceBetweenNodesNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Distance Between Nodes"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class RandomPointNearNodeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(RandomPointNearNodeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Random Point Near Node"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Scene Instantiation (exec flow)
// =============================================================================

class InstantiateSceneNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InstantiateSceneNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Instantiate Scene"; }
    virtual const char* GetNodeCategory() const override { return "Scene Graph"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Attribute Set Nodes (exec flow)
// =============================================================================

class SetAttributeIntNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAttributeIntNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Attribute (Int)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetAttributeFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAttributeFloatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Attribute (Float)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetAttributeVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAttributeVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Attribute (Vector)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetAttributeStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAttributeStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Attribute (String)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetAttributeBoolNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAttributeBoolNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Attribute (Bool)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetAttributeColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAttributeColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Attribute (Color)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Attribute Get Nodes (pure data)
// =============================================================================

class GetAttributeIntNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetAttributeIntNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Attribute (Int)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetAttributeFloatNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetAttributeFloatNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Attribute (Float)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetAttributeVectorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetAttributeVectorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Attribute (Vector)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetAttributeStringNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetAttributeStringNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Attribute (String)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetAttributeBoolNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetAttributeBoolNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Attribute (Bool)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetAttributeColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetAttributeColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Attribute (Color)"; }
    virtual const char* GetNodeCategory() const override { return "Attribute"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Widget Nodes
// =============================================================================

class SetTextNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetTextNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Text"; }
    virtual const char* GetNodeCategory() const override { return "Widget"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetTextNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetTextNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Text"; }
    virtual const char* GetNodeCategory() const override { return "Widget"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetSpriteNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetSpriteNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Sprite"; }
    virtual const char* GetNodeCategory() const override { return "Widget"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetSizeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetSizeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Size"; }
    virtual const char* GetNodeCategory() const override { return "Widget"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class GetSizeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GetSizeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Get Size"; }
    virtual const char* GetNodeCategory() const override { return "Widget"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetWidgetColorNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetWidgetColorNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Widget Color"; }
    virtual const char* GetNodeCategory() const override { return "Widget"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Audio Nodes
// =============================================================================

class SetAudioClipNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAudioClipNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Audio Clip"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetAudioTimeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetAudioTimeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Audio Time"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetVolumeNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetVolumeNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Volume"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SetPitchNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SetPitchNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Set Pitch"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class PlayAudioNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PlayAudioNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Play Audio"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class StopAudioNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StopAudioNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Stop Audio"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class PauseAudioNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PauseAudioNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Pause Audio"; }
    virtual const char* GetNodeCategory() const override { return "Audio"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Animation Nodes
// =============================================================================

class PlayAnimationNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(PlayAnimationNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Play Animation"; }
    virtual const char* GetNodeCategory() const override { return "Animation"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class StopAnimationNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StopAnimationNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Stop Animation"; }
    virtual const char* GetNodeCategory() const override { return "Animation"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// =============================================================================
// Flow Control Nodes
// =============================================================================

class ForLoopNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ForLoopNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "For Loop"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class ForEachLoopNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ForEachLoopNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "For Each"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class WaitTillNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(WaitTillNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Wait"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;

    float mStartTime = -1.0f;
};

class BranchNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(BranchNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Branch"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;
};

class SequenceNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(SequenceNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Sequence"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- While Loop ---
class WhileLoopNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(WhileLoopNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "While Loop"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Do Once ---
class DoOnceNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DoOnceNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Do Once"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;

    bool mHasFired = false;
};

// --- Flip Flop ---
class FlipFlopNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FlipFlopNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Flip Flop"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;

    bool mIsA = true;
};

// --- Gate ---
class GateNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GateNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Gate"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Do N ---
class DoNNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(DoNNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Do N"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;

    int32_t mCallCount = 0;
};

// --- Multi Gate ---
class MultiGateNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MultiGateNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Multi Gate"; }
    virtual const char* GetNodeCategory() const override { return "Flow Control"; }
    virtual glm::vec4 GetNodeColor() const override;

    int32_t mCurrentIndex = 0;
};

// =============================================================================
// Input Nodes
// =============================================================================

// --- Input Event ---
class InputEventNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InputEventNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual bool IsFlowNode() const override { return true; }
    virtual const char* GetNodeTypeName() const override { return "Input Event"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override;
    virtual bool GetPinEnumOptions(uint32_t pinIndex, std::vector<PinEnumOption>& outOptions) const override;
};

// --- Input Down ---
class InputDownNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InputDownNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Input Down"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override;
    virtual bool GetPinEnumOptions(uint32_t pinIndex, std::vector<PinEnumOption>& outOptions) const override;
};

// --- Input Count ---
class InputCountNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(InputCountNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Input Count"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Gamepad Axis ---
class GamepadAxisNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(GamepadAxisNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Gamepad Axis"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override;
    virtual bool GetPinEnumOptions(uint32_t pinIndex, std::vector<PinEnumOption>& outOptions) const override;
};

// --- Mouse Position ---
class MousePositionNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(MousePositionNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Mouse Position"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override;
};

// --- Scroll Wheel ---
class ScrollWheelNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ScrollWheelNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual const char* GetNodeTypeName() const override { return "Scroll Wheel"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override;
};
