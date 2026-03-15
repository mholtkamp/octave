#include "Timeline/Tracks/FunctionCallTrack.h"
#include "Timeline/Tracks/FunctionCallClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/Node.h"
#include "Script.h"
#include "Utilities.h"

FORCE_LINK_DEF(FunctionCallTrack);
DEFINE_TRACK(FunctionCallTrack);

FunctionCallTrack::FunctionCallTrack()
{
}

FunctionCallTrack::~FunctionCallTrack()
{
}

void FunctionCallTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    Script* script = target->GetScript();
    if (script == nullptr || !script->IsActive())
        return;

    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != FunctionCallClip::GetStaticType())
            continue;

        FunctionCallClip* clip = static_cast<FunctionCallClip*>(mClips[i]);

        if (clip->ContainsTime(time))
        {
            float localTime = clip->GetLocalTime(time);
            uint32_t numKf = clip->GetNumKeyframes();

            for (uint32_t k = 0; k < numKf; ++k)
            {
                float kfTime = clip->GetKeyframeTime(k);
                if (kfTime <= localTime && (int32_t)k > clip->mLastFiredKeyframe)
                {
                    const std::string& funcName = clip->GetKeyframeFunctionName(k);
                    if (!funcName.empty())
                    {
                        script->CallFunction(funcName.c_str());
                    }
                    clip->mLastFiredKeyframe = (int32_t)k;
                }
            }

            break;
        }
    }
}

void FunctionCallTrack::Reset(Node* target, TimelineInstance* inst)
{
    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != FunctionCallClip::GetStaticType())
            continue;

        FunctionCallClip* clip = static_cast<FunctionCallClip*>(mClips[i]);
        clip->mLastFiredKeyframe = -1;
    }
}

glm::vec4 FunctionCallTrack::GetTrackColor() const
{
    return glm::vec4(0.2f, 0.8f, 0.4f, 1.0f);
}

TypeId FunctionCallTrack::GetDefaultClipType() const
{
    return FunctionCallClip::GetStaticType();
}
