#pragma once

#include "Timeline/TimelineTrack.h"

class FunctionCallTrack : public TimelineTrack
{
public:
    DECLARE_TRACK(FunctionCallTrack, TimelineTrack);

    FunctionCallTrack();
    virtual ~FunctionCallTrack();

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Func Call"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;
};
