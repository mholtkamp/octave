#pragma once

#include "TransformComponent.h"
#include "Enums.h"
#include "CameraFrustum.h"

#include <glm/glm.hpp>

struct OrthoSettings
{
    float mWidth;
    float mHeight;
    float mNear;
    float mFar;

    OrthoSettings() :
        mWidth(12.8f),
        mHeight(7.2f),
        mNear(0.0f),
        mFar(100.0f)
    {

    }
};

struct PerspectiveSettings
{
    float mFovY;
    float mAspectRatio;
    float mNear;
    float mFar;

    PerspectiveSettings() :
        mFovY(70.0f),
        mAspectRatio(1.78f),
        mNear(1.0f),
        mFar(4096.0f)
    {

    }
};

class CameraComponent : public TransformComponent
{
public:

    DECLARE_COMPONENT(CameraComponent, TransformComponent);

    CameraComponent();
    ~CameraComponent();

    virtual const char* GetTypeName() const override;
    virtual void Destroy() override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    void SetOrthoSettings(float width,
        float height,
        float zNear,
        float zFar);

    void SetPerspectiveSettings(float fovY,
        float aspectRatio,
        float zNear,
        float zFar);

    ProjectionMode GetProjectionMode() const;
    void SetProjectionMode(ProjectionMode mode);

    PerspectiveSettings GetPerspectiveSettings() const;
    OrthoSettings GetOrthoSettings() const;

    const glm::mat4& GetViewProjectionMatrix();
    const glm::mat4& GetViewMatrix();
    const glm::mat4& GetProjectionMatrix();

    void ComputeMatrices();

    glm::mat4 CalculateViewMatrix();
    glm::mat4 CalculateInvViewMatrix();

    float GetNearZ() const;
    float GetFarZ() const;
    float GetFieldOfView() const;
    float GetFieldOfViewY() const;
    float GetFieldOfViewX() const;
    float GetAspectRatio() const;
    float GetWidth() const;
    float GetHeight() const;

    void SetNearZ(float nearZ);
    void SetFarZ(float farZ);
    void SetFieldOfView(float fovY);
    void SetAspectRatio(float aspectRatio);
    void SetWidth(float width);
    void SetHeight(float height);

    glm::vec3 WorldToScreenPosition(glm::vec3 worldPos);

protected:

    ProjectionMode mProjectionMode;
    glm::mat4 mViewProjectionMatrix;
    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;
    glm::mat4 mStandardViewProjectionMatrix;

    OrthoSettings mOrthoSettings;
    PerspectiveSettings mPerspectiveSettings;
};