#pragma once

#include "Timeline/TimelineClip.h"
#include "Maths.h"

#include <vector>

struct TransformKeyframe
{
    float mTime = 0.0f;
    glm::vec3 mPosition = glm::vec3(0.0f);
    glm::quat mRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 mScale = glm::vec3(1.0f);
    InterpMode mInterpMode = InterpMode::Linear;
};

class TransformClip : public TimelineClip
{
public:

    DECLARE_CLIP(TransformClip, TimelineClip);

    TransformClip();
    virtual ~TransformClip();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual bool SupportsKeyframes() const override { return true; }
    virtual uint32_t GetNumKeyframes() const override { return (uint32_t)mKeyframes.size(); }
    virtual float GetKeyframeTime(uint32_t index) const override { return mKeyframes[index].mTime; }
    virtual void AddKeyframeAtTime(float localTime, Node* targetNode) override;
    virtual void SetKeyframeTime(uint32_t index, float time) override;
    virtual void RemoveKeyframe(uint32_t index) override;

    const TransformKeyframe& GetKeyframe(uint32_t index) const { return mKeyframes[index]; }
    void AddKeyframe(const TransformKeyframe& kf);
    void SetKeyframe(uint32_t index, const TransformKeyframe& kf);

    void EvaluateAtLocalTime(float localTime, glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale) const;

protected:

    std::vector<TransformKeyframe> mKeyframes;

#if EDITOR
    mutable std::vector<glm::vec3> mEditorRotationEuler;
#endif
};
