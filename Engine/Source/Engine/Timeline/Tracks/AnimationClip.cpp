#include "Timeline/Tracks/AnimationClip.h"
#include "Utilities.h"

#if EDITOR
#include "EditorState.h"
#include "Assets/Timeline.h"
#include "Timeline/TimelineTrack.h"
#include "Timeline/TimelineInstance.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Assets/SkeletalMesh.h"
#include "imgui.h"
#endif

FORCE_LINK_DEF(AnimationClip);
DEFINE_CLIP(AnimationClip);

AnimationClip::AnimationClip()
{
}

AnimationClip::~AnimationClip()
{
}

void AnimationClip::SaveStream(Stream& stream)
{
    TimelineClip::SaveStream(stream);

    stream.WriteString(mAnimationName);
    stream.WriteFloat(mBlendIn);
    stream.WriteFloat(mBlendOut);
    stream.WriteFloat(mWeight);
    stream.WriteUint32((uint32_t)mWrapMode);
}

void AnimationClip::LoadStream(Stream& stream, uint32_t version)
{
    TimelineClip::LoadStream(stream, version);

    stream.ReadString(mAnimationName);
    mBlendIn = stream.ReadFloat();
    mBlendOut = stream.ReadFloat();
    mWeight = stream.ReadFloat();
    mWrapMode = (AnimationWrapMode)stream.ReadUint32();
}

#if EDITOR
bool AnimationClip::DrawCustomProperty(Property& prop)
{
    if (prop.mName != "Animation Name")
        return false;

    // Find the SkeletalMesh3D bound to this track to list available animations.
    EditorState* state = GetEditorState();
    Timeline* timeline = state->mEditedTimelineRef.Get<Timeline>();
    SkeletalMesh* skelMesh = nullptr;

    if (timeline != nullptr && state->mTimelineSelectedTrack >= 0)
    {
        TimelineTrack* track = timeline->GetTrack(state->mTimelineSelectedTrack);
        if (track != nullptr && state->mTimelinePreviewInstance != nullptr)
        {
            TrackInstanceData& data = state->mTimelinePreviewInstance->GetTrackData(state->mTimelineSelectedTrack);
            if (data.mResolvedNode != nullptr)
            {
                SkeletalMesh3D* skelNode = data.mResolvedNode->As<SkeletalMesh3D>();
                if (skelNode != nullptr)
                {
                    skelMesh = skelNode->GetSkeletalMesh();
                }
            }
        }
    }

    ImGui::Text("Animation Name");

    if (skelMesh != nullptr)
    {
        const std::vector<Animation>& animations = skelMesh->GetAnimations();
        int currentIdx = -1;

        for (uint32_t i = 0; i < animations.size(); ++i)
        {
            if (animations[i].mName == mAnimationName)
            {
                currentIdx = (int)i;
                break;
            }
        }

        const char* preview = (currentIdx >= 0) ? animations[currentIdx].mName.c_str() : mAnimationName.c_str();

        if (ImGui::BeginCombo("##AnimName", preview))
        {
            // Option to clear
            if (ImGui::Selectable("(None)", mAnimationName.empty()))
            {
                mAnimationName.clear();
            }

            for (uint32_t i = 0; i < animations.size(); ++i)
            {
                bool selected = ((int)i == currentIdx);
                if (ImGui::Selectable(animations[i].mName.c_str(), selected))
                {
                    mAnimationName = animations[i].mName;
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
    else
    {
        // No skeletal mesh bound — fall back to text input
        static char sTempAnimName[256] = {};
        strncpy(sTempAnimName, mAnimationName.c_str(), sizeof(sTempAnimName) - 1);
        sTempAnimName[sizeof(sTempAnimName) - 1] = '\0';
        if (ImGui::InputText("##AnimName", sTempAnimName, sizeof(sTempAnimName), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            mAnimationName = sTempAnimName;
        }
    }

    return true;
}
#endif

void AnimationClip::GatherProperties(std::vector<Property>& outProps)
{
    TimelineClip::GatherProperties(outProps);

    SCOPED_CATEGORY("Animation");

    outProps.push_back(Property(DatumType::String, "Animation Name", this, &mAnimationName));
    outProps.push_back(Property(DatumType::Float, "Blend In", this, &mBlendIn));
    outProps.push_back(Property(DatumType::Float, "Blend Out", this, &mBlendOut));
    outProps.push_back(Property(DatumType::Float, "Weight", this, &mWeight));

    static const char* sWrapModeStrings[] = { "Loop", "Hold", "PingPong" };
    outProps.push_back(Property(DatumType::Integer, "Wrap Mode", this, &mWrapMode, 1, nullptr, NULL_DATUM, (int32_t)AnimationWrapMode::Count, sWrapModeStrings));
}
