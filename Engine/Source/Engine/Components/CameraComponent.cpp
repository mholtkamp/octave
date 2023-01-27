#include "Components/CameraComponent.h"
#include "Log.h"
#include "Engine.h"
#include "World.h"
#include "Renderer.h"
#include "AssetManager.h"

#include "Graphics/Graphics.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

FORCE_LINK_DEF(CameraComponent);
DEFINE_COMPONENT(CameraComponent);

CameraComponent::CameraComponent() :
    mProjectionMode(ProjectionMode::PERSPECTIVE),
    mViewProjectionMatrix(1)
{
    mName = "Camera";
}

CameraComponent::~CameraComponent()
{

}

const char* CameraComponent::GetTypeName() const
{
    return "Camera";
}

void CameraComponent::Destroy()
{
    if (GetWorld() &&
        GetWorld()->GetActiveCamera() == this)
    {
        GetWorld()->SetActiveCamera(nullptr);
    }

    Component::Destroy();
}

void CameraComponent::GatherProperties(std::vector<Property>& outProps)
{
    TransformComponent::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Bool, "Perspective", this, &mProjectionMode));
    outProps.push_back(Property(DatumType::Float, "Field Of View", this, &mPerspectiveSettings.mFovY));
}

void CameraComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    if (GetType() == CameraComponent::GetStaticType() &&
        this != GetWorld()->GetActiveCamera())
    {
        glm::mat4 transform = glm::rotate(DEGREES_TO_RADIANS * -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = mTransform * transform;

        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cone");
        debugDraw.mActor = GetOwner();
        debugDraw.mComponent = this;
        debugDraw.mColor = glm::vec4(0.0f, 0.85f, 0.25f, 1.0f);
        debugDraw.mTransform = glm::scale(transform, { 0.5f, 0.5f, 0.5f });
        inoutDraws.push_back(debugDraw);
    }
#endif
}

void CameraComponent::SaveStream(Stream& stream)
{
    TransformComponent::SaveStream(stream);
    stream.WriteUint8(uint8_t(mProjectionMode));
    stream.WriteFloat(mPerspectiveSettings.mFovY);
}

void CameraComponent::LoadStream(Stream& stream)
{
    TransformComponent::LoadStream(stream);
    mProjectionMode = ProjectionMode(stream.ReadUint8());
    mPerspectiveSettings.mFovY = stream.ReadFloat();
}

void CameraComponent::SetOrthoSettings(float width, float height, float zNear, float zFar)
{
    mOrthoSettings.mWidth = width;
    mOrthoSettings.mHeight = height;
    mOrthoSettings.mNear = zNear;
    mOrthoSettings.mFar = zFar;
}

void CameraComponent::SetPerspectiveSettings(float fovY, float aspectRatio, float zNear, float zFar)
{
    mPerspectiveSettings.mFovY = fovY;
    mPerspectiveSettings.mAspectRatio = aspectRatio;
    mPerspectiveSettings.mNear = zNear;
    mPerspectiveSettings.mFar = zFar;
}

ProjectionMode CameraComponent::GetProjectionMode() const
{
    return mProjectionMode;
}

void CameraComponent::SetProjectionMode(ProjectionMode mode)
{
    mProjectionMode = mode;
}

PerspectiveSettings CameraComponent::GetPerspectiveSettings() const
{
    return mPerspectiveSettings;
}

OrthoSettings CameraComponent::GetOrthoSettings() const
{
    return mOrthoSettings;
}

const glm::mat4& CameraComponent::GetViewProjectionMatrix()
{
    return mViewProjectionMatrix;
}

const glm::mat4& CameraComponent::GetViewMatrix()
{
    return mViewMatrix;
}

const glm::mat4& CameraComponent::GetProjectionMatrix()
{
    return mProjectionMatrix;
}

