#include "NodeGraph/Nodes/PointNodes.h"
#include "NodeGraph/PointCloud.h"
#include "NodeGraph/NodeGraph.h"
#include "Utilities.h"
#include "Log.h"
#include "Nodes/3D/InstancedMesh3d.h"
#include "Nodes/3D/Spline3d.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Assets/StaticMesh.h"
#include "Vertex.h"
#include "World.h"

#include <algorithm>
#include <numeric>
#include <random>

FORCE_LINK_DEF(PointNodes);

static const glm::vec4 kPointNodeColor = glm::vec4(0.9f, 0.55f, 0.1f, 1.0f);

// Helper: get the owner Node from a graph node (for World access)
static Node* GetOwnerNode(GraphNode* graphNode)
{
    NodeGraph* graph = graphNode->GetGraph();
    if (graph != nullptr)
    {
        return graph->GetOwnerNode();
    }
    return nullptr;
}

// Helper: get PointCloud from input pin (returns nullptr if invalid)
static PointCloud* GetInputPointCloud(const GraphNode* node, uint32_t pinIndex)
{
    const Datum& d = node->GetInputValue(pinIndex);
    if (d.GetType() == DatumType::PointCloud && d.GetCount() > 0)
        return d.mData.pc[0];
    return nullptr;
}

// Helper: wrap a PointCloud* in a Datum (Datum takes ownership via deep copy)
static Datum MakePointCloudDatum(PointCloud* cloud)
{
    Datum d;
    d.SetType(DatumType::PointCloud);
    d.SetCount(1);
    // Delete the default nullptr and assign our cloud
    d.mData.pc[0] = cloud;
    return d;
}

// Helper: seeded random float [0, 1]
static float SeededRandom(uint32_t seed, uint32_t index)
{
    uint32_t h = seed ^ (index * 2654435761u);
    h ^= h >> 16;
    h *= 0x45d9f3b;
    h ^= h >> 16;
    h *= 0x45d9f3b;
    h ^= h >> 16;
    return float(h & 0x00FFFFFF) / float(0x00FFFFFF);
}

// =============================================================================
// CreatePointsGridNode
// =============================================================================
DEFINE_GRAPH_NODE(CreatePointsGridNode);

void CreatePointsGridNode::SetupPins()
{
    AddInputPin("Count X", DatumType::Integer, Datum(10));
    AddInputPin("Count Y", DatumType::Integer, Datum(1));
    AddInputPin("Count Z", DatumType::Integer, Datum(10));
    AddInputPin("Spacing", DatumType::Float, Datum(1.0f));
    AddOutputPin("Points", DatumType::PointCloud);
}

