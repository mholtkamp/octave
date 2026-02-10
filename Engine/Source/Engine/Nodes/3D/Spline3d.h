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
    void GeneratePoint();

protected:
    std::vector<glm::vec3> mPoints;
    NodePtrWeak mAttachmentCamera;
    NodePtrWeak mAttachmentStaticMesh;
    NodePtrWeak mAttachmentSkeletalMesh;
    NodePtrWeak mAttachmentParticle3D;
    NodePtrWeak mAttachmentPointLight;
    NodePtrWeak mAttachmentAudio3D;
    NodePtrWeak mAttachmentNode3D;

    NodePtrWeak mLinkFrom;
    NodePtrWeak mLinkTo;
    NodePtrWeak mLinkFrom2;
    NodePtrWeak mLinkTo2;
    NodePtrWeak mLinkFrom3;
    NodePtrWeak mLinkTo3;
    NodePtrWeak mLinkFrom4;
    NodePtrWeak mLinkTo4;
    NodePtrWeak mLinkFrom5;
    NodePtrWeak mLinkTo5;
    NodePtrWeak mLinkFrom6;
    NodePtrWeak mLinkTo6;
    NodePtrWeak mLinkFrom7;
    NodePtrWeak mLinkTo7;
    NodePtrWeak mLinkFrom8;
    NodePtrWeak mLinkTo8;
    NodePtrWeak mLinkFrom9;
    NodePtrWeak mLinkTo9;
    NodePtrWeak mLinkFrom10;
    NodePtrWeak mLinkTo10;
    NodePtrWeak mLinkFrom11;
    NodePtrWeak mLinkTo11;
    bool mFollowLink1 = true;
    bool mFollowLink2 = true;
    bool mFollowLink3 = true;
    bool mFollowLink4 = true;
    bool mFollowLink5 = true;
    bool mFollowLink6 = true;
    bool mFollowLink7 = true;
    bool mFollowLink8 = true;
    bool mFollowLink9 = true;
    bool mFollowLink10 = true;
    bool mFollowLink11 = true;
    bool mLinkTriggered1 = false;
    bool mLinkTriggered2 = false;
    bool mLinkTriggered3 = false;
    bool mLinkTriggered4 = false;
    bool mLinkTriggered5 = false;
    bool mLinkTriggered6 = false;
    bool mLinkTriggered7 = false;
    bool mLinkTriggered8 = false;
    bool mLinkTriggered9 = false;
    bool mLinkTriggered10 = false;
    bool mLinkTriggered11 = false;
    bool mLinkActive = false;
    bool mDisableBounce = false;
    float mLinkSpeedModifier = 1.0f;
    float mLinkSpeedModifier1 = 1.0f;
    float mLinkSpeedModifier2 = 1.0f;
    float mLinkSpeedModifier3 = 1.0f;
    float mLinkSpeedModifier4 = 1.0f;
    float mLinkSpeedModifier5 = 1.0f;
    float mLinkSpeedModifier6 = 1.0f;
    float mLinkSpeedModifier7 = 1.0f;
    float mLinkSpeedModifier8 = 1.0f;
    float mLinkSpeedModifier9 = 1.0f;
    float mLinkSpeedModifier10 = 1.0f;
    float mLinkSpeedModifier11 = 1.0f;
    bool mEnableLink11 = false;

    int32_t mGeneratedLinkCount = 10;
    NodePtrWeak mLinkFromExtra[65];
    NodePtrWeak mLinkToExtra[65];
    bool mFollowLinkExtra[65] = {};
    bool mLinkTriggeredExtra[65] = {};
    float mLinkSpeedModifierExtra[65] = {};

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
    };

    std::vector<PointSpeedEntry> mPointSpeedEntries;
    NodePtrWeak mPointSpeedTarget;
    float mPointSpeedValue = 1.0f;
};
