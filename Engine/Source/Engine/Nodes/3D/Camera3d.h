#pragma once

#include "Node3d.h"
#include "Enums.h"
#include "CameraFrustum.h"

#include "Maths.h"

class Primitive3D;

class Camera3D : public Node3D
{
public:

    DECLARE_NODE(Camera3D, Node3D);

    Camera3D();
    ~Camera3D();

    virtual const char* GetTypeName() const override;
    virtual void Start() override;
    virtual void Destroy() override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    ProjectionMode GetProjectionMode() const;
    void SetProjectionMode(ProjectionMode mode);
    void EnablePerspective(bool perspective);
    bool IsPerspectiveEnabled() const;

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
    float GetOrthoWidth() const;
    float GetOrthoHeight() const;
    float GetAspectRatio() const;

    float GetNearWidth() const;
    float GetNearHeight() const;

    void SetNearZ(float nearZ);
    void SetFarZ(float farZ);
    void SetFieldOfView(float fovY);
    void SetOrthoWidth(float width);

    glm::vec3 WorldToScreenPosition(glm::vec3 worldPos);
    glm::vec3 ScreenToWorldPosition(int32_t x, int32_t y);
    glm::vec3 TraceScreenToWorld(int32_t x, int32_t y, uint8_t colMask, RayTestResult& rayResult);

    const bool IsEditorCamera();

protected:

    ProjectionMode mProjectionMode;
    glm::mat4 mViewProjectionMatrix;
    glm::mat4 mViewMatrix;
    glm::mat4 mProjectionMatrix;
    glm::mat4 mStandardViewProjectionMatrix;

    float mNear = 0.25f;
    float mFar = 4096.0f;
    float mFovY = 70.0f;
    float mOrthoWidth = 12.8f;

    float mAspectRatio = 16.0f / 9.0f;
};