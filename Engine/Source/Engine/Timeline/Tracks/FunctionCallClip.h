#pragma once

#include "Timeline/TimelineClip.h"

#include <vector>
#include <string>

struct FunctionCallKeyframe
{
    float mTime = 0.0f;
    std::string mFunctionName;
};

class FunctionCallClip : public TimelineClip
{
public:
    DECLARE_CLIP(FunctionCallClip, TimelineClip);

    FunctionCallClip();
    virtual ~FunctionCallClip();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual bool SupportsKeyframes() const override { return true; }
    virtual uint32_t GetNumKeyframes() const override { return (uint32_t)mKeyframes.size(); }
    virtual float GetKeyframeTime(uint32_t index) const override { return mKeyframes[index].mTime; }
    virtual void SetKeyframeTime(uint32_t index, float time) override;
    virtual void AddKeyframeAtTime(float localTime, Node* targetNode) override;
    virtual void RemoveKeyframe(uint32_t index) override;

    const std::string& GetKeyframeFunctionName(uint32_t index) const { return mKeyframes[index].mFunctionName; }
    void SetKeyframeFunctionName(uint32_t index, const std::string& name);

    // Fire-once tracking (reset by FunctionCallTrack::Reset)
    int32_t mLastFiredKeyframe = -1;

protected:
    std::vector<FunctionCallKeyframe> mKeyframes;
};
