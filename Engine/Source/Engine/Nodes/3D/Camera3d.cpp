#include "Nodes/3D/Camera3d.h"
#include "Log.h"
#include "Engine.h"
#include "World.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Maths.h"

#include "Nodes/3D/Primitive3d.h"

#include "Graphics/Graphics.h"

#if EDITOR
#include "EditorState.h"
#endif

FORCE_LINK_DEF(Camera3D);
DEFINE_NODE(Camera3D, Node3D);

Camera3D::Camera3D() :
    mProjectionMode(ProjectionMode::PERSPECTIVE),
    mViewProjectionMatrix(1)
{
    mName = "Camera";
}

Camera3D::~Camera3D()
{

}

const char* Camera3D::GetTypeName() const
{
    return "Camera";
}

void Camera3D::Start()
{
    Node3D::Start();
}

void Camera3D::Destroy()
{
    Node3D::Destroy();
}

void Camera3D::GatherProperties(std::vector<Property>& outProps)
{
    Node3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Camera");

    outProps.push_back(Property(DatumType::Bool, "Perspective", this, &mProjectionMode));
    outProps.push_back(Property(DatumType::Float, "Near Plane", this, &mNear));
    outProps.push_back(Property(DatumType::Float, "Far Plane", this, &mFar));
    outProps.push_back(Property(DatumType::Float, "Field Of View", this, &mFovY));
    outProps.push_back(Property(DatumType::Float, "Ortho Width", this, &mOrthoWidth));
}

void Camera3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    if (this != GetWorld()->GetActiveCamera())
    {
        glm::mat4 transform = glm::rotate(DEGREES_TO_RADIANS * -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = mTransform * transform;

        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cone");
        debugDraw.mNode = this;
        debugDraw.mColor = glm::vec4(0.0f, 0.85f, 0.25f, 1.0f);
        debugDraw.mTransform = glm::scale(transform, { 0.5f, 0.5f, 0.5f });
        inoutDraws.push_back(debugDraw);
    }
#endif
}

ProjectionMode Camera3D::GetProjectionMode() const
{
    return mProjectionMode;
}

void Camera3D::SetProjectionMode(ProjectionMode mode)
{
    if (mProjectionMode != mode)
    {
        mProjectionMode = mode;
    }
}

void Camera3D::EnablePerspective(bool perspective)
{
    SetProjectionMode(perspective ? ProjectionMode::PERSPECTIVE : ProjectionMode::ORTHOGRAPHIC);
}

bool Camera3D::IsPerspectiveEnabled() const
{
    return mProjectionMode == ProjectionMode::PERSPECTIVE;
}

const glm::mat4& Camera3D::GetViewProjectionMatrix()
{
    return mViewProjectionMatrix;
}

const glm::mat4& Camera3D::GetViewMatrix()
{
    return mViewMatrix;
}

const glm::mat4& Camera3D::GetProjectionMatrix()
{
    return mProjectionMatrix;
}

void Camera3D::ComputeMatrices()
{
    // Make sure transform is up to date.
    UpdateTransform(false);

    EngineState* engineState = GetEngineState();
    Renderer* renderer = Renderer::Get();
    mAspectRatio = static_cast<float>(renderer->GetViewportWidth()) / (renderer->GetViewportHeight());

    // Use the scaling factor to address Wii widescreen stretching
    mAspectRatio *= engineState->mAspectRatioScale;

    mViewMatrix = CalculateViewMatrix();
    mViewMatrix = glm::toMat4(glm::conjugate(GetWorldRotationQuat()));

    mViewMatrix = translate(mViewMatrix, -GetWorldPosition());

    if (mProjectionMode == ProjectionMode::ORTHOGRAPHIC)
    {
        float orthoHeight = mOrthoWidth / mAspectRatio;

        mProjectionMatrix = GFX_MakeOrthographicMatrix(
            -mOrthoWidth,
            mOrthoWidth,
            -orthoHeight,
            orthoHeight,
            mNear,
            mFar);

        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;

        glm::mat4 stdProjMtx = glm::ortho(
            -mOrthoWidth,
            mOrthoWidth,
            -orthoHeight,
            orthoHeight,
            mNear,
            mFar);

        stdProjMtx[1][1] *= -1.0f;

        mStandardViewProjectionMatrix = stdProjMtx * mViewMatrix;
    }
    else
    {
        mProjectionMatrix = GFX_MakePerspectiveMatrix(mFovY,
            mAspectRatio,
            mNear,
            mFar);

        mViewProjectionMatrix = mProjectionMatrix * mViewMatrix;

        // Because the 3DS projection matrix is wonky and I don't know how to 
        // derive the clipspace position for World-To-Screen conversions,
        // just create a standard perspective matrix and we can use it for the conversions.
        glm::mat4 stdProjMtx = glm::perspectiveFov(
            glm::radians(mFovY),
            mAspectRatio,
            1.0f,
            mNear,
            mFar);

        stdProjMtx[1][1] *= -1.0f;

        mStandardViewProjectionMatrix = stdProjMtx * mViewMatrix;
    }
}

