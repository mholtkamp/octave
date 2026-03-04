#include "Timeline/TimelineTrack.h"
#include "Utilities.h"

FORCE_LINK_DEF(TimelineTrack);
DEFINE_FACTORY_MANAGER(TimelineTrack);
DEFINE_FACTORY(TimelineTrack, TimelineTrack);
DEFINE_OBJECT(TimelineTrack);

TimelineTrack::TimelineTrack()
{
}

TimelineTrack::~TimelineTrack()
{
    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        delete mClips[i];
    }
    mClips.clear();
}

void TimelineTrack::SaveStream(Stream& stream)
{
    stream.WriteUint64(mTargetNodeUuid);
    stream.WriteString(mTargetNodeName);
    stream.WriteBool(mMuted);
    stream.WriteBool(mLocked);

    stream.WriteUint32((uint32_t)mClips.size());
    for (uint32_t i = 0; i < mClips.size(); ++i)
    {
        stream.WriteUint32((uint32_t)mClips[i]->GetType());
        mClips[i]->SaveStream(stream);
    }
}

void TimelineTrack::LoadStream(Stream& stream, uint32_t version)
{
    mTargetNodeUuid = stream.ReadUint64();
    stream.ReadString(mTargetNodeName);
    mMuted = stream.ReadBool();
    mLocked = stream.ReadBool();

    uint32_t numClips = stream.ReadUint32();
    for (uint32_t i = 0; i < numClips; ++i)
    {
        TypeId clipType = (TypeId)stream.ReadUint32();
        TimelineClip* clip = TimelineClip::CreateInstance(clipType);

        if (clip != nullptr)
        {
            clip->LoadStream(stream, version);
            mClips.push_back(clip);
        }
    }
}

void TimelineTrack::GatherProperties(std::vector<Property>& outProps)
{
    SCOPED_CATEGORY("Track");

    outProps.push_back(Property(DatumType::String, "Target Node", this, &mTargetNodeName));
    outProps.push_back(Property(DatumType::Bool, "Muted", this, &mMuted));
    outProps.push_back(Property(DatumType::Bool, "Locked", this, &mLocked));
}

void TimelineTrack::Evaluate(float time, Node* target, TimelineInstance* inst)
{
}

void TimelineTrack::Reset(Node* target, TimelineInstance* inst)
{
}

glm::vec4 TimelineTrack::GetTrackColor() const
{
    return glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
}

TypeId TimelineTrack::GetDefaultClipType() const
{
    return TimelineClip::GetStaticType();
}

TimelineClip* TimelineTrack::GetClip(uint32_t index) const
{
    if (index < mClips.size())
    {
        return mClips[index];
    }
    return nullptr;
}

void TimelineTrack::AddClip(TimelineClip* clip)
{
    if (clip != nullptr)
    {
        mClips.push_back(clip);
    }
}

void TimelineTrack::RemoveClip(uint32_t index)
{
    if (index < mClips.size())
    {
        delete mClips[index];
        mClips.erase(mClips.begin() + index);
    }
}

void TimelineTrack::InsertClip(TimelineClip* clip, uint32_t index)
{
    if (clip != nullptr)
    {
        if (index <= mClips.size())
        {
            mClips.insert(mClips.begin() + index, clip);
        }
        else
        {
            mClips.push_back(clip);
        }
    }
}

int32_t TimelineTrack::FindClipIndex(TimelineClip* clip) const
{
    for (int32_t i = 0; i < (int32_t)mClips.size(); ++i)
    {
        if (mClips[i] == clip)
        {
            return i;
        }
    }
    return -1;
}
