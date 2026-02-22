#include "NodeGraph/Nodes/SceneGraphNodes.h"
#include "NodeGraph/NodeGraph.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/Audio3d.h"
#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Quad.h"
#include "Assets/Scene.h"
#include "Assets/SoundWave.h"
#include "Assets/Texture.h"
#include "UI/UIDocument.h"
#include "World.h"
#include "Engine.h"
#include "Clock.h"
#include "Nodes/NodeGraphPlayer.h"
#include "Log.h"
#include "Utilities.h"
#include "InputDevices.h"

#include <cstdlib>

FORCE_LINK_DEF(SceneGraphNodes);

static const glm::vec4 kSceneGraphNodeColor = glm::vec4(0.1f, 0.6f, 0.6f, 1.0f);
static const glm::vec4 kAttributeNodeColor  = glm::vec4(0.1f, 0.6f, 0.6f, 1.0f);
static const glm::vec4 kWidgetNodeColor     = glm::vec4(0.6f, 0.2f, 0.8f, 1.0f);
static const glm::vec4 kAudioNodeColor      = glm::vec4(0.2f, 0.8f, 0.3f, 1.0f);
static const glm::vec4 kAnimationNodeColor  = glm::vec4(0.8f, 0.5f, 0.2f, 1.0f);
static const glm::vec4 kFlowNodeColor       = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
static const glm::vec4 kEventNodeColor      = glm::vec4(0.8f, 0.2f, 0.2f, 1.0f);
static const glm::vec4 kGraphOutputColor    = glm::vec4(0.2f, 0.6f, 0.3f, 1.0f);
static const glm::vec4 kInputNodeColor      = glm::vec4(0.8f, 0.6f, 0.1f, 1.0f);

// Helper: get the owner Node from graph back-pointer
static Node* GetOwnerNode(GraphNode* graphNode)
{
    NodeGraph* graph = graphNode->GetGraph();
    if (graph != nullptr)
    {
        return graph->GetOwnerNode();
    }
    return nullptr;
}

// =============================================================================
// StartEventNode
// =============================================================================
DEFINE_GRAPH_NODE(StartEventNode);

void StartEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
}

void StartEventNode::Evaluate()
{
    // Event node: exec output is triggered by GraphProcessor when event fires.
}

glm::vec4 StartEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// TickEventNode
// =============================================================================
DEFINE_GRAPH_NODE(TickEventNode);

void TickEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("DeltaTime", DatumType::Float);
}

void TickEventNode::Evaluate()
{
    // Read deltaTime from the owning NodeGraphPlayer
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr)
    {
        NodeGraphPlayer* player = owner->As<NodeGraphPlayer>();
        if (player != nullptr)
        {
            SetOutputValue(1, Datum(player->GetDeltaTime()));
        }
    }
}

glm::vec4 TickEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// StopEventNode
// =============================================================================
DEFINE_GRAPH_NODE(StopEventNode);

void StopEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
}

void StopEventNode::Evaluate()
{
    // Event node: exec output is triggered by GraphProcessor when event fires.
}

glm::vec4 StopEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// BeginOverlapEventNode
// =============================================================================
DEFINE_GRAPH_NODE(BeginOverlapEventNode);

void BeginOverlapEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Other", DatumType::Node);
}

void BeginOverlapEventNode::Evaluate()
{
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr)
    {
        NodeGraphPlayer* player = owner->As<NodeGraphPlayer>();
        if (player != nullptr)
        {
            const OverlapEventData& data = player->GetCurrentOverlapEventData();
            SetOutputValue(1, Datum(data.mOtherNode));
        }
    }
}

glm::vec4 BeginOverlapEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// EndOverlapEventNode
// =============================================================================
DEFINE_GRAPH_NODE(EndOverlapEventNode);

void EndOverlapEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Other", DatumType::Node);
}

void EndOverlapEventNode::Evaluate()
{
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr)
    {
        NodeGraphPlayer* player = owner->As<NodeGraphPlayer>();
        if (player != nullptr)
        {
            const OverlapEventData& data = player->GetCurrentOverlapEventData();
            SetOutputValue(1, Datum(data.mOtherNode));
        }
    }
}

glm::vec4 EndOverlapEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// CollisionBeginEventNode
// =============================================================================
DEFINE_GRAPH_NODE(CollisionBeginEventNode);

void CollisionBeginEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Other", DatumType::Node);
    AddOutputPin("Hit Position", DatumType::Vector);
    AddOutputPin("Hit Normal", DatumType::Vector);
}

void CollisionBeginEventNode::Evaluate()
{
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr)
    {
        NodeGraphPlayer* player = owner->As<NodeGraphPlayer>();
        if (player != nullptr)
        {
            const CollisionEventData& data = player->GetCurrentCollisionEventData();
            SetOutputValue(1, Datum(data.mOtherNode));
            SetOutputValue(2, Datum(data.mHitPosition));
            SetOutputValue(3, Datum(data.mHitNormal));
        }
    }
}

glm::vec4 CollisionBeginEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// CollisionEndEventNode
// =============================================================================
DEFINE_GRAPH_NODE(CollisionEndEventNode);

void CollisionEndEventNode::SetupPins()
{
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Other", DatumType::Node);
}

void CollisionEndEventNode::Evaluate()
{
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr)
    {
        NodeGraphPlayer* player = owner->As<NodeGraphPlayer>();
        if (player != nullptr)
        {
            const CollisionEventData& data = player->GetCurrentCollisionEventData();
            SetOutputValue(1, Datum(data.mOtherNode));
        }
    }
}

glm::vec4 CollisionEndEventNode::GetNodeColor() const { return kEventNodeColor; }

// =============================================================================
// GraphOutputNode (flexible output sink for script readback)
// =============================================================================
DEFINE_GRAPH_NODE(GraphOutputNode);

void GraphOutputNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Float", DatumType::Float, Datum(0.0f));
    AddInputPin("Integer", DatumType::Integer, Datum(0));
    AddInputPin("Bool", DatumType::Bool, Datum(false));
    AddInputPin("Vector", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
}

void GraphOutputNode::Evaluate()
{
    // Sink node - values are read back by scripts via NodeGraphPlayer::GetOutput*()
}

glm::vec4 GraphOutputNode::GetNodeColor() const { return kGraphOutputColor; }

// =============================================================================
// SceneGraphOutputNode (existing)
// =============================================================================
DEFINE_GRAPH_NODE(SceneGraphOutputNode);

void SceneGraphOutputNode::SetupPins()
{
    AddInputPin("Transform", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Active", DatumType::Bool, Datum(true));
}

void SceneGraphOutputNode::Evaluate()
{
    // Sink node - collects scene graph output parameters.
}

glm::vec4 SceneGraphOutputNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetTransformNode (existing)
// =============================================================================
DEFINE_GRAPH_NODE(GetTransformNode);

void GetTransformNode::SetupPins()
{
    AddOutputPin("Position", DatumType::Vector);
    AddOutputPin("Rotation", DatumType::Vector);
    AddOutputPin("Scale", DatumType::Vector);
}

void GetTransformNode::Evaluate()
{
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
    SetOutputValue(1, Datum(glm::vec3(0.0f)));
    SetOutputValue(2, Datum(glm::vec3(1.0f)));
}

glm::vec4 GetTransformNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetTransformNode (existing)
// =============================================================================
DEFINE_GRAPH_NODE(SetTransformNode);

void SetTransformNode::SetupPins()
{
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Rotation", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scale", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddOutputPin("Transform", DatumType::Vector);
}

void SetTransformNode::Evaluate()
{
    glm::vec3 pos = GetInputValue(0).GetVector();
    SetOutputValue(0, Datum(pos));
}

glm::vec4 SetTransformNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// FindNodeNode (existing)
// =============================================================================
DEFINE_GRAPH_NODE(FindNodeNode);

void FindNodeNode::SetupPins()
{
    AddOutputPin("Found", DatumType::Bool);
}

void FindNodeNode::Evaluate()
{
    SetOutputValue(0, Datum(false));
}

glm::vec4 FindNodeNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetSelfNode
// =============================================================================
DEFINE_GRAPH_NODE(GetSelfNode);

void GetSelfNode::SetupPins()
{
    AddOutputPin("Self", DatumType::Node3D);
}

void GetSelfNode::Evaluate()
{
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr)
    {
        SetOutputValue(0, Datum(owner));
    }
}