void CreatePointsGridNode::Evaluate()
{
    int32_t cx = glm::max(GetInputValue(0).GetInteger(), (int32_t)1);
    int32_t cy = glm::max(GetInputValue(1).GetInteger(), (int32_t)1);
    int32_t cz = glm::max(GetInputValue(2).GetInteger(), (int32_t)1);
    float spacing = GetInputValue(3).GetFloat();

    PointCloud* cloud = new PointCloud();
    cloud->Resize(cx * cy * cz);

    uint32_t idx = 0;
    for (int32_t y = 0; y < cy; ++y)
    {
        for (int32_t z = 0; z < cz; ++z)
        {
            for (int32_t x = 0; x < cx; ++x)
            {
                cloud->SetPosition(idx, glm::vec3(x * spacing, y * spacing, z * spacing));
                idx++;
            }
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 CreatePointsGridNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// ScatterPointsOnMeshNode
// =============================================================================
DEFINE_GRAPH_NODE(ScatterPointsOnMeshNode);

void ScatterPointsOnMeshNode::SetupPins()
{
    AddInputPin("Mesh", DatumType::Asset, Datum((Asset*)nullptr));
    AddInputPin("Count", DatumType::Integer, Datum(100));
    AddInputPin("Seed", DatumType::Integer, Datum(0));
    AddOutputPin("Points", DatumType::PointCloud);
}

void ScatterPointsOnMeshNode::Evaluate()
{
    Asset* meshAsset = GetInputValue(0).GetAsset();
    int32_t count = glm::max(GetInputValue(1).GetInteger(), (int32_t)0);
    int32_t seed = GetInputValue(2).GetInteger();

    PointCloud* cloud = new PointCloud();

    StaticMesh* mesh = meshAsset ? meshAsset->As<StaticMesh>() : nullptr;
    if (mesh != nullptr && count > 0)
    {
        cloud->AddAttribute("N", DatumType::Vector);

        // Get mesh vertex data (raw pointers)
        Vertex* verts = mesh->GetVertices();
        IndexType* indices = mesh->GetIndices();
        uint32_t numIndices = mesh->GetNumIndices();

        if (verts && indices && numIndices >= 3)
        {
            // Calculate triangle areas for weighted sampling
            uint32_t numTris = numIndices / 3;
            std::vector<float> cumulativeArea(numTris);
            float totalArea = 0.0f;

            for (uint32_t t = 0; t < numTris; ++t)
            {
                glm::vec3 v0 = verts[indices[t * 3 + 0]].mPosition;
                glm::vec3 v1 = verts[indices[t * 3 + 1]].mPosition;
                glm::vec3 v2 = verts[indices[t * 3 + 2]].mPosition;
                float area = 0.5f * glm::length(glm::cross(v1 - v0, v2 - v0));
                totalArea += area;
                cumulativeArea[t] = totalArea;
            }

            for (int32_t i = 0; i < count; ++i)
            {
                // Pick random triangle weighted by area
                float r = SeededRandom((uint32_t)seed, (uint32_t)i) * totalArea;
                uint32_t triIdx = 0;
                for (uint32_t t = 0; t < numTris; ++t)
                {
                    if (cumulativeArea[t] >= r)
                    {
                        triIdx = t;
                        break;
                    }
                }

                // Random barycentric coords
                float u = SeededRandom((uint32_t)seed, (uint32_t)(i * 2 + 1));
                float v = SeededRandom((uint32_t)seed, (uint32_t)(i * 2 + 2));
                if (u + v > 1.0f) { u = 1.0f - u; v = 1.0f - v; }
                float w = 1.0f - u - v;

                uint32_t i0 = (uint32_t)indices[triIdx * 3 + 0];
                uint32_t i1 = (uint32_t)indices[triIdx * 3 + 1];
                uint32_t i2 = (uint32_t)indices[triIdx * 3 + 2];

                glm::vec3 pos = verts[i0].mPosition * w + verts[i1].mPosition * u + verts[i2].mPosition * v;
                glm::vec3 nrm = glm::normalize(verts[i0].mNormal * w + verts[i1].mNormal * u + verts[i2].mNormal * v);

                cloud->AddPoint(pos);
                cloud->SetAttributeVector(cloud->GetNumPoints() - 1, "N", nrm);
            }
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 ScatterPointsOnMeshNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// PointsFromSplineNode
// =============================================================================
DEFINE_GRAPH_NODE(PointsFromSplineNode);

void PointsFromSplineNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddInputPin("Count", DatumType::Integer, Datum(20));
    AddInputPin("Close Loop", DatumType::Bool, Datum(false));
    AddOutputPin("Points", DatumType::PointCloud);
}

void PointsFromSplineNode::Evaluate()
{
    Node* splineNode = GetInputValue(0).GetNode().Get();
    int32_t count = glm::max(GetInputValue(1).GetInteger(), (int32_t)2);
    bool closeLoop = GetInputValue(2).GetBool();

    PointCloud* cloud = new PointCloud();

    Spline3D* spline = splineNode ? splineNode->As<Spline3D>() : nullptr;
    if (spline != nullptr && spline->GetPointCount() >= 2)
    {
        cloud->AddAttribute("N", DatumType::Vector);

        for (int32_t i = 0; i < count; ++i)
        {
            float t = closeLoop ? (float)i / (float)count : (float)i / (float)(count - 1);
            glm::vec3 pos = spline->GetPositionAt(t);
            glm::vec3 tangent = glm::normalize(spline->GetTangentAt(t));

            cloud->AddPoint(pos);
            cloud->SetAttributeVector(cloud->GetNumPoints() - 1, "N", tangent);
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 PointsFromSplineNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// CreatePointsLineNode
// =============================================================================
DEFINE_GRAPH_NODE(CreatePointsLineNode);

void CreatePointsLineNode::SetupPins()
{
    AddInputPin("Start", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("End", DatumType::Vector, Datum(glm::vec3(10.0f, 0.0f, 0.0f)));
    AddInputPin("Count", DatumType::Integer, Datum(10));
    AddOutputPin("Points", DatumType::PointCloud);
}

void CreatePointsLineNode::Evaluate()
{
    glm::vec3 start = GetInputValue(0).GetVector();
    glm::vec3 end = GetInputValue(1).GetVector();
    int32_t count = glm::max(GetInputValue(2).GetInteger(), (int32_t)2);

    PointCloud* cloud = new PointCloud();
    for (int32_t i = 0; i < count; ++i)
    {
        float t = (float)i / (float)(count - 1);
        cloud->AddPoint(glm::mix(start, end, t));
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 CreatePointsLineNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// RandomPointsInBoxNode
// =============================================================================
DEFINE_GRAPH_NODE(RandomPointsInBoxNode);

void RandomPointsInBoxNode::SetupPins()
{
    AddInputPin("Min", DatumType::Vector, Datum(glm::vec3(-5.0f)));
    AddInputPin("Max", DatumType::Vector, Datum(glm::vec3(5.0f)));
    AddInputPin("Count", DatumType::Integer, Datum(100));
    AddInputPin("Seed", DatumType::Integer, Datum(0));
    AddOutputPin("Points", DatumType::PointCloud);
}

void RandomPointsInBoxNode::Evaluate()
{
    glm::vec3 bmin = GetInputValue(0).GetVector();
    glm::vec3 bmax = GetInputValue(1).GetVector();
    int32_t count = glm::max(GetInputValue(2).GetInteger(), (int32_t)0);
    int32_t seed = GetInputValue(3).GetInteger();

    PointCloud* cloud = new PointCloud();
    for (int32_t i = 0; i < count; ++i)
    {
        float rx = SeededRandom((uint32_t)seed, (uint32_t)(i * 3 + 0));
        float ry = SeededRandom((uint32_t)seed, (uint32_t)(i * 3 + 1));
        float rz = SeededRandom((uint32_t)seed, (uint32_t)(i * 3 + 2));
        glm::vec3 pos = glm::mix(bmin, bmax, glm::vec3(rx, ry, rz));
        cloud->AddPoint(pos);
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 RandomPointsInBoxNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// MergePointsNode
// =============================================================================
DEFINE_GRAPH_NODE(MergePointsNode);

void MergePointsNode::SetupPins()
{
    AddInputPin("Points A", DatumType::PointCloud);
    AddInputPin("Points B", DatumType::PointCloud);
    AddOutputPin("Points", DatumType::PointCloud);
}

void MergePointsNode::Evaluate()
{
    PointCloud* a = GetInputPointCloud(this, 0);
    PointCloud* b = GetInputPointCloud(this, 1);

    PointCloud* result = a ? a->Clone() : new PointCloud();
    if (b)
        result->Merge(*b);

    SetOutputValue(0, MakePointCloudDatum(result));
}

glm::vec4 MergePointsNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// SetPointAttributeFloatNode
// =============================================================================
DEFINE_GRAPH_NODE(SetPointAttributeFloatNode);

void SetPointAttributeFloatNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Name", DatumType::String, Datum(std::string("pscale")));
    AddInputPin("Value", DatumType::Float, Datum(1.0f));
    AddOutputPin("Points", DatumType::PointCloud);
}

void SetPointAttributeFloatNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    const std::string& name = GetInputValue(1).GetString();
    float value = GetInputValue(2).GetFloat();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (!cloud->HasAttribute(name))
        cloud->AddAttribute(name, DatumType::Float);

    for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
        cloud->SetAttributeFloat(i, name, value);

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 SetPointAttributeFloatNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// SetPointAttributeVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(SetPointAttributeVectorNode);

void SetPointAttributeVectorNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Name", DatumType::String, Datum(std::string("N")));
    AddInputPin("Value", DatumType::Vector, Datum(glm::vec3(0.0f, 1.0f, 0.0f)));
    AddOutputPin("Points", DatumType::PointCloud);
}

void SetPointAttributeVectorNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    const std::string& name = GetInputValue(1).GetString();
    glm::vec3 value = GetInputValue(2).GetVector();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (!cloud->HasAttribute(name))
        cloud->AddAttribute(name, DatumType::Vector);

    for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
        cloud->SetAttributeVector(i, name, value);

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 SetPointAttributeVectorNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// SetPointAttributeColorNode
// =============================================================================
DEFINE_GRAPH_NODE(SetPointAttributeColorNode);

void SetPointAttributeColorNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Name", DatumType::String, Datum(std::string("Cd")));
    AddInputPin("Value", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Points", DatumType::PointCloud);
}

void SetPointAttributeColorNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    const std::string& name = GetInputValue(1).GetString();
    glm::vec4 value = GetInputValue(2).GetColor();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (!cloud->HasAttribute(name))
        cloud->AddAttribute(name, DatumType::Color);

    for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
        cloud->SetAttributeColor(i, name, value);

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 SetPointAttributeColorNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// GetPointAttributeFloatNode
// =============================================================================
DEFINE_GRAPH_NODE(GetPointAttributeFloatNode);

void GetPointAttributeFloatNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddInputPin("Name", DatumType::String, Datum(std::string("pscale")));
    AddOutputPin("Value", DatumType::Float);
}

void GetPointAttributeFloatNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();
    const std::string& name = GetInputValue(2).GetString();

    float value = 0.0f;
    if (cloud && index >= 0 && (uint32_t)index < cloud->GetNumPoints())
        value = cloud->GetAttributeFloat((uint32_t)index, name);

    SetOutputValue(0, Datum(value));
}

glm::vec4 GetPointAttributeFloatNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// GetPointAttributeVectorNode
// =============================================================================
DEFINE_GRAPH_NODE(GetPointAttributeVectorNode);

void GetPointAttributeVectorNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddInputPin("Name", DatumType::String, Datum(std::string("N")));
    AddOutputPin("Value", DatumType::Vector);
}

void GetPointAttributeVectorNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();
    const std::string& name = GetInputValue(2).GetString();

    glm::vec3 value(0.0f);
    if (cloud && index >= 0 && (uint32_t)index < cloud->GetNumPoints())
        value = cloud->GetAttributeVector((uint32_t)index, name);

    SetOutputValue(0, Datum(value));
}

glm::vec4 GetPointAttributeVectorNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// GetPointAttributeColorNode
// =============================================================================
DEFINE_GRAPH_NODE(GetPointAttributeColorNode);

void GetPointAttributeColorNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddInputPin("Name", DatumType::String, Datum(std::string("Cd")));
    AddOutputPin("Value", DatumType::Color);
}

void GetPointAttributeColorNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();
    const std::string& name = GetInputValue(2).GetString();

    glm::vec4 value(1.0f);
    if (cloud && index >= 0 && (uint32_t)index < cloud->GetNumPoints())
        value = cloud->GetAttributeColor((uint32_t)index, name);

    SetOutputValue(0, Datum(value));
}

glm::vec4 GetPointAttributeColorNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// GetPointCountNode
// =============================================================================
DEFINE_GRAPH_NODE(GetPointCountNode);

void GetPointCountNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddOutputPin("Count", DatumType::Integer);
}

void GetPointCountNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    SetOutputValue(0, Datum(cloud ? (int32_t)cloud->GetNumPoints() : 0));
}

glm::vec4 GetPointCountNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// GetPointPositionNode
// =============================================================================
DEFINE_GRAPH_NODE(GetPointPositionNode);

void GetPointPositionNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddOutputPin("Position", DatumType::Vector);
}

void GetPointPositionNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();

    glm::vec3 pos(0.0f);
    if (cloud && index >= 0 && (uint32_t)index < cloud->GetNumPoints())
        pos = cloud->GetPosition((uint32_t)index);

    SetOutputValue(0, Datum(pos));
}

glm::vec4 GetPointPositionNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// SetPointPositionNode
// =============================================================================
DEFINE_GRAPH_NODE(SetPointPositionNode);

void SetPointPositionNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddOutputPin("Points", DatumType::PointCloud);
}

void SetPointPositionNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();
    glm::vec3 pos = GetInputValue(2).GetVector();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (index >= 0 && (uint32_t)index < cloud->GetNumPoints())
        cloud->SetPosition((uint32_t)index, pos);

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 SetPointPositionNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// ForEachPointNode
// =============================================================================
DEFINE_GRAPH_NODE(ForEachPointNode);

void ForEachPointNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Points", DatumType::PointCloud);
    AddOutputPin("Loop Body", DatumType::Execution);
    AddOutputPin("Completed", DatumType::Execution);
    AddOutputPin("Index", DatumType::Integer);
    AddOutputPin("Position", DatumType::Vector);
    AddOutputPin("Normal", DatumType::Vector);
    AddOutputPin("Color", DatumType::Color);
    AddOutputPin("PScale", DatumType::Float);
}

void ForEachPointNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 1);

    if (cloud)
    {
        for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
        {
            SetOutputValue(2, Datum((int32_t)i));
            SetOutputValue(3, Datum(cloud->GetPosition(i)));
            SetOutputValue(4, Datum(cloud->GetNormal(i)));
            SetOutputValue(5, Datum(cloud->GetCd(i)));
            SetOutputValue(6, Datum(cloud->GetPScale(i)));
            TriggerExecutionPin(0); // Loop Body
        }
    }

    TriggerExecutionPin(1); // Completed
}

