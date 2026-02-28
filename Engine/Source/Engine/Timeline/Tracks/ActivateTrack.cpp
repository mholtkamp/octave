#include "Timeline/Tracks/ActivateTrack.h"
#include "Timeline/Tracks/ActivateClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/Node.h"
#include "Utilities.h"

FORCE_LINK_DEF(ActivateTrack);
DEFINE_TRACK(ActivateTrack);

ActivateTrack::ActivateTrack()
{
}

ActivateTrack::~ActivateTrack()
{
}

void ActivateTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    bool anyClipActive = false;
    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != ActivateClip::GetStaticType())
            continue;

        ActivateClip* clip = static_cast<ActivateClip*>(mClips[i]);

        if (clip->ContainsTime(time))
        {
            target->SetActive(clip->GetSetActive());
            target->SetVisible(clip->GetSetVisible());
            anyClipActive = true;
            break;
        }
    }

    // If no clip is active at this time, restore pre-play state
    if (!anyClipActive && inst != nullptr)
    {
        target->SetActive(false);
        target->SetVisible(false);
        // Find our track index to access instance data
        // The caller passes the correct instance data, so the track just
        // needs to know it's not in any clip range.
    }
}

void ActivateTrack::Reset(Node* target, TimelineInstance* inst)
{
}

glm::vec4 ActivateTrack::GetTrackColor() const
{
    return glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
}

TypeId ActivateTrack::GetDefaultClipType() const
{
    return ActivateClip::GetStaticType();
}
