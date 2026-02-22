#include "NodeGraph/Nodes/SplineNodes.h"
#include "Nodes/3D/Spline3d.h"
#include "Utilities.h"

FORCE_LINK_DEF(SplineNodes);

static const glm::vec4 kSplineNodeColor = glm::vec4(0.3f, 0.7f, 0.9f, 1.0f);

// Helper: get Spline3D from a Spline3D pin
static Spline3D* GetInputSpline(const GraphNode* node, uint32_t pinIndex)
{
    Node* n = node->GetInputValue(pinIndex).GetNode().Get();
    return n ? n->As<Spline3D>() : nullptr;
}

// =============================================================================
// SplinePositionAtNode
// =============================================================================
DEFINE_GRAPH_NODE(SplinePositionAtNode);

void SplinePositionAtNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddInputPin("Percent", DatumType::Float, Datum(0.5f));
    AddOutputPin("Position", DatumType::Vector);
}

void SplinePositionAtNode::Evaluate()
{
    Spline3D* spline = GetInputSpline(this, 0);
    float t = GetInputValue(1).GetFloat();

    glm::vec3 pos(0.0f);
    if (spline && spline->GetPointCount() >= 2)
        pos = spline->GetPositionAt(glm::clamp(t, 0.0f, 1.0f));

    SetOutputValue(0, Datum(pos));
}

glm::vec4 SplinePositionAtNode::GetNodeColor() const { return kSplineNodeColor; }

// =============================================================================
// SplineTangentAtNode
// =============================================================================
DEFINE_GRAPH_NODE(SplineTangentAtNode);

void SplineTangentAtNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddInputPin("Percent", DatumType::Float, Datum(0.5f));
    AddOutputPin("Tangent", DatumType::Vector);
}

void SplineTangentAtNode::Evaluate()
{
    Spline3D* spline = GetInputSpline(this, 0);
    float t = GetInputValue(1).GetFloat();

    glm::vec3 tangent(0.0f, 0.0f, 1.0f);
    if (spline && spline->GetPointCount() >= 2)
        tangent = spline->GetTangentAt(glm::clamp(t, 0.0f, 1.0f));

    SetOutputValue(0, Datum(tangent));
}

glm::vec4 SplineTangentAtNode::GetNodeColor() const { return kSplineNodeColor; }

// =============================================================================
// SplinePointCountNode
// =============================================================================
DEFINE_GRAPH_NODE(SplinePointCountNode);

void SplinePointCountNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddOutputPin("Count", DatumType::Integer);
}

void SplinePointCountNode::Evaluate()
{
    Spline3D* spline = GetInputSpline(this, 0);
    SetOutputValue(0, Datum(spline ? (int32_t)spline->GetPointCount() : 0));
}

glm::vec4 SplinePointCountNode::GetNodeColor() const { return kSplineNodeColor; }

// =============================================================================
// SplineGetPointNode
// =============================================================================
DEFINE_GRAPH_NODE(SplineGetPointNode);

void SplineGetPointNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddInputPin("Index", DatumType::Integer, Datum(0));
    AddOutputPin("Position", DatumType::Vector);
}

void SplineGetPointNode::Evaluate()
{
    Spline3D* spline = GetInputSpline(this, 0);
    int32_t index = GetInputValue(1).GetInteger();

    glm::vec3 pos(0.0f);
    if (spline && index >= 0 && (uint32_t)index < spline->GetPointCount())
        pos = spline->GetPoint((uint32_t)index);

    SetOutputValue(0, Datum(pos));
}

glm::vec4 SplineGetPointNode::GetNodeColor() const { return kSplineNodeColor; }

// =============================================================================
// SplineNearestPercentNode
// =============================================================================
DEFINE_GRAPH_NODE(SplineNearestPercentNode);

void SplineNearestPercentNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddInputPin("Position", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Samples", DatumType::Integer, Datum(100));
    AddOutputPin("Percent", DatumType::Float);
    AddOutputPin("Distance", DatumType::Float);
}

void SplineNearestPercentNode::Evaluate()
{
    Spline3D* spline = GetInputSpline(this, 0);
    glm::vec3 worldPos = GetInputValue(1).GetVector();
    int32_t samples = glm::max(GetInputValue(2).GetInteger(), 2);

    float bestT = 0.0f;
    float bestDist = FLT_MAX;

    if (spline && spline->GetPointCount() >= 2)
    {
        for (int32_t i = 0; i <= samples; ++i)
        {
            float t = (float)i / (float)samples;
            glm::vec3 pos = spline->GetPositionAt(t);
            float dist = glm::distance(worldPos, pos);
            if (dist < bestDist)
            {
                bestDist = dist;
                bestT = t;
            }
        }
    }

    SetOutputValue(0, Datum(bestT));
    SetOutputValue(1, Datum(bestDist));
}

glm::vec4 SplineNearestPercentNode::GetNodeColor() const { return kSplineNodeColor; }

// =============================================================================
// SplineLengthNode
// =============================================================================
DEFINE_GRAPH_NODE(SplineLengthNode);

void SplineLengthNode::SetupPins()
{
    AddInputPin("Spline", DatumType::Spline3D, Datum((Node*)nullptr));
    AddInputPin("Samples", DatumType::Integer, Datum(50));
    AddOutputPin("Length", DatumType::Float);
}

void SplineLengthNode::Evaluate()
{
    Spline3D* spline = GetInputSpline(this, 0);
    int32_t samples = glm::max(GetInputValue(1).GetInteger(), 2);

    float length = 0.0f;

    if (spline && spline->GetPointCount() >= 2)
    {
        glm::vec3 prev = spline->GetPositionAt(0.0f);
        for (int32_t i = 1; i <= samples; ++i)
        {
            float t = (float)i / (float)samples;
            glm::vec3 cur = spline->GetPositionAt(t);
            length += glm::distance(prev, cur);
            prev = cur;
        }
    }

    SetOutputValue(0, Datum(length));
}

glm::vec4 SplineLengthNode::GetNodeColor() const { return kSplineNodeColor; }