glm::vec4 ForEachPointNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// SetPointInLoopNode
// =============================================================================
DEFINE_GRAPH_NODE(SetPointInLoopNode);

void SetPointInLoopNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("PScale", DatumType::Float, Datum(1.0f));
    AddInputPin("Color", DatumType::Color, Datum(glm::vec4(1.0f)));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Points", DatumType::PointCloud);
}

void SetPointInLoopNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 1);
    int32_t index = GetInputValue(2).GetInteger();
    glm::vec3 pos = GetInputValue(3).GetVector();
    float pscale = GetInputValue(4).GetFloat();
    glm::vec4 color = GetInputValue(5).GetColor();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (index >= 0 && (uint32_t)index < cloud->GetNumPoints())
    {
        cloud->SetPosition((uint32_t)index, pos);
        cloud->SetAttributeFloat((uint32_t)index, "pscale", pscale);
        cloud->SetAttributeColor((uint32_t)index, "Cd", color);
    }

    SetOutputValue(1, MakePointCloudDatum(cloud));
    TriggerExecutionPin(0);
}

glm::vec4 SetPointInLoopNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// FilterPointsNode
// =============================================================================
DEFINE_GRAPH_NODE(FilterPointsNode);

void FilterPointsNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Attribute", DatumType::String, Datum(std::string("pscale")));
    AddInputPin("Min", DatumType::Float, Datum(0.0f));
    AddInputPin("Max", DatumType::Float, Datum(1.0f));
    AddOutputPin("Passed", DatumType::PointCloud);
    AddOutputPin("Rejected", DatumType::PointCloud);
}

void FilterPointsNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    const std::string& attrName = GetInputValue(1).GetString();
    float minVal = GetInputValue(2).GetFloat();
    float maxVal = GetInputValue(3).GetFloat();

    PointCloud* passed = new PointCloud();
    PointCloud* rejected = new PointCloud();

    if (input)
    {
        // Copy attributes structure
        for (const auto& pair : input->GetAttributes())
        {
            passed->AddAttribute(pair.first, pair.second.mType);
            rejected->AddAttribute(pair.first, pair.second.mType);
        }

        for (uint32_t i = 0; i < input->GetNumPoints(); ++i)
        {
            float val = input->GetAttributeFloat(i, attrName);
            PointCloud* target = (val >= minVal && val <= maxVal) ? passed : rejected;

            uint32_t newIdx = target->GetNumPoints();
            target->AddPoint(input->GetPosition(i));

            // Copy all attributes
            for (const auto& pair : input->GetAttributes())
            {
                switch (pair.second.mType)
                {
                case DatumType::Float: target->SetAttributeFloat(newIdx, pair.first, input->GetAttributeFloat(i, pair.first)); break;
                case DatumType::Integer: target->SetAttributeInt(newIdx, pair.first, input->GetAttributeInt(i, pair.first)); break;
                case DatumType::Vector: target->SetAttributeVector(newIdx, pair.first, input->GetAttributeVector(i, pair.first)); break;
                case DatumType::Color: target->SetAttributeColor(newIdx, pair.first, input->GetAttributeColor(i, pair.first)); break;
                case DatumType::String: target->SetAttributeString(newIdx, pair.first, input->GetAttributeString(i, pair.first)); break;
                default: break;
                }
            }
        }
    }

    SetOutputValue(0, MakePointCloudDatum(passed));
    SetOutputValue(1, MakePointCloudDatum(rejected));
}

glm::vec4 FilterPointsNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// TransformPointsNode
// =============================================================================
DEFINE_GRAPH_NODE(TransformPointsNode);

void TransformPointsNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Translate", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Rotate", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Scale", DatumType::Vector, Datum(glm::vec3(1.0f)));
    AddOutputPin("Points", DatumType::PointCloud);
}

void TransformPointsNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    glm::vec3 translate = GetInputValue(1).GetVector();
    glm::vec3 rotate = GetInputValue(2).GetVector();
    glm::vec3 scale = GetInputValue(3).GetVector();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();

    // Build rotation matrix from euler angles (degrees)
    glm::mat4 rotMat = glm::mat4(1.0f);
    rotMat = glm::rotate(rotMat, glm::radians(rotate.x), glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, glm::radians(rotate.y), glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, glm::radians(rotate.z), glm::vec3(0, 0, 1));

    for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
    {
        glm::vec3 pos = cloud->GetPosition(i);
        pos = pos * scale;
        pos = glm::vec3(rotMat * glm::vec4(pos, 1.0f));
        pos = pos + translate;
        cloud->SetPosition(i, pos);
    }

    // Also rotate normals if present
    if (cloud->HasAttribute("N"))
    {
        for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
        {
            glm::vec3 n = cloud->GetAttributeVector(i, "N");
            n = glm::normalize(glm::vec3(rotMat * glm::vec4(n, 0.0f)));
            cloud->SetAttributeVector(i, "N", n);
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 TransformPointsNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// RandomizeAttributeNode
// =============================================================================
DEFINE_GRAPH_NODE(RandomizeAttributeNode);

void RandomizeAttributeNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Name", DatumType::String, Datum(std::string("pscale")));
    AddInputPin("Min", DatumType::Float, Datum(0.5f));
    AddInputPin("Max", DatumType::Float, Datum(1.5f));
    AddInputPin("Seed", DatumType::Integer, Datum(0));
    AddOutputPin("Points", DatumType::PointCloud);
}

void RandomizeAttributeNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    const std::string& name = GetInputValue(1).GetString();
    float minVal = GetInputValue(2).GetFloat();
    float maxVal = GetInputValue(3).GetFloat();
    int32_t seed = GetInputValue(4).GetInteger();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (!cloud->HasAttribute(name))
        cloud->AddAttribute(name, DatumType::Float);

    for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
    {
        float r = SeededRandom((uint32_t)seed, i);
        float value = glm::mix(minVal, maxVal, r);
        cloud->SetAttributeFloat(i, name, value);
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 RandomizeAttributeNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// CopyToPointsNode
// =============================================================================
DEFINE_GRAPH_NODE(CopyToPointsNode);

void CopyToPointsNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Mesh", DatumType::Asset, Datum((Asset*)nullptr));
    AddInputPin("Target", DatumType::Node3D, Datum((Node*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
}

void CopyToPointsNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 1);
    Asset* meshAsset = GetInputValue(2).GetAsset();
    Node* targetNode = GetInputValue(3).GetNode().Get();

    InstancedMesh3D* instMesh = targetNode ? targetNode->As<InstancedMesh3D>() : nullptr;
    StaticMesh* mesh = meshAsset ? meshAsset->As<StaticMesh>() : nullptr;

    if (instMesh && cloud && cloud->GetNumPoints() > 0)
    {
        if (mesh)
            instMesh->SetStaticMesh(mesh);

        std::vector<MeshInstanceData> instances;
        instances.resize(cloud->GetNumPoints());

        for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
        {
            MeshInstanceData& inst = instances[i];
            inst.mPosition = cloud->GetPosition(i);

            // Scale from pscale
            float pscale = cloud->GetPScale(i);
            inst.mScale = glm::vec3(pscale);

            // Rotation from normal direction
            glm::vec3 normal = cloud->GetNormal(i);
            if (glm::length(normal) > 0.001f)
            {
                normal = glm::normalize(normal);
                // Convert normal direction to euler rotation
                // Assuming default up is (0,1,0)
                glm::vec3 up(0.0f, 1.0f, 0.0f);
                if (glm::abs(glm::dot(normal, up)) < 0.999f)
                {
                    float pitch = glm::degrees(glm::asin(-normal.z));
                    float yaw = glm::degrees(glm::atan(normal.x, normal.y));
                    inst.mRotation = glm::vec3(pitch, yaw, 0.0f);
                }
            }
        }

        instMesh->SetInstanceData(instances);
        instMesh->MarkInstanceDataDirty();
    }

    TriggerExecutionPin(0);
}

glm::vec4 CopyToPointsNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// InstantiateAtPointsNode
// =============================================================================
DEFINE_GRAPH_NODE(InstantiateAtPointsNode);

void InstantiateAtPointsNode::SetupPins()
{
    AddInputPin("Exec", DatumType::Execution);
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Scene", DatumType::Scene, Datum((Asset*)nullptr));
    AddOutputPin("Exec", DatumType::Execution);
    AddOutputPin("Spawned Count", DatumType::Integer);
}

void InstantiateAtPointsNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 1);
    Asset* sceneAsset = GetInputValue(2).GetAsset();

    int32_t spawnedCount = 0;

    if (cloud && sceneAsset)
    {
        Node* owner = GetOwnerNode(this);
        World* world = owner ? owner->GetWorld() : nullptr;
        if (world)
        {
            for (uint32_t i = 0; i < cloud->GetNumPoints(); ++i)
            {
                Node* spawned = world->SpawnScene(sceneAsset->GetName().c_str());
                if (spawned)
                {
                    Node3D* node3d = spawned->As<Node3D>();
                    if (node3d)
                    {
                        node3d->SetPosition(cloud->GetPosition(i));
                        float pscale = cloud->GetPScale(i);
                        node3d->SetScale(glm::vec3(pscale));
                    }
                    spawnedCount++;
                }
            }
        }
    }

    SetOutputValue(1, Datum(spawnedCount));
    TriggerExecutionPin(0);
}

