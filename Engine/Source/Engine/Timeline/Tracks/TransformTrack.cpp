#include "Timeline/Tracks/TransformTrack.h"
#include "Timeline/Tracks/TransformClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/3D/Node3d.h"
#include "Utilities.h"

FORCE_LINK_DEF(TransformTrack);
DEFINE_TRACK(TransformTrack);

TransformTrack::TransformTrack()
{
}

TransformTrack::~TransformTrack()
{
}

void TransformTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr || !target->IsNode3D())
        return;

    Node3D* node3d = static_cast<Node3D*>(target);

    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != TransformClip::GetStaticType())
            continue;

        TransformClip* clip = static_cast<TransformClip*>(mClips[i]);

        if (clip->ContainsTime(time))
        {
            float localTime = clip->GetLocalTime(time);

            glm::vec3 pos;
            glm::quat rot;
            glm::vec3 scale;
            clip->EvaluateAtLocalTime(localTime, pos, rot, scale);

            node3d->SetPosition(pos);
            node3d->SetRotation(rot);
            node3d->SetScale(scale);
            break;
        }
    }
}

void TransformTrack::Reset(Node* target, TimelineInstance* inst)
{
}

glm::vec4 TransformTrack::GetTrackColor() const
{
    return glm::vec4(0.9f, 0.7f, 0.2f, 1.0f);
}

TypeId TransformTrack::GetDefaultClipType() const
{
    return TransformClip::GetStaticType();
}