glm::vec4 GetSelfNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetParentGraphNode
// =============================================================================
DEFINE_GRAPH_NODE(GetParentGraphNode);

void GetParentGraphNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddOutputPin("Parent", DatumType::Node);
}

void GetParentGraphNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    if (node != nullptr)
    {
        Node* parent = node->GetParent();
        SetOutputValue(0, Datum(parent));
    }
}

glm::vec4 GetParentGraphNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// FindInSceneNode
// =============================================================================
DEFINE_GRAPH_NODE(FindInSceneNode);

void FindInSceneNode::SetupPins()
{
    AddInputPin("Name", DatumType::String, Datum(std::string("")));
    AddOutputPin("Node", DatumType::Node);
    AddOutputPin("Found", DatumType::Bool);
}

void FindInSceneNode::Evaluate()
{
    std::string name = GetInputValue(0).GetString();
    Node* owner = GetOwnerNode(this);

    if (owner != nullptr && !name.empty())
    {
        World* world = owner->GetWorld();
        if (world != nullptr)
        {
            Node* found = world->FindNode(name);
            SetOutputValue(0, Datum(found));
            SetOutputValue(1, Datum(found != nullptr));
            return;
        }
    }
    SetOutputValue(0, Datum((Node*)nullptr));
    SetOutputValue(1, Datum(false));
}

glm::vec4 FindInSceneNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// FindChildGraphNode
// =============================================================================
DEFINE_GRAPH_NODE(FindChildGraphNode);

void FindChildGraphNode::SetupPins()
{
    AddInputPin("Parent", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Name", DatumType::String, Datum(std::string("")));
    AddInputPin("Recurse", DatumType::Bool, Datum(false));
    AddOutputPin("Child", DatumType::Node);
    AddOutputPin("Found", DatumType::Bool);
}

void FindChildGraphNode::Evaluate()
{
    Node* parent = GetInputValue(0).GetNode().Get();
    std::string name = GetInputValue(1).GetString();
    bool recurse = GetInputValue(2).GetBool();

    if (parent != nullptr && !name.empty())
    {
        Node* child = parent->FindChild(name, recurse);
        SetOutputValue(0, Datum(child));
        SetOutputValue(1, Datum(child != nullptr));
        return;
    }
    SetOutputValue(0, Datum((Node*)nullptr));
    SetOutputValue(1, Datum(false));
}

glm::vec4 FindChildGraphNode::GetNodeColor() const { return kSceneGraphNodeColor; }

DEFINE_GRAPH_NODE(GetNode3DName)
void GetNode3DName::SetupPins() {
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddOutputPin("Name", DatumType::String);
}
void GetNode3DName::Evaluate() {
    Node* node = GetInputValue(0).GetNode().Get();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            SetOutputValue(0, Datum(node3d->GetName()));
            return;
        }
    }
    SetOutputValue(0, Datum(""));
}
glm::vec4 GetNode3DName::GetNodeColor() const { return kSceneGraphNodeColor; }

DEFINE_GRAPH_NODE(GetNode3DClass)
void GetNode3DClass::SetupPins() {
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddOutputPin("Name", DatumType::String);
}
void GetNode3DClass::Evaluate() {
    Node* node = GetInputValue(0).GetNode().Get();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            SetOutputValue(0, Datum(node3d->GetClassName()));
            return;
        }
    }
    SetOutputValue(0, Datum(""));
}
glm::vec4 GetNode3DClass::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetPositionNode
// =============================================================================
DEFINE_GRAPH_NODE(GetPositionNode);

void GetPositionNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("World", DatumType::Bool, Datum(false));
    AddOutputPin("Position", DatumType::Vector);
}

void GetPositionNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    bool world = GetInputValue(1).GetBool();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            glm::vec3 pos = world ? node3d->GetWorldPosition() : node3d->GetPosition();
            SetOutputValue(0, Datum(pos));
            return;
        }
    }
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
}

glm::vec4 GetPositionNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetRotationNode
// =============================================================================
DEFINE_GRAPH_NODE(GetRotationNode);

void GetRotationNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("World", DatumType::Bool, Datum(false));
    AddOutputPin("Rotation", DatumType::Vector);
}

void GetRotationNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    bool world = GetInputValue(1).GetBool();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            glm::vec3 rot = world ? node3d->GetWorldRotationEuler() : node3d->GetRotationEuler();
            SetOutputValue(0, Datum(rot));
            return;
        }
    }
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
}

glm::vec4 GetRotationNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// GetScaleNode
// =============================================================================
DEFINE_GRAPH_NODE(GetScaleNode);

void GetScaleNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("World", DatumType::Bool, Datum(false));
    AddOutputPin("Scale", DatumType::Vector);
}

void GetScaleNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    bool world = GetInputValue(1).GetBool();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            glm::vec3 scale = world ? node3d->GetWorldScale() : node3d->GetScale();
            SetOutputValue(0, Datum(scale));
            return;
        }
    }
    SetOutputValue(0, Datum(glm::vec3(1.0f)));
}

glm::vec4 GetScaleNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetPositionNode
// =============================================================================
DEFINE_GRAPH_NODE(SetPositionNode);

void SetPositionNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("World", DatumType::Bool, Datum(false));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Node", DatumType::Node3D);
}

void SetPositionNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    glm::vec3 pos = GetInputValue(2).GetVector();
    bool world = GetInputValue(3).GetBool();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            if (world)
                node3d->SetWorldPosition(pos);
            else
                node3d->SetPosition(pos);
        }
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(node));
}

glm::vec4 SetPositionNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetRotationNode
// =============================================================================
DEFINE_GRAPH_NODE(SetRotationNode);

void SetRotationNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Rotation", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("World", DatumType::Bool, Datum(false));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Node", DatumType::Node3D);
}

void SetRotationNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    glm::vec3 rot = GetInputValue(2).GetVector();
    bool world = GetInputValue(3).GetBool();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            if (world)
                node3d->SetWorldRotation(rot);
            else
                node3d->SetRotation(rot);
        }
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(node));
}

glm::vec4 SetRotationNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetScaleNode
// =============================================================================
DEFINE_GRAPH_NODE(SetScaleNode);

void SetScaleNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Scale", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddInputPin("World", DatumType::Bool, Datum(false));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Node", DatumType::Node3D);
}

void SetScaleNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    glm::vec3 scale = GetInputValue(2).GetVector();
    bool world = GetInputValue(3).GetBool();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            if (world)
                node3d->SetWorldScale(scale);
            else
                node3d->SetScale(scale);
        }
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(node));
}

glm::vec4 SetScaleNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetVisibilityNode
// =============================================================================
DEFINE_GRAPH_NODE(SetVisibilityNode);

void SetVisibilityNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Visible", DatumType::Bool, Datum(true));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Node", DatumType::Node);
}

void SetVisibilityNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    bool visible = GetInputValue(2).GetBool();

    if (node != nullptr)
    {
        node->SetVisible(visible);
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(node));
}

glm::vec4 SetVisibilityNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// SetActiveGraphNode
// =============================================================================
DEFINE_GRAPH_NODE(SetActiveGraphNode);

void SetActiveGraphNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Active", DatumType::Bool, Datum(true));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Node", DatumType::Node);
}

void SetActiveGraphNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    bool active = GetInputValue(2).GetBool();

    if (node != nullptr)
    {
        node->SetActive(active);
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(node));
}

glm::vec4 SetActiveGraphNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// DistanceBetweenNodesNode
// =============================================================================
DEFINE_GRAPH_NODE(DistanceBetweenNodesNode);

void DistanceBetweenNodesNode::SetupPins()
{
    AddInputPin("Node A", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Node B", DatumType::Node3D, Datum((Node*)nullptr));
    AddOutputPin("Distance", DatumType::Float);
    AddOutputPin("Direction", DatumType::Vector);
}

void DistanceBetweenNodesNode::Evaluate()
{
    Node* nodeA = GetInputValue(0).GetNode().Get();
    Node* nodeB = GetInputValue(1).GetNode().Get();

    if (nodeA != nullptr && nodeB != nullptr)
    {
        Node3D* a3d = nodeA->As<Node3D>();
        Node3D* b3d = nodeB->As<Node3D>();

        if (a3d != nullptr && b3d != nullptr)
        {
            glm::vec3 posA = a3d->GetWorldPosition();
            glm::vec3 posB = b3d->GetWorldPosition();
            glm::vec3 diff = posB - posA;
            float dist = glm::length(diff);
            glm::vec3 dir = dist > 0.0001f ? diff / dist : glm::vec3(0.0f);

            SetOutputValue(0, Datum(dist));
            SetOutputValue(1, Datum(dir));
            return;
        }
    }
    SetOutputValue(0, Datum(0.0f));
    SetOutputValue(1, Datum(glm::vec3(0.0f)));
}

glm::vec4 DistanceBetweenNodesNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// RandomPointNearNodeNode
// =============================================================================
DEFINE_GRAPH_NODE(RandomPointNearNodeNode);

void RandomPointNearNodeNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Radius", DatumType::Float, Datum(1.0f));
    AddOutputPin("Position", DatumType::Vector);
}

void RandomPointNearNodeNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    float radius = GetInputValue(1).GetFloat();

    if (node != nullptr)
    {
        Node3D* node3d = node->As<Node3D>();
        if (node3d != nullptr)
        {
            glm::vec3 pos = node3d->GetWorldPosition();
            float angle = ((float)rand() / RAND_MAX) * 2.0f * 3.14159265f;
            float r = ((float)rand() / RAND_MAX) * radius;
            pos.x += r * cosf(angle);
            pos.z += r * sinf(angle);
            SetOutputValue(0, Datum(pos));
            return;
        }
    }
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
}

glm::vec4 RandomPointNearNodeNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// InstantiateSceneNode
// =============================================================================
DEFINE_GRAPH_NODE(InstantiateSceneNode);

void InstantiateSceneNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Scene", DatumType::Scene, Datum((Asset*)nullptr));
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Spawned", DatumType::Node);
}

void InstantiateSceneNode::Evaluate()
{
    Asset* asset = GetInputValue(1).GetAsset();
    glm::vec3 pos = GetInputValue(2).GetVector();

    Node* spawned = nullptr;
    Node* owner = GetOwnerNode(this);
    if (owner != nullptr && asset != nullptr)
    {
        Scene* scene = asset->As<Scene>();
        World* world = owner->GetWorld();
        if (scene != nullptr && world != nullptr)
        {
            spawned = world->SpawnScene(scene, pos);
        }
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(spawned));
}

glm::vec4 InstantiateSceneNode::GetNodeColor() const { return kSceneGraphNodeColor; }

// =============================================================================
// Attribute Set Nodes
// =============================================================================

// --- SetAttributeIntNode ---
DEFINE_GRAPH_NODE(SetAttributeIntNode);

void SetAttributeIntNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::Integer, Datum(0));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAttributeIntNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string key = GetInputValue(2).GetString();
    int32_t value = GetInputValue(3).GetInteger();

    if (node != nullptr && !key.empty())
    {
        node->SetField(key, Datum(value));
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAttributeIntNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- SetAttributeFloatNode ---
DEFINE_GRAPH_NODE(SetAttributeFloatNode);

void SetAttributeFloatNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::Float, Datum(0.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAttributeFloatNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string key = GetInputValue(2).GetString();
    float value = GetInputValue(3).GetFloat();

    if (node != nullptr && !key.empty())
    {
        node->SetField(key, Datum(value));
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAttributeFloatNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- SetAttributeVectorNode ---
DEFINE_GRAPH_NODE(SetAttributeVectorNode);

void SetAttributeVectorNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAttributeVectorNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string key = GetInputValue(2).GetString();
    glm::vec3 value = GetInputValue(3).GetVector();

    if (node != nullptr && !key.empty())
    {
        node->SetField(key, Datum(value));
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAttributeVectorNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- SetAttributeStringNode ---
DEFINE_GRAPH_NODE(SetAttributeStringNode);

void SetAttributeStringNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAttributeStringNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string key = GetInputValue(2).GetString();
    std::string value = GetInputValue(3).GetString();

    if (node != nullptr && !key.empty())
    {
        node->SetField(key, Datum(value));
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAttributeStringNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- SetAttributeBoolNode ---
DEFINE_GRAPH_NODE(SetAttributeBoolNode);

void SetAttributeBoolNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::Bool, Datum(false));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAttributeBoolNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string key = GetInputValue(2).GetString();
    bool value = GetInputValue(3).GetBool();

    if (node != nullptr && !key.empty())
    {
        node->SetField(key, Datum(value));
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAttributeBoolNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- SetAttributeColorNode ---
DEFINE_GRAPH_NODE(SetAttributeColorNode);

void SetAttributeColorNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAttributeColorNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string key = GetInputValue(2).GetString();
    glm::vec4 value = GetInputValue(3).GetColor();

    if (node != nullptr && !key.empty())
    {
        node->SetField(key, Datum(value));
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAttributeColorNode::GetNodeColor() const { return kAttributeNodeColor; }

// =============================================================================
// Attribute Get Nodes
// =============================================================================

// --- GetAttributeIntNode ---
DEFINE_GRAPH_NODE(GetAttributeIntNode);

void GetAttributeIntNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::Integer);
}

void GetAttributeIntNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    std::string key = GetInputValue(1).GetString();

    if (node != nullptr && !key.empty())
    {
        Datum field = node->GetField(key);
        SetOutputValue(0, Datum(field.GetInteger()));
        return;
    }
    SetOutputValue(0, Datum(0));
}

glm::vec4 GetAttributeIntNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- GetAttributeFloatNode ---
DEFINE_GRAPH_NODE(GetAttributeFloatNode);

void GetAttributeFloatNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::Float);
}

void GetAttributeFloatNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    std::string key = GetInputValue(1).GetString();

    if (node != nullptr && !key.empty())
    {
        Datum field = node->GetField(key);
        SetOutputValue(0, Datum(field.GetFloat()));
        return;
    }
    SetOutputValue(0, Datum(0.0f));
}

glm::vec4 GetAttributeFloatNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- GetAttributeVectorNode ---
DEFINE_GRAPH_NODE(GetAttributeVectorNode);

void GetAttributeVectorNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::Vector);
}

void GetAttributeVectorNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    std::string key = GetInputValue(1).GetString();

    if (node != nullptr && !key.empty())
    {
        Datum field = node->GetField(key);
        SetOutputValue(0, Datum(field.GetVector()));
        return;
    }
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
}

glm::vec4 GetAttributeVectorNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- GetAttributeStringNode ---
DEFINE_GRAPH_NODE(GetAttributeStringNode);

void GetAttributeStringNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::String);
}

void GetAttributeStringNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    std::string key = GetInputValue(1).GetString();

    if (node != nullptr && !key.empty())
    {
        Datum field = node->GetField(key);
        SetOutputValue(0, Datum(field.GetString()));
        return;
    }
    SetOutputValue(0, Datum(std::string("")));
}

