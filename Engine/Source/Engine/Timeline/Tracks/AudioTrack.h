#pragma once

#include "Timeline/TimelineTrack.h"

class AudioTrack : public TimelineTrack
{
public:

    DECLARE_TRACK(AudioTrack, TimelineTrack);

    AudioTrack();
    virtual ~AudioTrack();

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Audio"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;
};
