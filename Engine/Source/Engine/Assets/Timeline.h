#pragma once

#include "Asset.h"
#include "Factory.h"
#include "Timeline/TimelineTrack.h"

#include <vector>

class Timeline : public Asset
{
public:

    DECLARE_ASSET(Timeline, Asset);

    Timeline();
    ~Timeline();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    uint32_t GetNumTracks() const { return (uint32_t)mTracks.size(); }
    TimelineTrack* GetTrack(uint32_t index) const;
    const std::vector<TimelineTrack*>& GetTracks() const { return mTracks; }

    TimelineTrack* AddTrack(TypeId trackType);
    void RemoveTrack(uint32_t index);
    void MoveTrack(uint32_t from, uint32_t to);
    void InsertTrack(TimelineTrack* track, uint32_t index);

    float GetDuration() const { return mDuration; }
    void SetDuration(float duration) { mDuration = duration; }

    float GetPlayRate() const { return mPlayRate; }
    void SetPlayRate(float rate) { mPlayRate = rate; }

    bool IsLooping() const { return mLoop; }
    void SetLooping(bool loop) { mLoop = loop; }

    bool IsAutoPlay() const { return mAutoPlay; }
    void SetAutoPlay(bool autoPlay) { mAutoPlay = autoPlay; }

protected:

    std::vector<TimelineTrack*> mTracks;
    float mDuration = 5.0f;
    float mPlayRate = 1.0f;
    bool mLoop = false;
    bool mAutoPlay = false;
};
