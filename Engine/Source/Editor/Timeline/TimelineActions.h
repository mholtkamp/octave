#pragma once

#if EDITOR

#include "ActionManager.h"
#include "AssetRef.h"
#include "Stream.h"

class Timeline;
class TimelineTrack;
class TimelineClip;

// ======= Add Track =======
class ActionTimelineAddTrack : public Action
{
public:
    DECLARE_ACTION_INTERFACE(TimelineAddTrack);

    ActionTimelineAddTrack(Timeline* timeline, TypeId trackType);

protected:
    TimelineRef mTimeline;
    TypeId mTrackType = INVALID_TYPE_ID;
    int32_t mTrackIndex = -1;
};

// ======= Remove Track =======
class ActionTimelineRemoveTrack : public Action
{
public:
    DECLARE_ACTION_INTERFACE(TimelineRemoveTrack);

    ActionTimelineRemoveTrack(Timeline* timeline, int32_t trackIndex);

protected:
    TimelineRef mTimeline;
    int32_t mTrackIndex = -1;
    std::vector<uint8_t> mSerializedData;
    TypeId mTrackType = INVALID_TYPE_ID;
};

// ======= Add Clip =======
class ActionTimelineAddClip : public Action
{
public:
    DECLARE_ACTION_INTERFACE(TimelineAddClip);

    ActionTimelineAddClip(Timeline* timeline, int32_t trackIndex, TypeId clipType, float startTime, float duration);

protected:
    TimelineRef mTimeline;
    int32_t mTrackIndex = -1;
    int32_t mClipIndex = -1;
    TypeId mClipType = INVALID_TYPE_ID;
    float mStartTime = 0.0f;
    float mDuration = 1.0f;
};

// ======= Remove Clip =======
class ActionTimelineRemoveClip : public Action
{
public:
    DECLARE_ACTION_INTERFACE(TimelineRemoveClip);

    ActionTimelineRemoveClip(Timeline* timeline, int32_t trackIndex, int32_t clipIndex);

protected:
    TimelineRef mTimeline;
    int32_t mTrackIndex = -1;
    int32_t mClipIndex = -1;
    std::vector<uint8_t> mSerializedData;
    TypeId mClipType = INVALID_TYPE_ID;
};

// ======= Move Clip =======
class ActionTimelineMoveClip : public Action
{
public:
    DECLARE_ACTION_INTERFACE(TimelineMoveClip);

    ActionTimelineMoveClip(Timeline* timeline, int32_t trackIndex, int32_t clipIndex, float oldStartTime, float newStartTime);

protected:
    TimelineRef mTimeline;
    int32_t mTrackIndex = -1;
    int32_t mClipIndex = -1;
    float mOldStartTime = 0.0f;
    float mNewStartTime = 0.0f;
};

// ======= Bind Track (change target node) =======
class ActionTimelineBindTrack : public Action
{
public:
    DECLARE_ACTION_INTERFACE(TimelineBindTrack);

    ActionTimelineBindTrack(Timeline* timeline, int32_t trackIndex, uint64_t oldUuid, uint64_t newUuid, const std::string& oldName, const std::string& newName);

protected:
    TimelineRef mTimeline;
    int32_t mTrackIndex = -1;
    uint64_t mOldUuid = 0;
    uint64_t mNewUuid = 0;
    std::string mOldName;
    std::string mNewName;
};

#endif