glm::vec4 InstantiateAtPointsNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// PointCloudToProceduralOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(PointCloudToProceduralOutputNode);

void PointCloudToProceduralOutputNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddOutputPin("Height", DatumType::Float);
    AddOutputPin("Color", DatumType::Color);
    AddOutputPin("Density", DatumType::Float);
    AddOutputPin("Mask", DatumType::Float);
}

void PointCloudToProceduralOutputNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();

    float height = 0.0f;
    glm::vec4 color(1.0f);
    float density = 1.0f;
    float mask = 1.0f;

    if (cloud && index >= 0 && (uint32_t)index < cloud->GetNumPoints())
    {
        height = cloud->GetPosition((uint32_t)index).y;
        color = cloud->GetCd((uint32_t)index);
        density = cloud->GetAttributeFloat((uint32_t)index, "density");
        mask = cloud->GetAttributeFloat((uint32_t)index, "mask");
        if (!cloud->HasAttribute("density")) density = 1.0f;
        if (!cloud->HasAttribute("mask")) mask = 1.0f;
    }

    SetOutputValue(0, Datum(height));
    SetOutputValue(1, Datum(color));
    SetOutputValue(2, Datum(density));
    SetOutputValue(3, Datum(mask));
}

glm::vec4 PointCloudToProceduralOutputNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// PointCloudFirstPointNode
// =============================================================================
DEFINE_GRAPH_NODE(PointCloudFirstPointNode);

void PointCloudFirstPointNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddOutputPin("Position", DatumType::Vector);
    AddOutputPin("Index", DatumType::Integer);
}

void PointCloudFirstPointNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    glm::vec3 pos(0.0f);
    if (cloud && cloud->GetNumPoints() > 0)
        pos = cloud->GetPosition(0);

    SetOutputValue(0, Datum(pos));
    SetOutputValue(1, Datum(0));
}

glm::vec4 PointCloudFirstPointNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// PointCloudLastPointNode
// =============================================================================
DEFINE_GRAPH_NODE(PointCloudLastPointNode);

void PointCloudLastPointNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddOutputPin("Position", DatumType::Vector);
    AddOutputPin("Index", DatumType::Integer);
}

void PointCloudLastPointNode::Evaluate()
{
    PointCloud* cloud = GetInputPointCloud(this, 0);
    glm::vec3 pos(0.0f);
    int32_t index = 0;
    if (cloud && cloud->GetNumPoints() > 0)
    {
        index = (int32_t)(cloud->GetNumPoints() - 1);
        pos = cloud->GetPosition((uint32_t)index);
    }

    SetOutputValue(0, Datum(pos));
    SetOutputValue(1, Datum(index));
}

glm::vec4 PointCloudLastPointNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// RemovePointNode
// =============================================================================
DEFINE_GRAPH_NODE(RemovePointNode);

void RemovePointNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddOutputPin("Points", DatumType::PointCloud);
}

void RemovePointNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    int32_t index = GetInputValue(1).GetInteger();

    PointCloud* cloud = input ? input->Clone() : new PointCloud();
    if (index >= 0 && (uint32_t)index < cloud->GetNumPoints())
        cloud->RemovePoint((uint32_t)index);

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 RemovePointNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// ReversePointsNode
// =============================================================================
DEFINE_GRAPH_NODE(ReversePointsNode);

void ReversePointsNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddOutputPin("Points", DatumType::PointCloud);
}

void ReversePointsNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    if (!input || input->GetNumPoints() == 0)
    {
        SetOutputValue(0, MakePointCloudDatum(new PointCloud()));
        return;
    }

    // Build reversed cloud
    PointCloud* cloud = new PointCloud();
    uint32_t count = input->GetNumPoints();

    // Copy attributes structure
    for (const auto& pair : input->GetAttributes())
        cloud->AddAttribute(pair.first, pair.second.mType);

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t srcIdx = count - 1 - i;
        cloud->AddPoint(input->GetPosition(srcIdx));

        for (const auto& pair : input->GetAttributes())
        {
            switch (pair.second.mType)
            {
            case DatumType::Float: cloud->SetAttributeFloat(i, pair.first, input->GetAttributeFloat(srcIdx, pair.first)); break;
            case DatumType::Integer: cloud->SetAttributeInt(i, pair.first, input->GetAttributeInt(srcIdx, pair.first)); break;
            case DatumType::Vector: cloud->SetAttributeVector(i, pair.first, input->GetAttributeVector(srcIdx, pair.first)); break;
            case DatumType::Color: cloud->SetAttributeColor(i, pair.first, input->GetAttributeColor(srcIdx, pair.first)); break;
            case DatumType::String: cloud->SetAttributeString(i, pair.first, input->GetAttributeString(srcIdx, pair.first)); break;
            default: break;
            }
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 ReversePointsNode::GetNodeColor() const { return kPointNodeColor; }

// =============================================================================
// SortPointsByAttributeNode
// =============================================================================
DEFINE_GRAPH_NODE(SortPointsByAttributeNode);

void SortPointsByAttributeNode::SetupPins()
{
    AddInputPin("Points", DatumType::PointCloud);
    AddInputPin("Attribute", DatumType::String, Datum(std::string("pscale")));
    AddInputPin("Ascending", DatumType::Bool, Datum(true));
    AddOutputPin("Points", DatumType::PointCloud);
}

void SortPointsByAttributeNode::Evaluate()
{
    PointCloud* input = GetInputPointCloud(this, 0);
    const std::string& attrName = GetInputValue(1).GetString();
    bool ascending = GetInputValue(2).GetBool();

    if (!input || input->GetNumPoints() == 0)
    {
        SetOutputValue(0, MakePointCloudDatum(new PointCloud()));
        return;
    }

    uint32_t count = input->GetNumPoints();

    // Create index array and sort by attribute
    std::vector<uint32_t> indices(count);
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [&](uint32_t a, uint32_t b)
    {
        float va = input->GetAttributeFloat(a, attrName);
        float vb = input->GetAttributeFloat(b, attrName);
        return ascending ? (va < vb) : (va > vb);
    });

    // Build sorted cloud
    PointCloud* cloud = new PointCloud();
    for (const auto& pair : input->GetAttributes())
        cloud->AddAttribute(pair.first, pair.second.mType);

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t srcIdx = indices[i];
        cloud->AddPoint(input->GetPosition(srcIdx));

        for (const auto& pair : input->GetAttributes())
        {
            switch (pair.second.mType)
            {
            case DatumType::Float: cloud->SetAttributeFloat(i, pair.first, input->GetAttributeFloat(srcIdx, pair.first)); break;
            case DatumType::Integer: cloud->SetAttributeInt(i, pair.first, input->GetAttributeInt(srcIdx, pair.first)); break;
            case DatumType::Vector: cloud->SetAttributeVector(i, pair.first, input->GetAttributeVector(srcIdx, pair.first)); break;
            case DatumType::Color: cloud->SetAttributeColor(i, pair.first, input->GetAttributeColor(srcIdx, pair.first)); break;
            case DatumType::String: cloud->SetAttributeString(i, pair.first, input->GetAttributeString(srcIdx, pair.first)); break;
            default: break;
            }
        }
    }

    SetOutputValue(0, MakePointCloudDatum(cloud));
}

glm::vec4 SortPointsByAttributeNode::GetNodeColor() const { return kPointNodeColor; }
