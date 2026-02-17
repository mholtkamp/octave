#pragma once

#include "EngineTypes.h"
#include "Maths.h"

#include <vector>
#include <unordered_map>

class Node;
class World;
class TimelineTrack;

struct TrackInstanceData
{
    Node* mResolvedNode = nullptr;
    bool mBindingResolved = false;

    // Pre-play state snapshots for editor preview undo
    glm::vec3 mPrePlayPosition = {};
    glm::quat mPrePlayRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 mPrePlayScale = glm::vec3(1.0f);
    bool mPrePlayActive = true;
    bool mPrePlayVisible = true;
    bool mStateSnapshotted = false;

    // Track-specific runtime state
    bool mClipPlaying = false;
    int32_t mLastEvaluatedClipIndex = -1;
};

class TimelineInstance
{
public:

    TimelineInstance();
    ~TimelineInstance();

    void SetTrackCount(uint32_t count);
    uint32_t GetTrackCount() const { return (uint32_t)mTrackData.size(); }

    TrackInstanceData& GetTrackData(uint32_t index);

    void ResolveBindings(World* world, const std::vector<TimelineTrack*>& tracks);

    void CapturePrePlayState(const std::vector<TimelineTrack*>& tracks);
    void RestorePrePlayState(const std::vector<TimelineTrack*>& tracks);

    void ResetAll();

private:

    std::vector<TrackInstanceData> mTrackData;
};
