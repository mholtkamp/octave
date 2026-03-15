#pragma once

#include "Nodes/Node.h"
#include "AssetRef.h"
#include "Timeline/TimelineInstance.h"

class Timeline;

class TimelinePlayer : public Node
{
public:

    DECLARE_NODE(TimelinePlayer, Node);

    TimelinePlayer();
    virtual ~TimelinePlayer();

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;
    virtual void Start() override;
    virtual void Stop() override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual const char* GetTypeName() const override;

    // Playback API
    void Play();
    void Pause();
    void StopPlayback();
    void SetTime(float time);
    float GetTime() const;
    float GetDuration() const;
    bool IsPlaying() const;
    bool IsPaused() const;

    // Asset
    void SetTimeline(Timeline* timeline);
    Timeline* GetTimeline() const;
    TimelineRef GetTimelineRef() const { return mTimeline; }

    // Settings
    void SetPlayOnStart(bool playOnStart) { mPlayOnStart = playOnStart; }
    bool GetPlayOnStart() const { return mPlayOnStart; }

    // Instance access (for editor preview)
    TimelineInstance* GetInstance() { return mInstance; }
    void EnsureInstance();

    void EvaluateTimeline(float time);

protected:

    TimelineRef mTimeline;
    TimelineInstance* mInstance = nullptr;
    float mCurrentTime = 0.0f;
    bool mPlaying = false;
    bool mPaused = false;
    bool mPlayOnStart = false;
};
