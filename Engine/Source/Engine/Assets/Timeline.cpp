#include "Assets/Timeline.h"
#include "Log.h"
#include "Utilities.h"

FORCE_LINK_DEF(Timeline);
DEFINE_ASSET(Timeline);

Timeline::Timeline()
{
    mType = Timeline::GetStaticType();
}

Timeline::~Timeline()
{
}

void Timeline::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mDuration = stream.ReadFloat();
    mPlayRate = stream.ReadFloat();
    mLoop = stream.ReadBool();
    mAutoPlay = stream.ReadBool();

    uint32_t numTracks = stream.ReadUint32();
    for (uint32_t i = 0; i < numTracks; ++i)
    {
        TypeId trackType = (TypeId)stream.ReadUint32();
        TimelineTrack* track = TimelineTrack::CreateInstance(trackType);

        if (track != nullptr)
        {
            track->LoadStream(stream, mVersion);
            mTracks.push_back(track);
        }
        else
        {
            LogWarning("Failed to create timeline track of type %u", (uint32_t)trackType);
        }
    }
}

void Timeline::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteFloat(mDuration);
    stream.WriteFloat(mPlayRate);
    stream.WriteBool(mLoop);
    stream.WriteBool(mAutoPlay);

    stream.WriteUint32((uint32_t)mTracks.size());
    for (uint32_t i = 0; i < mTracks.size(); ++i)
    {
        stream.WriteUint32((uint32_t)mTracks[i]->GetType());
        mTracks[i]->SaveStream(stream);
    }
}

void Timeline::Create()
{
    Asset::Create();
}

void Timeline::Destroy()
{
    for (uint32_t i = 0; i < mTracks.size(); ++i)
    {
        delete mTracks[i];
    }
    mTracks.clear();

    Asset::Destroy();
}

void Timeline::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Float, "Duration", this, &mDuration));
    outProps.push_back(Property(DatumType::Float, "Play Rate", this, &mPlayRate));
    outProps.push_back(Property(DatumType::Bool, "Loop", this, &mLoop));
    outProps.push_back(Property(DatumType::Bool, "Auto Play", this, &mAutoPlay));
}

glm::vec4 Timeline::GetTypeColor()
{
    return glm::vec4(0.2f, 0.7f, 0.9f, 1.0f);
}

const char* Timeline::GetTypeName()
{
    return "Timeline";
}

TimelineTrack* Timeline::GetTrack(uint32_t index) const
{
    if (index < mTracks.size())
    {
        return mTracks[index];
    }
    return nullptr;
}

TimelineTrack* Timeline::AddTrack(TypeId trackType)
{
    TimelineTrack* track = TimelineTrack::CreateInstance(trackType);
    if (track != nullptr)
    {
        mTracks.push_back(track);
    }
    return track;
}

void Timeline::RemoveTrack(uint32_t index)
{
    if (index < mTracks.size())
    {
        delete mTracks[index];
        mTracks.erase(mTracks.begin() + index);
    }
}

void Timeline::MoveTrack(uint32_t from, uint32_t to)
{
    if (from < mTracks.size() && to < mTracks.size() && from != to)
    {
        TimelineTrack* track = mTracks[from];
        mTracks.erase(mTracks.begin() + from);

        if (to > mTracks.size())
        {
            to = (uint32_t)mTracks.size();
        }

        mTracks.insert(mTracks.begin() + to, track);
    }
}

void Timeline::InsertTrack(TimelineTrack* track, uint32_t index)
{
    if (track != nullptr)
    {
        if (index <= mTracks.size())
        {
            mTracks.insert(mTracks.begin() + index, track);
        }
        else
        {
            mTracks.push_back(track);
        }
    }
}
