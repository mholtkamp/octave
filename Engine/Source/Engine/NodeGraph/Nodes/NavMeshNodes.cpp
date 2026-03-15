#include "NodeGraph/Nodes/NavMeshNodes.h"
#include "NodeGraph/PointCloud.h"
#include "NodeGraph/NodeGraph.h"
#include "World.h"

FORCE_LINK_DEF(NavMeshNodes);

static const glm::vec4 kNavMeshNodeColor = glm::vec4(0.2f, 0.7f, 0.5f, 1.0f);

static Node* GetOwnerNode(GraphNode* graphNode)
{
    NodeGraph* graph = graphNode->GetGraph();
    if (graph != nullptr)
    {
        return graph->GetOwnerNode();
    }
    return nullptr;
}

static World* GetOwnerWorld(GraphNode* graphNode)
{
    Node* owner = GetOwnerNode(graphNode);
    return owner ? owner->GetWorld() : nullptr;
}

static Datum MakePointCloudDatum(PointCloud* cloud)
{
    Datum d;
    d.SetType(DatumType::PointCloud);
    d.SetCount(1);
    d.mData.pc[0] = cloud;
    return d;
}

// =============================================================================
// FindNavPathNode
// =============================================================================
DEFINE_GRAPH_NODE(FindNavPathNode);

void FindNavPathNode::SetupPins()
{
    AddInputPin("Start", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Goal", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Path", DatumType::PointCloud);
    AddOutputPin("Found", DatumType::Bool);
    AddOutputPin("Waypoint Count", DatumType::Integer);
}

void FindNavPathNode::Evaluate()
{
    glm::vec3 start = GetInputValue(0).GetVector();
    glm::vec3 goal = GetInputValue(1).GetVector();

    World* world = GetOwnerWorld(this);
    if (world == nullptr)
    {
        SetOutputValue(0, MakePointCloudDatum(new PointCloud()));
        SetOutputValue(1, Datum(false));
        SetOutputValue(2, Datum((int32_t)0));
        return;
    }

    std::vector<glm::vec3> path;
    bool found = world->FindNavPath(start, goal, path);

    PointCloud* cloud = new PointCloud();
    if (found)
    {
        cloud->Resize((uint32_t)path.size());
        for (uint32_t i = 0; i < (uint32_t)path.size(); ++i)
        {
            cloud->SetPosition(i, path[i]);
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
    SetOutputValue(1, Datum(found));
    SetOutputValue(2, Datum((int32_t)path.size()));
}

glm::vec4 FindNavPathNode::GetNodeColor() const { return kNavMeshNodeColor; }

// =============================================================================
// FindRandomNavPointNode
// =============================================================================
DEFINE_GRAPH_NODE(FindRandomNavPointNode);

void FindRandomNavPointNode::SetupPins()
{
    AddOutputPin("Point", DatumType::Vector);
    AddOutputPin("Found", DatumType::Bool);
}

void FindRandomNavPointNode::Evaluate()
{
    World* world = GetOwnerWorld(this);
    if (world == nullptr)
    {
        SetOutputValue(0, Datum(glm::vec3(0.0f)));
        SetOutputValue(1, Datum(false));
        return;
    }

    glm::vec3 point;
    bool found = world->FindRandomNavPoint(point);

    SetOutputValue(0, Datum(found ? point : glm::vec3(0.0f)));
    SetOutputValue(1, Datum(found));
}

glm::vec4 FindRandomNavPointNode::GetNodeColor() const { return kNavMeshNodeColor; }

// =============================================================================
// FindClosestNavPointNode
// =============================================================================
DEFINE_GRAPH_NODE(FindClosestNavPointNode);

void FindClosestNavPointNode::SetupPins()
{
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Point", DatumType::Vector);
    AddOutputPin("Found", DatumType::Bool);
}

void FindClosestNavPointNode::Evaluate()
{
    glm::vec3 position = GetInputValue(0).GetVector();

    World* world = GetOwnerWorld(this);
    if (world == nullptr)
    {
        SetOutputValue(0, Datum(glm::vec3(0.0f)));
        SetOutputValue(1, Datum(false));
        return;
    }

    glm::vec3 outPoint;
    bool found = world->FindClosestNavPoint(position, outPoint);

    SetOutputValue(0, Datum(found ? outPoint : glm::vec3(0.0f)));
    SetOutputValue(1, Datum(found));
}

glm::vec4 FindClosestNavPointNode::GetNodeColor() const { return kNavMeshNodeColor; }

// =============================================================================
// IsAutoNavRebuildNode
// =============================================================================
DEFINE_GRAPH_NODE(IsAutoNavRebuildNode);

void IsAutoNavRebuildNode::SetupPins()
{
    AddOutputPin("Enabled", DatumType::Bool);
}

void IsAutoNavRebuildNode::Evaluate()
{
    World* world = GetOwnerWorld(this);
    bool enabled = (world != nullptr) ? world->IsAutoNavRebuildEnabled() : false;
    SetOutputValue(0, Datum(enabled));
}

glm::vec4 IsAutoNavRebuildNode::GetNodeColor() const { return kNavMeshNodeColor; }

// =============================================================================
// BuildNavDataNode
// =============================================================================
DEFINE_GRAPH_NODE(BuildNavDataNode);

void BuildNavDataNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddOutputPin("Exec", DatumType::Execution);
}

void BuildNavDataNode::Evaluate()
{
    World* world = GetOwnerWorld(this);
    if (world != nullptr)
    {
        world->BuildNavigationData();
    }
    TriggerExecutionPin(0);
}

glm::vec4 BuildNavDataNode::GetNodeColor() const { return kNavMeshNodeColor; }

// =============================================================================
// EnableAutoNavRebuildNode
// =============================================================================
DEFINE_GRAPH_NODE(EnableAutoNavRebuildNode);

void EnableAutoNavRebuildNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Enable", DatumType::Bool, Datum(true));
    AddOutputPin("Exec", DatumType::Execution);
}

void EnableAutoNavRebuildNode::Evaluate()
{
    bool enable = GetInputValue(1).GetBool();

    World* world = GetOwnerWorld(this);
    if (world != nullptr)
    {
        world->EnableAutoNavRebuild(enable);
    }
    TriggerExecutionPin(0);
}

glm::vec4 EnableAutoNavRebuildNode::GetNodeColor() const { return kNavMeshNodeColor; }

// =============================================================================
// InvalidateNavMeshNode
// =============================================================================
DEFINE_GRAPH_NODE(InvalidateNavMeshNode);

void InvalidateNavMeshNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddOutputPin("Exec", DatumType::Execution);
}

void InvalidateNavMeshNode::Evaluate()
{
    World* world = GetOwnerWorld(this);
    if (world != nullptr)
    {
        world->InvalidateNavMesh();
    }
    TriggerExecutionPin(0);
}

glm::vec4 InvalidateNavMeshNode::GetNodeColor() const { return kNavMeshNodeColor; }
