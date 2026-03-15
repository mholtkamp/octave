#pragma once

#include "Timeline/TimelineTrack.h"

class TransformTrack : public TimelineTrack
{
public:

    DECLARE_TRACK(TransformTrack, TimelineTrack);

    TransformTrack();
    virtual ~TransformTrack();

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Transform"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;
};
