#include "Nodes/3D/DirectionalLight3d.h"
#include "Renderer.h"
#include "Constants.h"
#include "Maths.h"

FORCE_LINK_DEF(DirectionalLight3D);
DEFINE_NODE(DirectionalLight3D, Light3D);

DirectionalLight3D::DirectionalLight3D()
{
    mName = "Directional Light";
    mViewProjectionMatrix = glm::mat4(1);
}

DirectionalLight3D::~DirectionalLight3D()
{

}

void DirectionalLight3D::Destroy()
{
    Light3D::Destroy();
}

void DirectionalLight3D::Tick(float deltaTime)
{
    Light3D::Tick(deltaTime);
    TickCommon(deltaTime);
}

void DirectionalLight3D::EditorTick(float deltaTime)
{
    Light3D::EditorTick(deltaTime);
    TickCommon(deltaTime);
}

void DirectionalLight3D::TickCommon(float deltaTime)
{
    GenerateViewProjectionMatrix();
}

const char* DirectionalLight3D::GetTypeName() const
{
    return "DirectionalLight";
}

void DirectionalLight3D::GatherProperties(std::vector<Property>& outProps)
{
    Light3D::GatherProperties(outProps);
    //SCOPED_CATEGORY("Light");
}

void DirectionalLight3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    glm::vec4 color = glm::vec4(0.8f, 0.8f, 0.3f, 1.0f);

    if (mDomain == LightingDomain::Static)
    {
        color = glm::vec4(0.8f, 0.5f, 0.3f, 1.0f);
    }
    else if (mDomain == LightingDomain::Dynamic)
    {
        color = glm::vec4(0.8f, 0.8f, 0.6f, 1.0f);
    }

    float scale = 0.3f;

    {
        // Cylinder
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cylinder");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        glm::mat4 trans = MakeTransform({}, { -90.0f, 0.0f, 0.0f}, { scale * 0.5, scale, scale * 0.5});
        debugDraw.mTransform = mTransform * trans; // glm::scale(mTransform, { rScale, hScale, rScale });
        inoutDraws.push_back(debugDraw);
    }

    {
        // Pointer Cone
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cone");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        glm::mat4 trans = MakeTransform({0.0f, 0.0f, -2.0f * scale}, { -90.0f, 0.0f, 0.0f }, { scale, scale, scale });
        debugDraw.mTransform = mTransform * trans;
        inoutDraws.push_back(debugDraw);
    }
#endif
}

bool DirectionalLight3D::IsPointLight3D() const
{
    return false;
}

bool DirectionalLight3D::IsDirectionalLight3D() const
{
    return true;
}

glm::vec3 DirectionalLight3D::GetDirection() const
{
    return GetForwardVector();
}

void DirectionalLight3D::SetDirection(const glm::vec3& dir)
{
    LookAt(GetWorldPosition() + dir, { 0.0f, 1.0f, 0.0f });
}

const glm::mat4& DirectionalLight3D::GetViewProjectionMatrix() const
{
    return mViewProjectionMatrix;
}

void DirectionalLight3D::GenerateViewProjectionMatrix()
{
    Camera3D* camera = GetWorld()->GetActiveCamera();

    if (camera != nullptr)
    {
        glm::mat4 view;
        glm::mat4 proj;

        glm::vec3 cameraPosition = camera->GetWorldPosition();
        glm::vec3 direction = GetDirection();

        glm::vec3 upVector = fabs(direction.y) > 0.5f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
        view = glm::lookAtRH(cameraPosition, cameraPosition + direction, upVector);
        proj = glm::orthoRH(-SHADOW_RANGE, SHADOW_RANGE, -SHADOW_RANGE, SHADOW_RANGE, -SHADOW_RANGE_Z, SHADOW_RANGE_Z);

        // Needed for adjusting to NDC
        const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, -1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f);

        mViewProjectionMatrix = clip * proj * view;
    }
}
