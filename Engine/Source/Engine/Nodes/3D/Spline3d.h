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

    glm::mat4 mOrigCamTransform = glm::mat4(1.0f);
    glm::mat4 mOrigStaticTransform = glm::mat4(1.0f);
    glm::mat4 mOrigSkeletalTransform = glm::mat4(1.0f);
    glm::mat4 mOrigParticleTransform = glm::mat4(1.0f);
    glm::mat4 mOrigPointLightTransform = glm::mat4(1.0f);
    glm::mat4 mOrigAudioTransform = glm::mat4(1.0f);

    float mSpeed = 2.0f;
    bool mPlaying = false;
    bool mLoop = true;
    bool mCloseLoop = false;
    bool mSmoothCurve = true;
    bool mFaceTangent = false;
    float mTravel = 0.0f;

    struct PointSpeedEntry
    {
        std::string name;
        float speed = 1.0f;
    };

    std::vector<PointSpeedEntry> mPointSpeedEntries;
    NodePtrWeak mPointSpeedTarget;
    float mPointSpeedValue = 1.0f;
};
