#include "NodeGraph/Nodes/AnimationNodes.h"
#include "Utilities.h"

FORCE_LINK_DEF(AnimationNodes);

static const glm::vec4 kAnimationNodeColor = glm::vec4(0.2f, 0.7f, 0.3f, 1.0f);

// =============================================================================
// AnimationOutputNode
// =============================================================================
DEFINE_GRAPH_NODE(AnimationOutputNode);

void AnimationOutputNode::SetupPins()
{
    AddInputPin("Pose", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Blend Weight", DatumType::Float, Datum(1.0f));
    AddInputPin("Speed", DatumType::Float, Datum(1.0f));
}

void AnimationOutputNode::Evaluate()
{
    // Sink node - collects final animation parameters.
}

glm::vec4 AnimationOutputNode::GetNodeColor() const { return kAnimationNodeColor; }

// =============================================================================
// AnimClipNode
// =============================================================================
DEFINE_GRAPH_NODE(AnimClipNode);

void AnimClipNode::SetupPins()
{
    AddOutputPin("Pose", DatumType::Vector);
    AddOutputPin("Duration", DatumType::Float);
}

void AnimClipNode::Evaluate()
{
    // Placeholder - actual animation clip data would come from the animation system.
    SetOutputValue(0, Datum(glm::vec3(0.0f)));
    SetOutputValue(1, Datum(1.0f));
}

glm::vec4 AnimClipNode::GetNodeColor() const { return kAnimationNodeColor; }

// =============================================================================
// BlendNode
// =============================================================================
DEFINE_GRAPH_NODE(BlendNode);

void BlendNode::SetupPins()
{
    AddInputPin("Pose A", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Pose B", DatumType::Vector, Datum(glm::vec3(0.0f)));
    AddInputPin("Alpha", DatumType::Float, Datum(0.5f));
    AddOutputPin("Result", DatumType::Vector);
}

void BlendNode::Evaluate()
{
    glm::vec3 poseA = GetInputValue(0).GetVector();
    glm::vec3 poseB = GetInputValue(1).GetVector();
    float alpha = glm::clamp(GetInputValue(2).GetFloat(), 0.0f, 1.0f);
    SetOutputValue(0, Datum(glm::mix(poseA, poseB, alpha)));
}

glm::vec4 BlendNode::GetNodeColor() const { return kAnimationNodeColor; }

// =============================================================================
// AnimSpeedNode
// =============================================================================
DEFINE_GRAPH_NODE(AnimSpeedNode);

void AnimSpeedNode::SetupPins()
{
    AddInputPin("Speed", DatumType::Float, Datum(1.0f));
    AddInputPin("Time", DatumType::Float, Datum(0.0f));
    AddOutputPin("Scaled Time", DatumType::Float);
}

void AnimSpeedNode::Evaluate()
{
    float speed = GetInputValue(0).GetFloat();
    float time = GetInputValue(1).GetFloat();
    SetOutputValue(0, Datum(speed * time));
}

glm::vec4 AnimSpeedNode::GetNodeColor() const { return kAnimationNodeColor; }
