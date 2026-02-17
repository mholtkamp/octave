#pragma once

#include "Timeline/TimelineTrack.h"

class ActivateTrack : public TimelineTrack
{
public:

    DECLARE_TRACK(ActivateTrack, TimelineTrack);

    ActivateTrack();
    virtual ~ActivateTrack();

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Activate"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;
};
