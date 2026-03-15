#include "Timeline/Tracks/AnimationTrack.h"
#include "Timeline/Tracks/AnimationClip.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Assets/SkeletalMesh.h"
#include "Utilities.h"

FORCE_LINK_DEF(AnimationTrack);
DEFINE_TRACK(AnimationTrack);

AnimationTrack::AnimationTrack()
{
}

AnimationTrack::~AnimationTrack()
{
}

void AnimationTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    SkeletalMesh3D* skelMesh = target->As<SkeletalMesh3D>();
    if (skelMesh == nullptr)
        return;

    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        if (mClips[i]->GetType() != AnimationClip::GetStaticType())
            continue;

        AnimationClip* clip = static_cast<AnimationClip*>(mClips[i]);
        const std::string& animName = clip->GetAnimationName();
        if (animName.empty())
            continue;

        if (clip->ContainsTime(time))
        {
            float localTime = clip->GetLocalTime(time);
            float weight = clip->GetWeight();

            // Apply wrap mode when localTime exceeds animation duration
            AnimationWrapMode wrapMode = clip->GetWrapMode();
            if (wrapMode != AnimationWrapMode::Hold)
            {
                SkeletalMesh* mesh = skelMesh->GetSkeletalMesh();
                float animDuration = mesh ? mesh->GetAnimationDuration(animName.c_str()) : 0.0f;
                if (animDuration > 0.0f && localTime > animDuration)
                {
                    if (wrapMode == AnimationWrapMode::Loop)
                    {
                        localTime = fmod(localTime, animDuration);
                    }
                    else // PingPong
                    {
                        float cycle = localTime / animDuration;
                        int cycleInt = (int)cycle;
                        float frac = cycle - cycleInt;
                        localTime = (cycleInt % 2 == 0) ? (frac * animDuration) : ((1.0f - frac) * animDuration);
                    }
                }
            }

            // Ensure the animation is active
            ActiveAnimation* active = skelMesh->FindActiveAnimation(animName.c_str());
            if (active == nullptr)
            {
                skelMesh->PlayAnimation(animName.c_str(), false, 0.0f, weight);
                active = skelMesh->FindActiveAnimation(animName.c_str());
            }

            if (active != nullptr)
            {
                // Drive the animation time directly from the timeline
                active->mTime = localTime;
                active->mWeight = weight;
                active->mSpeed = 0.0f;
            }
        }
        else
        {
            // Playhead is outside this clip — stop its animation if active
            ActiveAnimation* active = skelMesh->FindActiveAnimation(animName.c_str());
            if (active != nullptr)
            {
                skelMesh->StopAnimation(animName.c_str());
            }
        }
    }
}

void AnimationTrack::Reset(Node* target, TimelineInstance* inst)
{
    if (target == nullptr)
        return;

    SkeletalMesh3D* skelMesh = target->As<SkeletalMesh3D>();
    if (skelMesh != nullptr)
    {
        skelMesh->StopAllAnimations();
    }
}

glm::vec4 AnimationTrack::GetTrackColor() const
{
    return glm::vec4(0.3f, 0.4f, 0.9f, 1.0f);
}

TypeId AnimationTrack::GetDefaultClipType() const
{
    return AnimationClip::GetStaticType();
}