glm::mat4 Camera3D::CalculateViewMatrix()
{
    glm::mat4 view = glm::toMat4(glm::conjugate(GetWorldRotationQuat()));
    view = translate(mViewMatrix, -GetWorldPosition());
    return view;
}

glm::mat4 Camera3D::CalculateInvViewMatrix()
{
    glm::mat4 invView;

    invView = CalculateViewMatrix();
    invView = glm::inverse(invView);

    return invView;
}

float Camera3D::GetNearZ() const
{
    return mNear;
}

float Camera3D::GetFarZ() const
{
    return mFar;
}

float Camera3D::GetFieldOfView() const
{
    return mFovY;
}

float Camera3D::GetFieldOfViewY() const
{
    return mFovY;
}

float Camera3D::GetFieldOfViewX() const
{
    float aspectRatio = mAspectRatio;
    float fovRadiansY = mFovY * DEGREES_TO_RADIANS;

    float fovRadiansX = 2 * atanf(tanf(fovRadiansY * 0.5f) * aspectRatio);
    float fovDegreesX = fovRadiansX * RADIANS_TO_DEGREES;

    return fovDegreesX;
}

float Camera3D::GetOrthoWidth() const
{
    return mOrthoWidth;
}

float Camera3D::GetOrthoHeight() const
{
    return (mOrthoWidth / mAspectRatio);
}

float Camera3D::GetAspectRatio() const
{
    return mAspectRatio;
}

float Camera3D::GetNearWidth() const
{
    float width = 0.0f;

    if (mProjectionMode == ProjectionMode::PERSPECTIVE)
    {
        width = GetNearHeight() * mAspectRatio;
    }
    else
    {
        width = mOrthoWidth / 2.0f;
    }

    return width;
}

float Camera3D::GetNearHeight() const
{
    float height = 0.0f;

    if (mProjectionMode == ProjectionMode::PERSPECTIVE)
    {
        height = 2.0f * mNear * tanf(mFovY * 0.5f * DEGREES_TO_RADIANS);
    }
    else
    {
        float orthoHeight = mOrthoWidth / mAspectRatio;
        height = orthoHeight / 2.0f;
    }

    return height;
}

void Camera3D::SetNearZ(float nearZ)
{
    mNear = nearZ;
}

void Camera3D::SetFarZ(float farZ)
{
    mFar = farZ;
}

void Camera3D::SetFieldOfView(float fovY)
{
    mFovY = fovY;
}

void Camera3D::SetOrthoWidth(float width)
{
    mOrthoWidth = width;
}

glm::vec3 Camera3D::WorldToScreenPosition(glm::vec3 worldPos)
{
    glm::vec3 screenPos = {};

    if (GetWorld())
    {
        Renderer* renderer = Renderer::Get();
        glm::vec4 clipPos = mStandardViewProjectionMatrix * glm::vec4(worldPos, 1.0f);

        float w = clipPos.w;
        clipPos /= w;

        glm::vec2 screen2d = glm::vec2(clipPos);
        screen2d += glm::vec2(1.0f, 1.0f);
        screen2d *= glm::vec2(0.5f, 0.5f);
        screen2d *= glm::vec2(renderer->GetViewportWidth(), renderer->GetViewportHeight());
        screen2d += glm::vec2(renderer->GetViewportX(), renderer->GetViewportY());

        screenPos.x = screen2d.x;
        screenPos.y = screen2d.y;
        screenPos.z = w;
    }

    return screenPos;
}

glm::vec3 Camera3D::ScreenToWorldPosition(int32_t x, int32_t y)
{
    Renderer* renderer = Renderer::Get();
    float screenX = float(x);
    float screenY = float(y);
    float vpX = (float)renderer->GetViewportX();
    float vpY = (float)renderer->GetViewportY();
    float vpWidth = (float)renderer->GetViewportWidth();
    float vpHeight = (float)renderer->GetViewportHeight();

    float cX = ((screenX - vpX) / vpWidth) * 2.0f - 1.0f;
    float cY = ((screenY - vpY) / vpHeight) * 2.0f - 1.0f;
    float cZ = 0.0f; // Near Plane

    // Use standard VP here because android might rotate the proj matrix.
    glm::mat4 invViewProj = glm::inverse(mStandardViewProjectionMatrix);
    glm::vec4 worldPos4 = invViewProj * glm::vec4(cX, cY, cZ, 1.0f);
    worldPos4 = worldPos4 / worldPos4.w;

    glm::vec3 worldPos = glm::vec3(worldPos4.x, worldPos4.y, worldPos4.z);

    return worldPos;
}

glm::vec3 Camera3D::TraceScreenToWorld(int32_t x, int32_t y, uint8_t colMask, RayTestResult& rayResult)
{
    glm::vec3 worldPos = ScreenToWorldPosition(x, y);

    glm::vec3 startPos = GetWorldPosition();
    glm::vec3 rayDir = Maths::SafeNormalize(worldPos - startPos);
    glm::vec3 endPos = startPos + rayDir * GetFarZ();

    GetWorld()->RayTest(startPos, endPos, colMask, rayResult);

    return rayResult.mHitPosition;
}

const bool Camera3D::IsEditorCamera()
{
#if EDITOR
    return (this == GetEditorState()->mEditorCamera);
#endif

    return false;
}
