#include "Nodes/TimelinePlayer.h"
#include "Assets/Timeline.h"
#include "Timeline/TimelineTrack.h"
#include "Timeline/TimelineInstance.h"
#include "World.h"
#include "Log.h"
#include "Utilities.h"

FORCE_LINK_DEF(TimelinePlayer);
DEFINE_NODE(TimelinePlayer, Node);

TimelinePlayer::TimelinePlayer()
{
    mName = "TimelinePlayer";
}

TimelinePlayer::~TimelinePlayer()
{
}

void TimelinePlayer::Create()
{
    Node::Create();
}

void TimelinePlayer::Destroy()
{
    if (mInstance != nullptr)
    {
        delete mInstance;
        mInstance = nullptr;
    }

    Node::Destroy();
}

void TimelinePlayer::Tick(float deltaTime)
{
    Node::Tick(deltaTime);

    if (mPlaying && !mPaused)
    {
        Timeline* timeline = mTimeline.Get<Timeline>();
        if (timeline == nullptr)
            return;

        float playRate = timeline->GetPlayRate();
        mCurrentTime += deltaTime * playRate;

        float duration = timeline->GetDuration();

        if (mCurrentTime >= duration)
        {
            if (timeline->IsLooping())
            {
                while (mCurrentTime >= duration)
                {
                    mCurrentTime -= duration;
                }
            }
            else
            {
                mCurrentTime = duration;
                mPlaying = false;
            }
        }

        EvaluateTimeline(mCurrentTime);
    }
}

void TimelinePlayer::EditorTick(float deltaTime)
{
    Node::EditorTick(deltaTime);
}

void TimelinePlayer::Start()
{
    Node::Start();

    if (mPlayOnStart)
    {
        Play();
    }
}

void TimelinePlayer::Stop()
{
    StopPlayback();
    Node::Stop();
}

void TimelinePlayer::GatherProperties(std::vector<Property>& outProps)
{
    Node::GatherProperties(outProps);

    SCOPED_CATEGORY("Timeline");

    outProps.push_back(Property(DatumType::Asset, "Timeline", this, &mTimeline, 1, nullptr, int32_t(Timeline::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Play On Start", this, &mPlayOnStart));
}

const char* TimelinePlayer::GetTypeName() const
{
    return "TimelinePlayer";
}

void TimelinePlayer::Play()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr)
    {
        LogWarning("TimelinePlayer::Play() - No timeline asset assigned");
        return;
    }

    EnsureInstance();

    if (mInstance != nullptr)
    {
        mInstance->ResolveBindings(GetWorld(), timeline->GetTracks());
    }

    mPlaying = true;
    mPaused = false;

    if (mCurrentTime >= timeline->GetDuration())
    {
        mCurrentTime = 0.0f;
    }
}

void TimelinePlayer::Pause()
{
    mPaused = true;
}

void TimelinePlayer::StopPlayback()
{
    Timeline* timeline = mTimeline.Get<Timeline>();

    if (mInstance != nullptr && timeline != nullptr)
    {
        const std::vector<TimelineTrack*>& tracks = timeline->GetTracks();
        for (uint32_t i = 0; i < tracks.size(); ++i)
        {
            TrackInstanceData& data = mInstance->GetTrackData(i);
            tracks[i]->Reset(data.mResolvedNode, mInstance);
        }
        mInstance->ResetAll();
    }

    mPlaying = false;
    mPaused = false;
    mCurrentTime = 0.0f;
}

void TimelinePlayer::SetTime(float time)
{
    mCurrentTime = time;

    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline != nullptr)
    {
        mCurrentTime = glm::clamp(mCurrentTime, 0.0f, timeline->GetDuration());
    }

    EvaluateTimeline(mCurrentTime);
}

float TimelinePlayer::GetTime() const
{
    return mCurrentTime;
}

float TimelinePlayer::GetDuration() const
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline != nullptr)
    {
        return timeline->GetDuration();
    }
    return 0.0f;
}

bool TimelinePlayer::IsPlaying() const
{
    return mPlaying && !mPaused;
}

bool TimelinePlayer::IsPaused() const
{
    return mPaused;
}

void TimelinePlayer::SetTimeline(Timeline* timeline)
{
    mTimeline = timeline;

    if (mInstance != nullptr)
    {
        delete mInstance;
        mInstance = nullptr;
    }
}

Timeline* TimelinePlayer::GetTimeline() const
{
    return mTimeline.Get<Timeline>();
}

void TimelinePlayer::EnsureInstance()
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr)
        return;

    if (mInstance == nullptr)
    {
        mInstance = new TimelineInstance();
    }

    mInstance->SetTrackCount(timeline->GetNumTracks());
}

void TimelinePlayer::EvaluateTimeline(float time)
{
    Timeline* timeline = mTimeline.Get<Timeline>();
    if (timeline == nullptr)
        return;

    EnsureInstance();

    if (mInstance == nullptr)
        return;

    mInstance->ResolveBindings(GetWorld(), timeline->GetTracks());

    const std::vector<TimelineTrack*>& tracks = timeline->GetTracks();
    for (uint32_t i = 0; i < tracks.size(); ++i)
    {
        if (tracks[i]->IsMuted())
            continue;

        TrackInstanceData& data = mInstance->GetTrackData(i);
        tracks[i]->Evaluate(time, data.mResolvedNode, mInstance);
    }
}
