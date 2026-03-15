#include "Timeline/TimelineActions.h"

#if EDITOR

#include "Assets/Timeline.h"
#include "Timeline/TimelineTrack.h"
#include "Timeline/TimelineClip.h"
#include "Timeline/TimelineInstance.h"
#include "EditorState.h"
#include "Stream.h"
#include "Log.h"

// ======= Add Track =======

ActionTimelineAddTrack::ActionTimelineAddTrack(Timeline* timeline, TypeId trackType)
{
    mTimeline = timeline;
    mTrackType = trackType;
}

void ActionTimelineAddTrack::Execute()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr)
        return;

    if (mTrackIndex < 0)
    {
        // First execute: add to end
        TimelineTrack* track = timeline->AddTrack(mTrackType);
        if (track != nullptr)
        {
            mTrackIndex = (int32_t)timeline->GetNumTracks() - 1;
        }
    }
    else
    {
        // Re-execute after undo: re-create and insert at same index
        TimelineTrack* track = TimelineTrack::CreateInstance(mTrackType);
        if (track != nullptr)
        {
            timeline->InsertTrack(track, (uint32_t)mTrackIndex);
        }
    }
}

void ActionTimelineAddTrack::Reverse()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0)
        return;

    // Remove the track that was added (this deletes it)
    timeline->RemoveTrack((uint32_t)mTrackIndex);
}

// ======= Remove Track =======

ActionTimelineRemoveTrack::ActionTimelineRemoveTrack(Timeline* timeline, int32_t trackIndex)
{
    mTimeline = timeline;
    mTrackIndex = trackIndex;
}

void ActionTimelineRemoveTrack::Execute()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    // Serialize the track data before removing so we can restore on undo
    mTrackType = track->GetType();
    mSerializedData.clear();

    Stream stream;
    track->SaveStream(stream);
    uint32_t size = stream.GetSize();
    if (size > 0)
    {
        mSerializedData.resize(size);
        memcpy(mSerializedData.data(), stream.GetData(), size);
    }

    // Remove (and delete) the track
    timeline->RemoveTrack((uint32_t)mTrackIndex);
}

void ActionTimelineRemoveTrack::Reverse()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0)
        return;

    // Re-create the track from serialized data
    TimelineTrack* track = TimelineTrack::CreateInstance(mTrackType);
    if (track == nullptr)
        return;

    if (!mSerializedData.empty())
    {
        Stream stream((const char*)mSerializedData.data(), (uint32_t)mSerializedData.size());
        track->LoadStream(stream, ASSET_VERSION_CURRENT);
    }

    timeline->InsertTrack(track, (uint32_t)mTrackIndex);
}

// ======= Add Clip =======

ActionTimelineAddClip::ActionTimelineAddClip(Timeline* timeline, int32_t trackIndex, TypeId clipType, float startTime, float duration)
{
    mTimeline = timeline;
    mTrackIndex = trackIndex;
    mClipType = clipType;
    mStartTime = startTime;
    mDuration = duration;
}

void ActionTimelineAddClip::Execute()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    TimelineClip* clip = TimelineClip::CreateInstance(mClipType);
    if (clip == nullptr)
        return;

    clip->SetStartTime(mStartTime);
    clip->SetDuration(mDuration);

    if (mClipIndex < 0)
    {
        track->AddClip(clip);
        mClipIndex = (int32_t)track->GetNumClips() - 1;
    }
    else
    {
        track->InsertClip(clip, (uint32_t)mClipIndex);
    }
}

void ActionTimelineAddClip::Reverse()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0 || mClipIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    track->RemoveClip((uint32_t)mClipIndex);
}

// ======= Remove Clip =======

ActionTimelineRemoveClip::ActionTimelineRemoveClip(Timeline* timeline, int32_t trackIndex, int32_t clipIndex)
{
    mTimeline = timeline;
    mTrackIndex = trackIndex;
    mClipIndex = clipIndex;
}

