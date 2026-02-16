#pragma once

#include "Nodes/3D/Node3d.h"
#include <vector>

class Spline3D : public Node3D
{
public:
    DECLARE_NODE(Spline3D, Node3D);

    virtual void Create() override;
    virtual void Start() override;
    virtual void Stop() override;
    virtual void Tick(float deltaTime) override;
    virtual void Copy(Node* srcNode, bool recurse) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void LoadStream(Stream& stream, Platform platform, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& props) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    void AddPoint(const glm::vec3& p);
    void ClearPoints();
    uint32_t GetPointCount() const;
    glm::vec3 GetPoint(uint32_t index) const;
    void SetPoint(uint32_t index, const glm::vec3& p);

    glm::vec3 GetPositionAt(float t) const;  // t in [0,1]
    glm::vec3 GetTangentAt(float t) const;   // normalized tangent

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    static glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);
    static glm::vec3 CatmullRomTangent(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t);

    void Play();
    void StopPlayback();
    void SetPaused(bool paused);
    bool IsPaused() const;
    void SetFollowLinkEnabled(uint32_t index, bool enabled);
    bool IsFollowLinkEnabled(uint32_t index) const;
    bool IsNearLinkFrom(uint32_t index, float epsilon = 0.05f) const;
    bool IsNearLinkTo(uint32_t index, float epsilon = 0.05f) const;
    bool IsLinkDirectionForward(uint32_t index, float threshold = 0.0f) const;
    bool TriggerLink(uint32_t index);
    void CancelActiveLink();

    static void SetSplineLinesVisible(bool visible);
    static bool IsSplineLinesVisible();

#if EDITOR
    virtual bool DrawCustomProperty(Property& prop) override;
#endif

protected:
    struct SplineLink;
    void GeneratePoint();
    SplineLink* GetLinkByIndex(uint32_t index);
    const SplineLink* GetLinkByIndex(uint32_t index) const;
    void EnsureLinkSlots(uint32_t count);

protected:
    std::vector<glm::vec3> mPoints;
    NodePtrWeak mAttachmentCamera;
    NodePtrWeak mAttachmentStaticMesh;
    NodePtrWeak mAttachmentSkeletalMesh;
    NodePtrWeak mAttachmentParticle3D;
    NodePtrWeak mAttachmentPointLight;
    NodePtrWeak mAttachmentAudio3D;
    NodePtrWeak mAttachmentNode3D;

    struct SplineLink
    {
        NodePtrWeak mLinkFrom;
        NodePtrWeak mLinkTo;
        bool mFollow = true;
        bool mTriggered = false;
        float mSpeed = 1.0f;
    };

    std::vector<SplineLink> mLinks;
    bool mLinkActive = false;
    bool mDisableBounce = false;
    float mLinkSpeedModifier = 1.0f;
    int32_t mGeneratedLinkCount = 10;

    float mActiveLinkSpeedModifier = 1.0f;
    bool mLinkSmoothStep = false;
    bool mLinkSmoothRotate = false;
    float mLinkTravel = 0.0f;
    float mLinkLen = 0.0f;
    glm::vec3 mLinkStart = glm::vec3(0.0f);
    glm::vec3 mLinkEnd = glm::vec3(0.0f);
    NodePtrWeak mLinkTargetSpline;
    float mLinkTargetStartDist = 0.0f;
    float mLinkTargetPrevDist = 0.0f;
    float mLinkTargetTotalLen = 0.0f;

    glm::mat4 mOrigCamTransform = glm::mat4(1.0f);
    glm::mat4 mOrigStaticTransform = glm::mat4(1.0f);
    glm::mat4 mOrigSkeletalTransform = glm::mat4(1.0f);
    glm::mat4 mOrigParticleTransform = glm::mat4(1.0f);
    glm::mat4 mOrigPointLightTransform = glm::mat4(1.0f);
    glm::mat4 mOrigAudioTransform = glm::mat4(1.0f);
    glm::mat4 mOrigNodeTransform = glm::mat4(1.0f);

    float mSpeed = 2.0f;
    bool mPlaying = false;
    bool mLoop = true;
    bool mCloseLoop = false;
    bool mSmoothCurve = true;
    bool mSmoothRotate = false;
    bool mPause = false;
    bool mFaceTangent = false;
    bool mReverseFaceTangent = false;
    float mTravel = 0.0f;

    bool mHasTrackedPos = false;
    glm::vec3 mPrevTrackedPos = glm::vec3(0.0f);
    glm::vec3 mTrackedMoveDir = glm::vec3(0.0f);

    struct PointSpeedEntry
    {
        std::string name;
        float speed = 1.0f;
        bool smoothIn = false;
        bool smoothOut = false;
        bool smoothCurve = false;
    };

    std::vector<PointSpeedEntry> mPointSpeedEntries;
    NodePtrWeak mPointSpeedTarget;
    float mPointSpeedValue = 1.0f;
    bool mPointSmoothInValue = false;
    bool mPointSmoothOutValue = false;
    bool mPointSmoothCurveValue = false;
};
