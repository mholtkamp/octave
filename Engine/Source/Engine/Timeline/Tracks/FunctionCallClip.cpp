#include "Timeline/Tracks/FunctionCallClip.h"
#include "Utilities.h"

FORCE_LINK_DEF(FunctionCallClip);
DEFINE_CLIP(FunctionCallClip);

FunctionCallClip::FunctionCallClip()
{
}

FunctionCallClip::~FunctionCallClip()
{
}

void FunctionCallClip::SaveStream(Stream& stream)
{
    TimelineClip::SaveStream(stream);

    stream.WriteUint32((uint32_t)mKeyframes.size());
    for (uint32_t i = 0; i < mKeyframes.size(); ++i)
    {
        stream.WriteFloat(mKeyframes[i].mTime);
        stream.WriteString(mKeyframes[i].mFunctionName);
    }
}

void FunctionCallClip::LoadStream(Stream& stream, uint32_t version)
{
    TimelineClip::LoadStream(stream, version);

    uint32_t numKeyframes = stream.ReadUint32();
    mKeyframes.resize(numKeyframes);
    for (uint32_t i = 0; i < numKeyframes; ++i)
    {
        mKeyframes[i].mTime = stream.ReadFloat();
        stream.ReadString(mKeyframes[i].mFunctionName);
    }
}

void FunctionCallClip::GatherProperties(std::vector<Property>& outProps)
{
    TimelineClip::GatherProperties(outProps);

#if EDITOR
    SCOPED_CATEGORY("Keyframes");

    uint32_t startIdx = 0;
    uint32_t endIdx = 0;
    GetKeyframeDisplayRange(startIdx, endIdx);

    for (uint32_t i = startIdx; i < endIdx; ++i)
    {
        char timeName[32];
        snprintf(timeName, sizeof(timeName), "KF %d Time", i);
        outProps.push_back(Property(DatumType::Float, timeName, this, &mKeyframes[i].mTime));

        char funcName[32];
        snprintf(funcName, sizeof(funcName), "KF %d Func", i);
        outProps.push_back(Property(DatumType::String, funcName, this, &mKeyframes[i].mFunctionName));
    }
#endif
}

void FunctionCallClip::SetKeyframeFunctionName(uint32_t index, const std::string& name)
{
    if (index < mKeyframes.size())
    {
        mKeyframes[index].mFunctionName = name;
    }
}

void FunctionCallClip::SetKeyframeTime(uint32_t index, float time)
{
    if (index < mKeyframes.size())
    {
        mKeyframes[index].mTime = time;
    }
}

void FunctionCallClip::AddKeyframeAtTime(float localTime, Node* targetNode)
{
    FunctionCallKeyframe kf;
    kf.mTime = localTime;

    // Insert sorted by time
    bool inserted = false;
    for (uint32_t i = 0; i < mKeyframes.size(); ++i)
    {
        if (kf.mTime < mKeyframes[i].mTime)
        {
            mKeyframes.insert(mKeyframes.begin() + i, kf);
            inserted = true;
            break;
        }
    }

    if (!inserted)
    {
        mKeyframes.push_back(kf);
    }
}

void FunctionCallClip::RemoveKeyframe(uint32_t index)
{
    if (index < mKeyframes.size())
    {
        mKeyframes.erase(mKeyframes.begin() + index);
    }
}
