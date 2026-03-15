#pragma once

#include "Timeline/TimelineClip.h"
#include "Datum.h"

#include <vector>

struct ScriptValueKeyframe
{
    float mTime = 0.0f;
    Datum mValue;
    InterpMode mInterpMode = InterpMode::Linear;
};

class ScriptValueClip : public TimelineClip
{
public:

    DECLARE_CLIP(ScriptValueClip, TimelineClip);

    ScriptValueClip();
    virtual ~ScriptValueClip();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual bool SupportsKeyframes() const override { return true; }
    virtual uint32_t GetNumKeyframes() const override { return (uint32_t)mKeyframes.size(); }
    virtual float GetKeyframeTime(uint32_t index) const override { return mKeyframes[index].mTime; }
    virtual void AddKeyframeAtTime(float localTime, Node* targetNode) override;
    virtual void SetKeyframeTime(uint32_t index, float time) override;
    virtual void RemoveKeyframe(uint32_t index) override;

    const ScriptValueKeyframe& GetKeyframe(uint32_t index) const { return mKeyframes[index]; }
    void AddKeyframe(const ScriptValueKeyframe& kf);

    Datum EvaluateAtLocalTime(float localTime) const;

protected:

    std::vector<ScriptValueKeyframe> mKeyframes;
};