void CameraComponent::ComputeMatrices()
{
    // Make sure transform is up to date.
    UpdateTransform(false);

    EngineState* engineState = GetEngineState();
    mPerspectiveSettings.mAspectRatio = static_cast<float>(engineState->mWindowWidth) / engineState->mWindowHeight;

    mViewMatrix = CalculateViewMatrix();
    mViewMatrix = glm::toMat4(glm::conjugate(GetAbsoluteRotationQuat()));

    mViewMatrix = translate(mViewMatrix, -GetAbsolutePosition());

    if (mProjectionMode == ProjectionMode::ORTHOGRAPHIC)
    {
        mProjectionMatrix = GFX_MakeOrthographicMatrix(
            -mOrthoSettings.mWidth,
            mOrthoSettings.mWidth,
            -mOrthoSettings.mHeight,
            mOrthoSettings.mHeight,
            mOrthoSettings.mNear,
            mOrthoSettings.mFar);

        // The perspective clip matrix was causing geometry behind the camera to render.
        // For ortho, I don't think there's a need to convert Z to 0-1, I think it's already in that range.
        const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        mViewProjectionMatrix = clip * mProjectionMatrix * mViewMatrix;

        glm::mat4 stdProjMtx = glm::ortho(
            -mOrthoSettings.mWidth,
            mOrthoSettings.mWidth,
            -mOrthoSettings.mHeight,
            mOrthoSettings.mHeight,
            mOrthoSettings.mNear,
            mOrthoSettings.mFar);

        mStandardViewProjectionMatrix = clip * stdProjMtx * mViewMatrix;
    }
    else
    {
        mProjectionMatrix = GFX_MakePerspectiveMatrix(mPerspectiveSettings.mFovY,
            mPerspectiveSettings.mAspectRatio,
            mPerspectiveSettings.mNear,
            mPerspectiveSettings.mFar);

        // Needed for adjusting to NDC
        const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            0.0f, 0.0f, 0.5f, 1.0f);

        mViewProjectionMatrix = clip * mProjectionMatrix * mViewMatrix;

        // Because the 3DS projection matrix is wonky and I don't know how to 
        // derive the clipspace position for World-To-Screen conversions,
        // just create a standard perspective matrix and we can use it for the conversions.
        glm::mat4 stdProjMtx = glm::perspectiveFov(
            glm::radians(mPerspectiveSettings.mFovY),
            mPerspectiveSettings.mAspectRatio,
            1.0f,
            mPerspectiveSettings.mNear,
            mPerspectiveSettings.mFar);

        mStandardViewProjectionMatrix = clip * stdProjMtx * mViewMatrix;
    }
}

glm::mat4 CameraComponent::CalculateViewMatrix()
{
    glm::mat4 view = glm::toMat4(glm::conjugate(GetAbsoluteRotationQuat()));
    view = translate(mViewMatrix, -GetAbsolutePosition());
    return view;
}

glm::mat4 CameraComponent::CalculateInvViewMatrix()
{
    glm::mat4 invView;

    invView = CalculateViewMatrix();
    invView = glm::inverse(invView);

    return invView;
}

float CameraComponent::GetNearZ() const
{
    return (mProjectionMode == ProjectionMode::ORTHOGRAPHIC) ?
        mOrthoSettings.mNear :
        mPerspectiveSettings.mNear;
}

float CameraComponent::GetFarZ() const
{
    return (mProjectionMode == ProjectionMode::ORTHOGRAPHIC) ?
        mOrthoSettings.mFar :
        mPerspectiveSettings.mFar;
}

float CameraComponent::GetFieldOfView() const
{
    return mPerspectiveSettings.mFovY;
}

float CameraComponent::GetFieldOfViewY() const
{
    return mPerspectiveSettings.mFovY;
}

float CameraComponent::GetFieldOfViewX() const
{
    float aspectRatio = mPerspectiveSettings.mAspectRatio;
    float fovRadiansY = mPerspectiveSettings.mFovY * DEGREES_TO_RADIANS;

    float fovRadiansX = 2 * atanf(tanf(fovRadiansY * 0.5f) * aspectRatio);
    float fovDegreesX = fovRadiansX * RADIANS_TO_DEGREES;

    return fovDegreesX;
}

float CameraComponent::GetAspectRatio() const
{
    return mPerspectiveSettings.mAspectRatio;
}

float CameraComponent::GetWidth() const
{
    return mOrthoSettings.mWidth;
}

float CameraComponent::GetHeight() const
{
    return mOrthoSettings.mHeight;
}

void CameraComponent::SetNearZ(float nearZ)
{
    if (mProjectionMode == ProjectionMode::PERSPECTIVE)
    {
        mPerspectiveSettings.mNear = nearZ;
    }
    else
    {
        mOrthoSettings.mNear = nearZ;
    }
}

void CameraComponent::SetFarZ(float farZ)
{
    if (mProjectionMode == ProjectionMode::PERSPECTIVE)
    {
        mPerspectiveSettings.mNear = farZ;
    }
    else
    {
        mOrthoSettings.mNear = farZ;
    }
}

void CameraComponent::SetFieldOfView(float fovY)
{
    mPerspectiveSettings.mFovY = fovY;
}

void CameraComponent::SetAspectRatio(float aspectRatio)
{
    mPerspectiveSettings.mAspectRatio = aspectRatio;
}

void CameraComponent::SetWidth(float width)
{
    mOrthoSettings.mWidth = width;
}

void CameraComponent::SetHeight(float height)
{
    mOrthoSettings.mHeight = height;
}

glm::vec3 CameraComponent::WorldToScreenPosition(glm::vec3 worldPos)
{
    glm::vec3 screenPos = {};

    if (GetWorld())
    {
        glm::vec4 clipPos = mStandardViewProjectionMatrix * glm::vec4(worldPos, 1.0f);

        float w = clipPos.w;
        clipPos /= w;

        glm::vec2 screen2d = glm::vec2(clipPos);
        screen2d += glm::vec2(1.0f, 1.0f);
        screen2d *= glm::vec2(0.5f, 0.5f);
        screen2d *= glm::vec2(GetEngineState()->mWindowWidth, GetEngineState()->mWindowHeight);

        screenPos.x = screen2d.x;
        screenPos.y = screen2d.y;
        screenPos.z = w;
    }

    return screenPos;
}
