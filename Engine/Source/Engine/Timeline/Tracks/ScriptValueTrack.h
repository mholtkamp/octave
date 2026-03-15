#pragma once

#include "Timeline/TimelineTrack.h"

#include <string>

class ScriptValueTrack : public TimelineTrack
{
public:

    DECLARE_TRACK(ScriptValueTrack, TimelineTrack);

    ScriptValueTrack();
    virtual ~ScriptValueTrack();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Evaluate(float time, Node* target, TimelineInstance* inst) override;
    virtual void Reset(Node* target, TimelineInstance* inst) override;

    virtual const char* GetTrackTypeName() const override { return "Script Value"; }
    virtual glm::vec4 GetTrackColor() const override;
    virtual TypeId GetDefaultClipType() const override;

    const std::string& GetPropertyName() const { return mPropertyName; }
    void SetPropertyName(const std::string& name) { mPropertyName = name; }

protected:

    std::string mPropertyName;
};
