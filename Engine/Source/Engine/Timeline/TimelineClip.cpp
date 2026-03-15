#include "Timeline/TimelineClip.h"
#include "Utilities.h"

#if EDITOR
#include "EditorState.h"
#endif

FORCE_LINK_DEF(TimelineClip);
DEFINE_FACTORY_MANAGER(TimelineClip);
DEFINE_FACTORY(TimelineClip, TimelineClip);
DEFINE_OBJECT(TimelineClip);

TimelineClip::TimelineClip()
{
}

TimelineClip::~TimelineClip()
{
}

void TimelineClip::SaveStream(Stream& stream)
{
    stream.WriteFloat(mStartTime);
    stream.WriteFloat(mDuration);
    stream.WriteFloat(mClipInTime);
    stream.WriteFloat(mClipOutTime);
    stream.WriteFloat(mSpeed);
}

void TimelineClip::LoadStream(Stream& stream, uint32_t version)
{
    mStartTime = stream.ReadFloat();
    mDuration = stream.ReadFloat();
    mClipInTime = stream.ReadFloat();
    mClipOutTime = stream.ReadFloat();
    mSpeed = stream.ReadFloat();
}

void TimelineClip::GatherProperties(std::vector<Property>& outProps)
{
    SCOPED_CATEGORY("Clip");

    outProps.push_back(Property(DatumType::Float, "Start Time", this, &mStartTime));
    outProps.push_back(Property(DatumType::Float, "Duration", this, &mDuration));
    outProps.push_back(Property(DatumType::Float, "Clip In", this, &mClipInTime));
    outProps.push_back(Property(DatumType::Float, "Clip Out", this, &mClipOutTime));
    outProps.push_back(Property(DatumType::Float, "Speed", this, &mSpeed));
}

bool TimelineClip::ContainsTime(float globalTime) const
{
    return globalTime >= mStartTime && globalTime < (mStartTime + mDuration);
}

float TimelineClip::GetLocalTime(float globalTime) const
{
    float localTime = (globalTime - mStartTime) * mSpeed;
    localTime += mClipInTime;
    return localTime;
}

#if EDITOR
void TimelineClip::GetKeyframeDisplayRange(uint32_t& outStart, uint32_t& outEnd) const
{
    outStart = 0;
    outEnd = GetNumKeyframes();

    int32_t selKf = GetEditorState()->mTimelineSelectedKeyframe;
    if (selKf >= 0 && (uint32_t)selKf < outEnd)
    {
        outStart = (uint32_t)selKf;
        outEnd = outStart + 1;
    }
}
#endif

bool TimelineClip::OverlapsWith(const TimelineClip* other) const
{
    if (other == nullptr)
        return false;

    return mStartTime < other->GetEndTime() && GetEndTime() > other->GetStartTime();
}
