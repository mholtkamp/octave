#pragma once

#include "Timeline/TimelineTrack.h"

class AnimationTrack : public TimelineTrack
{
public:

    DECLARE_TRACK(AnimationTrack, TimelineTrack);

    AnimationTrack();
    virtual ~AnimationTrack();

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Animation"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;
};
