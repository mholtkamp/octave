#pragma once

#include "Maths.h"

class CameraFrustum
{
public:

    glm::vec3 mPosition = {};
    glm::vec3 mBasisX = { 1.0f, 0.0f, 0.0f };
    glm::vec3 mBasisY = { 0.0f, 1.0f, 0.0f };
    glm::vec3 mBasisZ = { 0.0f, 0.0f, -1.0f };

    float mNearDist = 0.0f;
    float mFarDist = 0.0f;
    float mNearWidth = 0.0f;
    float mNearHeight = 0.0f;
    float mTangent = 0.0f;
    float mAspectRatio = 0.0f;

    float mSphereFactorX = 0.0f;
    float mSphereFactorY = 0.0f;

    bool mOrtho = false;

public:

    void SetPerspective(float angle, float ratio, float nearDist, float farDist);
    void SetOrthographic(float width, float height, float nearDist, float farDist);
    void SetPosition(glm::vec3 position);
    void SetBasis(glm::vec3 forward, glm::vec3 up, glm::vec3 right);

    bool IsPointInFrustum(glm::vec3 p) const;
    bool IsSphereInFrustum(glm::vec3 center, float radius) const;

    bool IsPointInFrustumOrtho(glm::vec3 p) const;
    bool IsSphereInFrustumOrtho(glm::vec3 center, float radius) const;
};
