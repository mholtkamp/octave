#pragma once

#include "TransformComponent.h"
#include "Enums.h"
#include "CameraFrustum.h"

#include "Maths.h"

class Primitive3D;

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

class Camera3D : public Node3D
{
public:

    DECLARE_NODE(Camera3D, Node3D);

    Camera3D();
    ~Camera3D();

    virtual const char* GetTypeName() const override;
    virtual void BeginPlay() override;
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

    float GetNearWidth() const;
    float GetNearHeight() const;

    void SetNearZ(float nearZ);
    void SetFarZ(float farZ);
    void SetFieldOfView(float fovY);
    void SetAspectRatio(float aspectRatio);
    void SetWidth(float width);
    void SetHeight(float height);

    glm::vec3 WorldToScreenPosition(glm::vec3 worldPos);
    glm::vec3 ScreenToWorldPosition(int32_t x, int32_t y);
    glm::vec3 TraceScreenToWorld(int32_t x, int32_t y, uint8_t colMask, Primitive3D** outComp = nullptr);

protected:

    ProjectionMode mProjectionMode;
    glm::mat4 mViewProjectionMatrix;
    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;
    glm::mat4 mStandardViewProjectionMatrix;

    OrthoSettings mOrthoSettings;
    PerspectiveSettings mPerspectiveSettings;
};