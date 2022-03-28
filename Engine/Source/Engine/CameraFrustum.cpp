#include "CameraFrustum.h"
#include "Maths.h"

// This camera frustum culling code was taken from:
// http://www.lighthouse3d.com/tutorials/view-frustum-culling/ 
// The original algorithm was introduced in Game Programming Gems 5 (radar culling).

void CameraFrustum::SetPerspective(float angle, float ratio, float nearDist, float farDist)
{
    mAspectRatio = ratio;
    mNearDist = nearDist;
    mFarDist = farDist;

    float halfAngle = DEGREES_TO_RADIANS * angle * 0.5f;
    mTangent = tanf(halfAngle);

    float cosHalfAngle = cosf(halfAngle);
    mSphereFactorY = (cosHalfAngle != 0.0f) ? (1.0f / cosHalfAngle) : 0.0f;

    //float halfAngleX = atanf(mTangent) * ratio;
    float halfAngleX = atanf(mTangent * ratio);
    float cosHalfAngleX = cosf(halfAngleX);
    mSphereFactorX = (cosHalfAngleX != 0.0f) ? (1.0f / cosHalfAngleX) : 0.0f;

    mNearHeight = nearDist * mTangent;
    mNearWidth = mNearHeight * ratio;

    mOrtho = false;
}

void CameraFrustum::SetOrthographic(float width, float height, float nearDist, float farDist)
{
    mNearWidth = width;
    mNearHeight = height;
    mNearDist = nearDist;
    mFarDist = farDist;

    mTangent = 1.0f;
    mAspectRatio = 1.0f;

    mOrtho = true;
}

void CameraFrustum::SetPosition(glm::vec3 position)
{
    mPosition = position;
}

void CameraFrustum::SetBasis(glm::vec3 forward, glm::vec3 up, glm::vec3 right)
{
    mBasisX = right;
    mBasisY = up;
    mBasisZ = forward;
}

bool CameraFrustum::IsPointInFrustum(glm::vec3 p) const
{
    float pcz = 0.0f;
    float pcx = 0.0f;
    float pcy = 0.0f;
    float aux = 0.0f;

    glm::vec3 v = p - mPosition;

    // Z axis
    pcz = glm::dot(v, mBasisZ);
    if (pcz > mFarDist || pcz < mNearDist)
    {
        return false;
    }

    // Y axis
    pcy = glm::dot(v, mBasisY);
    aux = pcz * mTangent;
    if (pcy > aux || pcy < -aux)
    {
        return false;
    }

    // X axis
    pcx = glm::dot(v, mBasisX);
    aux = aux * mAspectRatio;
    if (pcx > aux || pcx < -aux)
    {
        return false;
    }

    return true;
}

bool CameraFrustum::IsSphereInFrustum(glm::vec3 center, float radius) const
{
    glm::vec3 v = center - mPosition;

    float az = glm::dot(v, mBasisZ);
    if (az > mFarDist + radius || az < mNearDist - radius)
        return false;

    float ay = glm::dot(v, mBasisY);
    float d = mSphereFactorY * radius;
    float vert = az * mTangent;

    if (ay > vert + d || ay < -vert - d)
        return false;

    float ax = glm::dot(v, mBasisX);
    float hori = vert * mAspectRatio;
    d = mSphereFactorX * radius;

    if (ax > hori + d || ax < -hori - d)
        return false;

    return true;
}

bool CameraFrustum::IsPointInFrustumOrtho(glm::vec3 p) const
{
    glm::vec3 v = p - mPosition;

    float az = glm::dot(v, mBasisZ);
    if (az > mFarDist || az < mNearDist)
        return false;

    float ay = glm::dot(v, mBasisY);
    if (ay > mNearHeight || ay < -mNearHeight)
        return false;

    float ax = glm::dot(v, mBasisX);
    if (ax > mNearWidth || ax < -mNearWidth)
        return false;

    return true;
}

bool CameraFrustum::IsSphereInFrustumOrtho(glm::vec3 center, float radius) const
{
    glm::vec3 v = center - mPosition;

    float az = glm::dot(v, mBasisZ);
    if (az > mFarDist + radius || az < mNearDist - radius)
        return false;

    float ay = glm::dot(v, mBasisY);
    if (ay > mNearHeight + radius || ay < -mNearHeight - radius)
        return false;

    float ax = glm::dot(v, mBasisX);
    if (ax > mNearWidth + radius || ax < -mNearWidth - radius)
        return false;

    return true;
}