void ActionTimelineRemoveClip::Execute()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0 || mClipIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    TimelineClip* clip = track->GetClip((uint32_t)mClipIndex);
    if (clip == nullptr)
        return;

    // Serialize clip data before removing
    mClipType = clip->GetType();
    mSerializedData.clear();

    Stream stream;
    clip->SaveStream(stream);
    uint32_t size = stream.GetSize();
    if (size > 0)
    {
        mSerializedData.resize(size);
        memcpy(mSerializedData.data(), stream.GetData(), size);
    }

    track->RemoveClip((uint32_t)mClipIndex);
}

void ActionTimelineRemoveClip::Reverse()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0 || mClipIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    // Re-create the clip from serialized data
    TimelineClip* clip = TimelineClip::CreateInstance(mClipType);
    if (clip == nullptr)
        return;

    if (!mSerializedData.empty())
    {
        Stream stream((const char*)mSerializedData.data(), (uint32_t)mSerializedData.size());
        clip->LoadStream(stream, ASSET_VERSION_CURRENT);
    }

    track->InsertClip(clip, (uint32_t)mClipIndex);
}

// ======= Move Clip =======

ActionTimelineMoveClip::ActionTimelineMoveClip(Timeline* timeline, int32_t trackIndex, int32_t clipIndex, float oldStartTime, float newStartTime)
{
    mTimeline = timeline;
    mTrackIndex = trackIndex;
    mClipIndex = clipIndex;
    mOldStartTime = oldStartTime;
    mNewStartTime = newStartTime;
}

void ActionTimelineMoveClip::Execute()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0 || mClipIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    TimelineClip* clip = track->GetClip((uint32_t)mClipIndex);
    if (clip == nullptr)
        return;

    clip->SetStartTime(mNewStartTime);
}

void ActionTimelineMoveClip::Reverse()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0 || mClipIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    TimelineClip* clip = track->GetClip((uint32_t)mClipIndex);
    if (clip == nullptr)
        return;

    clip->SetStartTime(mOldStartTime);
}

// ======= Bind Track =======

ActionTimelineBindTrack::ActionTimelineBindTrack(Timeline* timeline, int32_t trackIndex, uint64_t oldUuid, uint64_t newUuid, const std::string& oldName, const std::string& newName)
{
    mTimeline = timeline;
    mTrackIndex = trackIndex;
    mOldUuid = oldUuid;
    mNewUuid = newUuid;
    mOldName = oldName;
    mNewName = newName;
}

void ActionTimelineBindTrack::Execute()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    track->SetTargetNodeUuid(mNewUuid);
    track->SetTargetNodeName(mNewName);

    // Invalidate cached binding so the preview instance re-resolves the target
    TimelineInstance* inst = GetEditorState()->mTimelinePreviewInstance;
    if (inst != nullptr && mTrackIndex < (int32_t)timeline->GetNumTracks())
    {
        TrackInstanceData& data = inst->GetTrackData((uint32_t)mTrackIndex);
        data.mResolvedNode = nullptr;
        data.mBindingResolved = false;
    }
}

void ActionTimelineBindTrack::Reverse()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr || mTrackIndex < 0)
        return;

    TimelineTrack* track = timeline->GetTrack((uint32_t)mTrackIndex);
    if (track == nullptr)
        return;

    track->SetTargetNodeUuid(mOldUuid);
    track->SetTargetNodeName(mOldName);

    // Invalidate cached binding so the preview instance re-resolves the target
    TimelineInstance* inst = GetEditorState()->mTimelinePreviewInstance;
    if (inst != nullptr && mTrackIndex < (int32_t)timeline->GetNumTracks())
    {
        TrackInstanceData& data = inst->GetTrackData((uint32_t)mTrackIndex);
        data.mResolvedNode = nullptr;
        data.mBindingResolved = false;
    }
}

#endif