glm::vec4 GetAttributeStringNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- GetAttributeBoolNode ---
DEFINE_GRAPH_NODE(GetAttributeBoolNode);

void GetAttributeBoolNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::Bool);
}

void GetAttributeBoolNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    std::string key = GetInputValue(1).GetString();

    if (node != nullptr && !key.empty())
    {
        Datum field = node->GetField(key);
        SetOutputValue(0, Datum(field.GetBool()));
        return;
    }
    SetOutputValue(0, Datum(false));
}

glm::vec4 GetAttributeBoolNode::GetNodeColor() const { return kAttributeNodeColor; }

// --- GetAttributeColorNode ---
DEFINE_GRAPH_NODE(GetAttributeColorNode);

void GetAttributeColorNode::SetupPins()
{
    AddInputPin("Node", DatumType::Node, Datum((Node*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddOutputPin("Value", DatumType::Color);
}

void GetAttributeColorNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();
    std::string key = GetInputValue(1).GetString();

    if (node != nullptr && !key.empty())
    {
        Datum field = node->GetField(key);
        SetOutputValue(0, Datum(field.GetColor()));
        return;
    }
    SetOutputValue(0, Datum(glm::vec4(0.0f)));
}

glm::vec4 GetAttributeColorNode::GetNodeColor() const { return kAttributeNodeColor; }

// =============================================================================
// Widget Nodes
// =============================================================================

// --- SetTextNode ---
DEFINE_GRAPH_NODE(SetTextNode);

void SetTextNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Text, Datum((Node*)nullptr));
    AddInputPin("Text", DatumType::String, Datum(std::string("")));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetTextNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string text = GetInputValue(2).GetString();

    if (node != nullptr)
    {
        Text* textNode = node->As<Text>();
        if (textNode != nullptr)
        {
            textNode->SetText(text);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetTextNode::GetNodeColor() const { return kWidgetNodeColor; }

// --- GetTextNode ---
DEFINE_GRAPH_NODE(GetTextNode);

void GetTextNode::SetupPins()
{
    AddInputPin("Node", DatumType::Text, Datum((Node*)nullptr));
    AddOutputPin("Value", DatumType::String);
}

void GetTextNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();

    if (node != nullptr)
    {
        Text* textNode = node->As<Text>();
        if (textNode != nullptr)
        {
            SetOutputValue(0, Datum(textNode->GetText()));
            return;
        }
    }
    SetOutputValue(0, Datum(std::string("")));
}

glm::vec4 GetTextNode::GetNodeColor() const { return kWidgetNodeColor; }

// --- SetSpriteNode ---
DEFINE_GRAPH_NODE(SetSpriteNode);

void SetSpriteNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Quad, Datum((Node*)nullptr));
    AddInputPin("Texture", DatumType::Asset, Datum((Asset*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetSpriteNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    Asset* asset = GetInputValue(2).GetAsset();

    if (node != nullptr && asset != nullptr)
    {
        Quad* quad = node->As<Quad>();
        Texture* texture = asset->As<Texture>();
        if (quad != nullptr && texture != nullptr)
        {
            quad->SetTexture(texture);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetSpriteNode::GetNodeColor() const { return kWidgetNodeColor; }

// --- SetSizeNode ---
DEFINE_GRAPH_NODE(SetSizeNode);

void SetSizeNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Widget, Datum((Node*)nullptr));
    AddInputPin("Width", DatumType::Float, Datum(100.0f));
    AddInputPin("Height", DatumType::Float, Datum(100.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetSizeNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    float width = GetInputValue(2).GetFloat();
    float height = GetInputValue(3).GetFloat();

    if (node != nullptr)
    {
        Widget* widget = node->As<Widget>();
        if (widget != nullptr)
        {
            widget->SetDimensions(width, height);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetSizeNode::GetNodeColor() const { return kWidgetNodeColor; }

// --- GetSizeNode ---
DEFINE_GRAPH_NODE(GetSizeNode);

void GetSizeNode::SetupPins()
{
    AddInputPin("Node", DatumType::Widget, Datum((Node*)nullptr));
    AddOutputPin("Width", DatumType::Float);
    AddOutputPin("Height", DatumType::Float);
}

void GetSizeNode::Evaluate()
{
    Node* node = GetInputValue(0).GetNode().Get();

    if (node != nullptr)
    {
        Widget* widget = node->As<Widget>();
        if (widget != nullptr)
        {
            SetOutputValue(0, Datum(widget->GetWidth()));
            SetOutputValue(1, Datum(widget->GetHeight()));
            return;
        }
    }
    SetOutputValue(0, Datum(0.0f));
    SetOutputValue(1, Datum(0.0f));
}

glm::vec4 GetSizeNode::GetNodeColor() const { return kWidgetNodeColor; }

// --- SetWidgetColorNode ---
DEFINE_GRAPH_NODE(SetWidgetColorNode);

void SetWidgetColorNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Widget, Datum((Node*)nullptr));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetWidgetColorNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    glm::vec4 color = GetInputValue(2).GetColor();

    if (node != nullptr)
    {
        Widget* widget = node->As<Widget>();
        if (widget != nullptr)
        {
            widget->SetColor(color);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetWidgetColorNode::GetNodeColor() const { return kWidgetNodeColor; }

// =============================================================================
// UI Document Nodes
// =============================================================================

static const glm::vec4 kUINodeColor = glm::vec4(0.2f, 0.8f, 0.6f, 1.0f);

// --- LoadUIDocumentNode ---
DEFINE_GRAPH_NODE(LoadUIDocumentNode);

LoadUIDocumentNode::~LoadUIDocumentNode()
{
    if (mDocument != nullptr)
    {
        mDocument->Unmount();
        mDocument->Destroy();
        delete mDocument;
        mDocument = nullptr;
    }
}

void LoadUIDocumentNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Path", DatumType::String, Datum(std::string("")));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Document", DatumType::Asset);
}

void LoadUIDocumentNode::Evaluate()
{
    std::string path = GetInputValue(1).GetString();

    if (!path.empty())
    {
        // Clean up previous document
        if (mDocument != nullptr)
        {
            mDocument->Unmount();
            mDocument->Destroy();
            delete mDocument;
            mDocument = nullptr;
        }

        // Resolve relative paths against the project directory
        std::string fullPath = GetEngineState()->mProjectDirectory + path;

        mDocument = new UIDocument();
        mDocument->Create();

        if (mDocument->Import(fullPath, nullptr))
        {
            SetOutputValue(1, Datum((Asset*)mDocument));
        }
        else
        {
            LogError("LoadUIDocumentNode: Failed to import '%s'", path.c_str());
            delete mDocument;
            mDocument = nullptr;
            SetOutputValue(1, Datum((Asset*)nullptr));
        }
    }
    else
    {
        SetOutputValue(1, Datum((Asset*)nullptr));
    }

    TriggerExecutionPin(0);
}

glm::vec4 LoadUIDocumentNode::GetNodeColor() const { return kUINodeColor; }

// --- MountUINode ---
DEFINE_GRAPH_NODE(MountUINode);

void MountUINode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddInputPin("Parent", DatumType::Widget, Datum((Node*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void MountUINode::Evaluate()
{
    Asset* asset = GetInputValue(1).GetAsset();
    Node* parentNode = GetInputValue(2).GetNode().Get();

    if (asset != nullptr && parentNode != nullptr)
    {
        UIDocument* doc = asset->As<UIDocument>();
        Widget* parent = parentNode->As<Widget>();
        if (doc != nullptr && parent != nullptr)
        {
            doc->Mount(parent);
        }
    }

    TriggerExecutionPin(0);
}

glm::vec4 MountUINode::GetNodeColor() const { return kUINodeColor; }

// --- UnmountUINode ---
DEFINE_GRAPH_NODE(UnmountUINode);

void UnmountUINode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void UnmountUINode::Evaluate()
{
    Asset* asset = GetInputValue(1).GetAsset();

    if (asset != nullptr)
    {
        UIDocument* doc = asset->As<UIDocument>();
        if (doc != nullptr)
        {
            doc->Unmount();
        }
    }

    TriggerExecutionPin(0);
}

glm::vec4 UnmountUINode::GetNodeColor() const { return kUINodeColor; }

// --- SetUIDataNode ---
DEFINE_GRAPH_NODE(SetUIDataNode);

void SetUIDataNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddInputPin("Key", DatumType::String, Datum(std::string("")));
    AddInputPin("Value", DatumType::String, Datum(std::string("")));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetUIDataNode::Evaluate()
{
    Asset* asset = GetInputValue(1).GetAsset();
    std::string key = GetInputValue(2).GetString();
    std::string value = GetInputValue(3).GetString();

    if (asset != nullptr && !key.empty())
    {
        UIDocument* doc = asset->As<UIDocument>();
        if (doc != nullptr)
        {
            doc->SetData(key, value);
        }
    }

    TriggerExecutionPin(0);
}

glm::vec4 SetUIDataNode::GetNodeColor() const { return kUINodeColor; }

// --- UIFindByIdNode ---
DEFINE_GRAPH_NODE(UIFindByIdNode);

void UIFindByIdNode::SetupPins()
{
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddInputPin("Id", DatumType::String, Datum(std::string("")));
    AddOutputPin("Widget", DatumType::Widget);
}

void UIFindByIdNode::Evaluate()
{
    Asset* asset = GetInputValue(0).GetAsset();
    std::string id = GetInputValue(1).GetString();

    Widget* result = nullptr;

    if (asset != nullptr && !id.empty())
    {
        UIDocument* doc = asset->As<UIDocument>();
        if (doc != nullptr)
        {
            result = doc->FindById(id);
        }
    }

    SetOutputValue(0, Datum((Node*)result));
}

glm::vec4 UIFindByIdNode::GetNodeColor() const { return kUINodeColor; }

// --- UIGetRootWidgetNode ---
DEFINE_GRAPH_NODE(UIGetRootWidgetNode);

void UIGetRootWidgetNode::SetupPins()
{
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddOutputPin("Root", DatumType::Widget);
}

void UIGetRootWidgetNode::Evaluate()
{
    Asset* asset = GetInputValue(0).GetAsset();
    Widget* root = nullptr;

    if (asset != nullptr)
    {
        UIDocument* doc = asset->As<UIDocument>();
        if (doc != nullptr)
        {
            root = doc->GetRootWidget();
        }
    }

    SetOutputValue(0, Datum((Node*)root));
}

glm::vec4 UIGetRootWidgetNode::GetNodeColor() const { return kUINodeColor; }

// --- UITickNode ---
DEFINE_GRAPH_NODE(UITickNode);

void UITickNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void UITickNode::Evaluate()
{
    Asset* asset = GetInputValue(1).GetAsset();

    if (asset != nullptr)
    {
        UIDocument* doc = asset->As<UIDocument>();
        if (doc != nullptr)
        {
            doc->Tick();
        }
    }

    TriggerExecutionPin(0);
}

glm::vec4 UITickNode::GetNodeColor() const { return kUINodeColor; }

// --- InstantiateUINode ---
DEFINE_GRAPH_NODE(InstantiateUINode);

void InstantiateUINode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Document", DatumType::Asset, Datum((Asset*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Root", DatumType::Widget);
}

void InstantiateUINode::Evaluate()
{
    Asset* asset = GetInputValue(1).GetAsset();
    Widget* root = nullptr;

    if (asset != nullptr)
    {
        UIDocument* doc = asset->As<UIDocument>();
        if (doc != nullptr)
        {
            root = doc->Instantiate();
        }
    }

    SetOutputValue(1, Datum((Node*)root));
    TriggerExecutionPin(0);
}

glm::vec4 InstantiateUINode::GetNodeColor() const { return kUINodeColor; }

// =============================================================================
// Audio Nodes
// =============================================================================

// --- SetAudioClipNode ---
DEFINE_GRAPH_NODE(SetAudioClipNode);

void SetAudioClipNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddInputPin("Clip", DatumType::Asset, Datum((Asset*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAudioClipNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    Asset* asset = GetInputValue(2).GetAsset();

    if (node != nullptr && asset != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        SoundWave* soundWave = asset->As<SoundWave>();
        if (audio != nullptr && soundWave != nullptr)
        {
            audio->SetSoundWave(soundWave);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAudioClipNode::GetNodeColor() const { return kAudioNodeColor; }

// --- SetAudioTimeNode ---
DEFINE_GRAPH_NODE(SetAudioTimeNode);

void SetAudioTimeNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddInputPin("Time", DatumType::Float, Datum(0.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetAudioTimeNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    float time = GetInputValue(2).GetFloat();

    if (node != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        if (audio != nullptr)
        {
            audio->SetStartOffset(time);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetAudioTimeNode::GetNodeColor() const { return kAudioNodeColor; }

// --- SetVolumeNode ---
DEFINE_GRAPH_NODE(SetVolumeNode);

void SetVolumeNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddInputPin("Volume", DatumType::Float, Datum(1.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetVolumeNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    float volume = GetInputValue(2).GetFloat();

    if (node != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        if (audio != nullptr)
        {
            audio->SetVolume(volume);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetVolumeNode::GetNodeColor() const { return kAudioNodeColor; }

// --- SetPitchNode ---
DEFINE_GRAPH_NODE(SetPitchNode);

void SetPitchNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddInputPin("Pitch", DatumType::Float, Datum(1.0f));
    AddOutputPin("Exec", DatumType::Execution);
}

void SetPitchNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    float pitch = GetInputValue(2).GetFloat();

    if (node != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        if (audio != nullptr)
        {
            audio->SetPitch(pitch);
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 SetPitchNode::GetNodeColor() const { return kAudioNodeColor; }

// --- PlayAudioNode ---
DEFINE_GRAPH_NODE(PlayAudioNode);

void PlayAudioNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Playing", DatumType::Bool);
}

void PlayAudioNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();

    bool playing = false;
    if (node != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        if (audio != nullptr)
        {
            audio->PlayAudio();
            playing = audio->IsPlaying();
        }
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(playing));
}

glm::vec4 PlayAudioNode::GetNodeColor() const { return kAudioNodeColor; }

// --- StopAudioNode ---
DEFINE_GRAPH_NODE(StopAudioNode);

void StopAudioNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void StopAudioNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();

    if (node != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        if (audio != nullptr)
        {
            audio->StopAudio();
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 StopAudioNode::GetNodeColor() const { return kAudioNodeColor; }

// --- PauseAudioNode ---
DEFINE_GRAPH_NODE(PauseAudioNode);

void PauseAudioNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Audio3D, Datum((Node*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void PauseAudioNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();

    if (node != nullptr)
    {
        Audio3D* audio = node->As<Audio3D>();
        if (audio != nullptr)
        {
            audio->PauseAudio();
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 PauseAudioNode::GetNodeColor() const { return kAudioNodeColor; }

// =============================================================================
// Animation Nodes
// =============================================================================

// --- PlayAnimationNode ---
DEFINE_GRAPH_NODE(PlayAnimationNode);

void PlayAnimationNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Animation", DatumType::String, Datum(std::string("")));
    AddInputPin("Loop", DatumType::Bool, Datum(false));
    AddInputPin("Speed", DatumType::Float, Datum(1.0f));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Playing", DatumType::Bool);
}

void PlayAnimationNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string animName = GetInputValue(2).GetString();
    bool loop = GetInputValue(3).GetBool();
    float speed = GetInputValue(4).GetFloat();

    bool playing = false;
    if (node != nullptr && !animName.empty())
    {
        SkeletalMesh3D* skelMesh = node->As<SkeletalMesh3D>();
        if (skelMesh != nullptr)
        {
            skelMesh->PlayAnimation(animName.c_str(), loop, speed);
            playing = skelMesh->IsAnimationPlaying(animName.c_str());
        }
    }

    TriggerExecutionPin(0);
    SetOutputValue(1, Datum(playing));
}

glm::vec4 PlayAnimationNode::GetNodeColor() const { return kAnimationNodeColor; }

// --- StopAnimationNode ---
DEFINE_GRAPH_NODE(StopAnimationNode);

void StopAnimationNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Node", DatumType::Node3D, Datum((Node*)nullptr));
    AddInputPin("Animation", DatumType::String, Datum(std::string("")));
    AddOutputPin("Exec", DatumType::Execution);
}

void StopAnimationNode::Evaluate()
{
    Node* node = GetInputValue(1).GetNode().Get();
    std::string animName = GetInputValue(2).GetString();

    if (node != nullptr && !animName.empty())
    {
        SkeletalMesh3D* skelMesh = node->As<SkeletalMesh3D>();
        if (skelMesh != nullptr)
        {
            skelMesh->StopAnimation(animName.c_str());
        }
    }
    TriggerExecutionPin(0);
}

glm::vec4 StopAnimationNode::GetNodeColor() const { return kAnimationNodeColor; }

// =============================================================================
// Flow Control Nodes
// =============================================================================

// --- ForLoopNode ---
DEFINE_GRAPH_NODE(ForLoopNode);

void ForLoopNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Start", DatumType::Integer, Datum(0));
    AddInputPin("End", DatumType::Integer, Datum(10));
    AddOutputPin("Loop Body", DatumType::Execution);
    AddOutputPin("Completed", DatumType::Execution);
    AddOutputPin("Index", DatumType::Integer);
}

void ForLoopNode::Evaluate()
{
    int32_t start = GetInputValue(1).GetInteger();
    int32_t end = GetInputValue(2).GetInteger();

    for (int32_t i = start; i < end; ++i)
    {
        SetOutputValue(2, Datum(i));
        TriggerExecutionPin(0); // Loop Body
    }

    TriggerExecutionPin(1); // Completed
}

glm::vec4 ForLoopNode::GetNodeColor() const { return kFlowNodeColor; }

// --- ForEachLoopNode ---
DEFINE_GRAPH_NODE(ForEachLoopNode);

void ForEachLoopNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Count", DatumType::Integer, Datum(0));
    AddOutputPin("Loop Body", DatumType::Execution);
    AddOutputPin("Completed", DatumType::Execution);
    AddOutputPin("Index", DatumType::Integer);
}

void ForEachLoopNode::Evaluate()
{
    int32_t count = GetInputValue(1).GetInteger();

    for (int32_t i = 0; i < count; ++i)
    {
        SetOutputValue(2, Datum(i));
        TriggerExecutionPin(0); // Loop Body
    }

    TriggerExecutionPin(1); // Completed
}

glm::vec4 ForEachLoopNode::GetNodeColor() const { return kFlowNodeColor; }

// --- WaitTillNode ---
DEFINE_GRAPH_NODE(WaitTillNode);

void WaitTillNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Duration", DatumType::Float, Datum(1.0f));
    AddInputPin("Reset", DatumType::Bool, Datum(false));
    AddOutputPin("Completed", DatumType::Execution);
    AddOutputPin("Elapsed", DatumType::Float);
    AddOutputPin("Progress", DatumType::Float);
}

void WaitTillNode::Evaluate()
{
    float duration = GetInputValue(1).GetFloat();
    bool reset = GetInputValue(2).GetBool();
    float currentTime = GetAppClock()->GetTime();

    if (reset || mStartTime < 0.0f)
    {
        mStartTime = currentTime;
    }

    float elapsed = currentTime - mStartTime;
    float progress = (duration > 0.0f) ? glm::clamp(elapsed / duration, 0.0f, 1.0f) : 1.0f;

    SetOutputValue(1, Datum(elapsed));
    SetOutputValue(2, Datum(progress));

    if (elapsed >= duration)
    {
        TriggerExecutionPin(0); // Completed
        mStartTime = -1.0f;
    }
}

glm::vec4 WaitTillNode::GetNodeColor() const { return kFlowNodeColor; }

// --- BranchNode ---
DEFINE_GRAPH_NODE(BranchNode);

void BranchNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Condition", DatumType::Bool, Datum(true));
    AddOutputPin("True", DatumType::Execution);
    AddOutputPin("False", DatumType::Execution);
}

void BranchNode::Evaluate()
{
    bool condition = GetInputValue(1).GetBool();

    if (condition)
    {
        TriggerExecutionPin(0); // True
    }
    else
    {
        TriggerExecutionPin(1); // False
    }
}

glm::vec4 BranchNode::GetNodeColor() const { return kFlowNodeColor; }

// --- SequenceNode ---
DEFINE_GRAPH_NODE(SequenceNode);

void SequenceNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddOutputPin("Then 0", DatumType::Execution);
    AddOutputPin("Then 1", DatumType::Execution);
    AddOutputPin("Then 2", DatumType::Execution);
    AddOutputPin("Then 3", DatumType::Execution);
}

void SequenceNode::Evaluate()
{
    TriggerExecutionPin(0);
    TriggerExecutionPin(1);
    TriggerExecutionPin(2);
    TriggerExecutionPin(3);
}

glm::vec4 SequenceNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// WhileLoopNode
// =============================================================================

DEFINE_GRAPH_NODE(WhileLoopNode);

void WhileLoopNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Condition", DatumType::Bool, Datum(true));
    AddInputPin("Max Iterations", DatumType::Integer, Datum(1000));
    AddOutputPin("Loop Body", DatumType::Execution);
    AddOutputPin("Completed", DatumType::Execution);
    AddOutputPin("Index", DatumType::Integer);
}

void WhileLoopNode::Evaluate()
{
    int32_t maxIter = GetInputValue(2).GetInteger();
    for (int32_t i = 0; i < maxIter; ++i)
    {
        bool cond = GetInputValue(1).GetBool();
        if (!cond)
            break;
        SetOutputValue(2, Datum(i));
        TriggerExecutionPin(0);
    }
    TriggerExecutionPin(1);
}

glm::vec4 WhileLoopNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// DoOnceNode
// =============================================================================

DEFINE_GRAPH_NODE(DoOnceNode);

void DoOnceNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Reset", DatumType::Bool, Datum(false));
    AddOutputPin("Completed", DatumType::Execution);
}

void DoOnceNode::Evaluate()
{
    bool reset = GetInputValue(1).GetBool();
    if (reset)
        mHasFired = false;
    if (!mHasFired)
    {
        mHasFired = true;
        TriggerExecutionPin(0);
    }
}

glm::vec4 DoOnceNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// FlipFlopNode
// =============================================================================

DEFINE_GRAPH_NODE(FlipFlopNode);

void FlipFlopNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddOutputPin("A", DatumType::Execution);
    AddOutputPin("B", DatumType::Execution);
    AddOutputPin("Is A", DatumType::Bool);
}

void FlipFlopNode::Evaluate()
{
    if (mIsA)
    {
        TriggerExecutionPin(0);
    }
    else
    {
        TriggerExecutionPin(1);
    }
    SetOutputValue(2, Datum(mIsA));
    mIsA = !mIsA;
}

glm::vec4 FlipFlopNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// GateNode
// =============================================================================

DEFINE_GRAPH_NODE(GateNode);

void GateNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Open", DatumType::Bool, Datum(true));
    AddOutputPin("Exec", DatumType::Execution);
}

void GateNode::Evaluate()
{
    bool open = GetInputValue(1).GetBool();
    if (open)
    {
        TriggerExecutionPin(0);
    }
}

glm::vec4 GateNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// DoNNode
// =============================================================================

DEFINE_GRAPH_NODE(DoNNode);

void DoNNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("N", DatumType::Integer, Datum(1));
    AddInputPin("Reset", DatumType::Bool, Datum(false));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Count", DatumType::Integer);
}

void DoNNode::Evaluate()
{
    int32_t n = GetInputValue(1).GetInteger();
    bool reset = GetInputValue(2).GetBool();
    if (reset)
        mCallCount = 0;
    if (mCallCount < n)
    {
        mCallCount++;
        SetOutputValue(1, Datum(mCallCount));
        TriggerExecutionPin(0);
    }
}

glm::vec4 DoNNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// MultiGateNode
// =============================================================================

DEFINE_GRAPH_NODE(MultiGateNode);

void MultiGateNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Loop", DatumType::Bool, Datum(false));
    AddInputPin("Reset", DatumType::Bool, Datum(false));
    AddOutputPin("Out 0", DatumType::Execution);
    AddOutputPin("Out 1", DatumType::Execution);
    AddOutputPin("Out 2", DatumType::Execution);
    AddOutputPin("Out 3", DatumType::Execution);
    AddOutputPin("Index", DatumType::Integer);
}

void MultiGateNode::Evaluate()
{
    bool loop = GetInputValue(1).GetBool();
    bool reset = GetInputValue(2).GetBool();
    if (reset)
        mCurrentIndex = 0;
    if (mCurrentIndex < 4)
    {
        SetOutputValue(4, Datum(mCurrentIndex));
        TriggerExecutionPin(mCurrentIndex);
        mCurrentIndex++;
        if (loop && mCurrentIndex >= 4)
            mCurrentIndex = 0;
    }
}

glm::vec4 MultiGateNode::GetNodeColor() const { return kFlowNodeColor; }

// =============================================================================
// Input Enum Helpers
// =============================================================================

static const PinEnumOption sSourceOptions[] = {
    { "Keyboard", 0 },
    { "Gamepad",  1 },
    { "Mouse",    2 },
};

static void FillKeyboardOptions(std::vector<PinEnumOption>& out)
{
    out.push_back({ "Space",     OCTAVE_KEY_SPACE });
    out.push_back({ "Enter",     OCTAVE_KEY_ENTER });
    out.push_back({ "Escape",    OCTAVE_KEY_ESCAPE });
    out.push_back({ "Tab",       OCTAVE_KEY_TAB });
    out.push_back({ "Backspace", OCTAVE_KEY_BACKSPACE });
    out.push_back({ "Shift",     OCTAVE_KEY_SHIFT_L });
    out.push_back({ "Ctrl",      OCTAVE_KEY_CONTROL_L });
    out.push_back({ "Alt",       OCTAVE_KEY_ALT_L });
#if !PLATFORM_3DS
    out.push_back({ "Up",    OCTAVE_KEY_UP });
    out.push_back({ "Down",  OCTAVE_KEY_DOWN });
    out.push_back({ "Left",  OCTAVE_KEY_LEFT });
    out.push_back({ "Right", OCTAVE_KEY_RIGHT });
    out.push_back({ "A", OCTAVE_KEY_A }); out.push_back({ "B", OCTAVE_KEY_B });
#endif
    out.push_back({ "C", OCTAVE_KEY_C }); out.push_back({ "D", OCTAVE_KEY_D });
    out.push_back({ "E", OCTAVE_KEY_E }); out.push_back({ "F", OCTAVE_KEY_F });
    out.push_back({ "G", OCTAVE_KEY_G }); out.push_back({ "H", OCTAVE_KEY_H });
    out.push_back({ "I", OCTAVE_KEY_I }); out.push_back({ "J", OCTAVE_KEY_J });
    out.push_back({ "K", OCTAVE_KEY_K });
#if !PLATFORM_3DS
    out.push_back({ "L", OCTAVE_KEY_L });
#endif
    out.push_back({ "M", OCTAVE_KEY_M }); out.push_back({ "N", OCTAVE_KEY_N });
    out.push_back({ "O", OCTAVE_KEY_O }); out.push_back({ "P", OCTAVE_KEY_P });
    out.push_back({ "Q", OCTAVE_KEY_Q });
#if !PLATFORM_3DS
    out.push_back({ "R", OCTAVE_KEY_R });
#endif
    out.push_back({ "S", OCTAVE_KEY_S }); out.push_back({ "T", OCTAVE_KEY_T });
    out.push_back({ "U", OCTAVE_KEY_U }); out.push_back({ "V", OCTAVE_KEY_V });
    out.push_back({ "W", OCTAVE_KEY_W });
#if !PLATFORM_3DS
    out.push_back({ "X", OCTAVE_KEY_X }); out.push_back({ "Y", OCTAVE_KEY_Y });
#endif
    out.push_back({ "Z", OCTAVE_KEY_Z });
    out.push_back({ "0", OCTAVE_KEY_0 }); out.push_back({ "1", OCTAVE_KEY_1 });
    out.push_back({ "2", OCTAVE_KEY_2 }); out.push_back({ "3", OCTAVE_KEY_3 });
    out.push_back({ "4", OCTAVE_KEY_4 }); out.push_back({ "5", OCTAVE_KEY_5 });
    out.push_back({ "6", OCTAVE_KEY_6 }); out.push_back({ "7", OCTAVE_KEY_7 });
    out.push_back({ "8", OCTAVE_KEY_8 }); out.push_back({ "9", OCTAVE_KEY_9 });
    out.push_back({ "F1",  OCTAVE_KEY_F1 });  out.push_back({ "F2",  OCTAVE_KEY_F2 });
    out.push_back({ "F3",  OCTAVE_KEY_F3 });  out.push_back({ "F4",  OCTAVE_KEY_F4 });
    out.push_back({ "F5",  OCTAVE_KEY_F5 });  out.push_back({ "F6",  OCTAVE_KEY_F6 });
    out.push_back({ "F7",  OCTAVE_KEY_F7 });  out.push_back({ "F8",  OCTAVE_KEY_F8 });
    out.push_back({ "F9",  OCTAVE_KEY_F9 });  out.push_back({ "F10", OCTAVE_KEY_F10 });
    out.push_back({ "F11", OCTAVE_KEY_F11 }); out.push_back({ "F12", OCTAVE_KEY_F12 });
    out.push_back({ "Insert",    OCTAVE_KEY_INSERT });
    out.push_back({ "Delete",    OCTAVE_KEY_DELETE });
    out.push_back({ "Home",      OCTAVE_KEY_HOME });
    out.push_back({ "End",       OCTAVE_KEY_END });
    out.push_back({ "Page Up",   OCTAVE_KEY_PAGE_UP });
    out.push_back({ "Page Down", OCTAVE_KEY_PAGE_DOWN });
}

static const PinEnumOption sGamepadOptions[] = {
    { "A",       GAMEPAD_A },
    { "B",       GAMEPAD_B },
    { "X",       GAMEPAD_X },
    { "Y",       GAMEPAD_Y },
    { "L1",      GAMEPAD_L1 },
    { "R1",      GAMEPAD_R1 },
    { "L2",      GAMEPAD_L2 },
    { "R2",      GAMEPAD_R2 },
    { "Start",   GAMEPAD_START },
    { "Select",  GAMEPAD_SELECT },
    { "Home",    GAMEPAD_HOME },
    { "L Stick", GAMEPAD_THUMBL },
    { "R Stick", GAMEPAD_THUMBR },
    { "D-Up",    GAMEPAD_UP },
    { "D-Down",  GAMEPAD_DOWN },
    { "D-Left",  GAMEPAD_LEFT },
    { "D-Right", GAMEPAD_RIGHT },
};

static const PinEnumOption sMouseOptions[] = {
    { "Left",   MOUSE_LEFT },
    { "Right",  MOUSE_RIGHT },
    { "Middle", MOUSE_MIDDLE },
    { "X1",     MOUSE_X1 },
    { "X2",     MOUSE_X2 },
};

static void FillButtonOptions(int32_t source, std::vector<PinEnumOption>& outOptions)
{
    if (source == 0) // Keyboard
    {
        FillKeyboardOptions(outOptions);
    }
    else if (source == 1) // Gamepad
    {
        for (const auto& opt : sGamepadOptions)
            outOptions.push_back(opt);
    }
    else if (source == 2) // Mouse
    {
        for (const auto& opt : sMouseOptions)
            outOptions.push_back(opt);
    }
}

// =============================================================================
// InputEventNode
// =============================================================================

DEFINE_GRAPH_NODE(InputEventNode);

void InputEventNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Source", DatumType::Integer, Datum(0));
    AddInputPin("Button", DatumType::Integer, Datum(0));
    AddInputPin("Controller", DatumType::Integer, Datum(0));
    AddOutputPin("Pressed", DatumType::Execution);
    AddOutputPin("Held", DatumType::Execution);
    AddOutputPin("Released", DatumType::Execution);
}

void InputEventNode::Evaluate()
{
    int32_t source = GetInputValue(1).GetInteger();
    int32_t button = GetInputValue(2).GetInteger();
    int32_t controller = GetInputValue(3).GetInteger();

    bool pressed = false;
    bool held = false;
    bool released = false;

    if (source == 0) // Keyboard
    {
        pressed = IsKeyJustDown(button);
        held = IsKeyDown(button);
        released = IsKeyJustUp(button);
    }
    else if (source == 1) // Gamepad
    {
        pressed = IsGamepadButtonJustDown(button, controller);
        held = IsGamepadButtonDown(button, controller);
        released = IsGamepadButtonJustUp(button, controller);
    }
    else if (source == 2) // Mouse
    {
        pressed = IsMouseButtonJustDown(button);
        held = IsMouseButtonDown(button);
        released = IsMouseButtonJustUp(button);
    }

    if (pressed)  TriggerExecutionPin(0);
    if (held)     TriggerExecutionPin(1);
    if (released) TriggerExecutionPin(2);
}

glm::vec4 InputEventNode::GetNodeColor() const { return kInputNodeColor; }

bool InputEventNode::GetPinEnumOptions(uint32_t pinIndex, std::vector<PinEnumOption>& outOptions) const
{
    if (pinIndex == 1) // Source
    {
        for (const auto& opt : sSourceOptions)
            outOptions.push_back(opt);
        return true;
    }
    if (pinIndex == 2) // Button
    {
        int32_t source = mInputPins[1].mValue.GetInteger();
        FillButtonOptions(source, outOptions);
        return true;
    }
    return false;
}

// =============================================================================
// InputDownNode
// =============================================================================

DEFINE_GRAPH_NODE(InputDownNode);

void InputDownNode::SetupPins()
{
    AddInputPin("Source", DatumType::Integer, Datum(0));
    AddInputPin("Button", DatumType::Integer, Datum(0));
    AddInputPin("Controller", DatumType::Integer, Datum(0));
    AddOutputPin("Down", DatumType::Bool);
}

void InputDownNode::Evaluate()
{
    int32_t source = GetInputValue(0).GetInteger();
    int32_t button = GetInputValue(1).GetInteger();
    int32_t controller = GetInputValue(2).GetInteger();

    bool down = false;
    if (source == 0)
        down = IsKeyDown(button);
    else if (source == 1)
        down = IsGamepadButtonDown(button, controller);
    else if (source == 2)
        down = IsMouseButtonDown(button);

    SetOutputValue(0, Datum(down));
}

glm::vec4 InputDownNode::GetNodeColor() const { return kInputNodeColor; }

bool InputDownNode::GetPinEnumOptions(uint32_t pinIndex, std::vector<PinEnumOption>& outOptions) const
{
    if (pinIndex == 0) // Source
    {
        for (const auto& opt : sSourceOptions)
            outOptions.push_back(opt);
        return true;
    }
    if (pinIndex == 1) // Button
    {
        int32_t source = mInputPins[0].mValue.GetInteger();
        FillButtonOptions(source, outOptions);
        return true;
    }
    return false;
}

// =============================================================================
// InputCountNode
// =============================================================================

DEFINE_GRAPH_NODE(InputCountNode);

void InputCountNode::SetupPins()
{
    AddOutputPin("Count", DatumType::Integer);
}

void InputCountNode::Evaluate()
{
    int32_t count = 0;
    for (int32_t i = 0; i < 4; ++i)
    {
        if (IsGamepadConnected(i))
            count++;
    }
    SetOutputValue(0, Datum(count));
}

glm::vec4 InputCountNode::GetNodeColor() const { return kInputNodeColor; }

// =============================================================================
// GamepadAxisNode
// =============================================================================

static const PinEnumOption sGamepadAxisOptions[] = {
    { "L Stick X",  GAMEPAD_AXIS_LTHUMB_X },
    { "L Stick Y",  GAMEPAD_AXIS_LTHUMB_Y },
    { "R Stick X",  GAMEPAD_AXIS_RTHUMB_X },
    { "R Stick Y",  GAMEPAD_AXIS_RTHUMB_Y },
    { "L Trigger",  GAMEPAD_AXIS_LTRIGGER },
    { "R Trigger",  GAMEPAD_AXIS_RTRIGGER },
};

DEFINE_GRAPH_NODE(GamepadAxisNode);

void GamepadAxisNode::SetupPins()
{
    AddInputPin("Axis", DatumType::Integer, Datum((int32_t)GAMEPAD_AXIS_LTHUMB_X));
    AddInputPin("Controller", DatumType::Integer, Datum(0));
    AddOutputPin("Value", DatumType::Float);
}

void GamepadAxisNode::Evaluate()
{
    int32_t axis = GetInputValue(0).GetInteger();
    int32_t controller = GetInputValue(1).GetInteger();

    float value = GetGamepadAxisValue(axis, controller);
    SetOutputValue(0, Datum(value));
}

glm::vec4 GamepadAxisNode::GetNodeColor() const { return kInputNodeColor; }

bool GamepadAxisNode::GetPinEnumOptions(uint32_t pinIndex, std::vector<PinEnumOption>& outOptions) const
{
    if (pinIndex == 0) // Axis
    {
        for (const auto& opt : sGamepadAxisOptions)
            outOptions.push_back(opt);
        return true;
    }
    return false;
}

// =============================================================================
// MousePositionNode
// =============================================================================

DEFINE_GRAPH_NODE(MousePositionNode);

void MousePositionNode::SetupPins()
{
    AddOutputPin("X", DatumType::Integer);
    AddOutputPin("Y", DatumType::Integer);
}

void MousePositionNode::Evaluate()
{
    int32_t x = 0, y = 0;
    GetMousePosition(x, y);
    SetOutputValue(0, Datum(x));
    SetOutputValue(1, Datum(y));
}

glm::vec4 MousePositionNode::GetNodeColor() const { return kInputNodeColor; }

// =============================================================================
// ScrollWheelNode
// =============================================================================

DEFINE_GRAPH_NODE(ScrollWheelNode);

void ScrollWheelNode::SetupPins()
{
    AddOutputPin("Delta", DatumType::Integer);
}

void ScrollWheelNode::Evaluate()
{
    SetOutputValue(0, Datum(GetScrollWheelDelta()));
}

glm::vec4 ScrollWheelNode::GetNodeColor() const { return kInputNodeColor; }
