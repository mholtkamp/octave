#include "Nodes/3D/DirectionalLightComponent.h"
#include "Renderer.h"
#include "Constants.h"
#include "Maths.h"

FORCE_LINK_DEF(DirectionalLightComponent);
DEFINE_NODE(DirectionalLightComponent);

DirectionalLightComponent::DirectionalLightComponent()
{
    mName = "Directional Light";
    mViewProjectionMatrix = glm::mat4(1);
}

DirectionalLightComponent::~DirectionalLightComponent()
{

}

void DirectionalLightComponent::SetOwner(Actor* newOwner)
{
    LightComponent::SetOwner(newOwner);
}

void DirectionalLightComponent::Destroy()
{
    LightComponent::Destroy();
}

void DirectionalLightComponent::Tick(float deltaTime)
{
    LightComponent::Tick(deltaTime);
    GenerateViewProjectionMatrix();
}

const char* DirectionalLightComponent::GetTypeName() const
{
    return "DirectionalLight";
}

void DirectionalLightComponent::GatherProperties(std::vector<Property>& outProps)
{
    LightComponent::GatherProperties(outProps);
}

void DirectionalLightComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    LightComponent::GatherProxyDraws(inoutDraws);


    if (GetType() == DirectionalLightComponent::GetStaticType())
    {
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
            debugDraw.mActor = GetOwner();
            debugDraw.mComponent = this;
            debugDraw.mColor = color;
            glm::mat4 trans = MakeTransform({}, { -90.0f, 0.0f, 0.0f}, { scale * 0.5, scale, scale * 0.5});
            debugDraw.mTransform = mTransform * trans; // glm::scale(mTransform, { rScale, hScale, rScale });
            inoutDraws.push_back(debugDraw);
        }

        {
            // Pointer Cone
            DebugDraw debugDraw;
            debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cone");
            debugDraw.mActor = GetOwner();
            debugDraw.mComponent = this;
            debugDraw.mColor = color;
            glm::mat4 trans = MakeTransform({0.0f, 0.0f, -2.0f * scale}, { -90.0f, 0.0f, 0.0f }, { scale, scale, scale });
            debugDraw.mTransform = mTransform * trans;
            inoutDraws.push_back(debugDraw);
        }
    }
#endif
}

void DirectionalLightComponent::SaveStream(Stream& stream)
{
    LightComponent::SaveStream(stream);
}

void DirectionalLightComponent::LoadStream(Stream& stream)
{
    LightComponent::LoadStream(stream);
}

bool DirectionalLightComponent::IsPointLightComponent() const
{
    return false;
}

bool DirectionalLightComponent::IsDirectionalLightComponent() const
{
    return true;
}

glm::vec3 DirectionalLightComponent::GetDirection() const
{
    return GetForwardVector();
}

void DirectionalLightComponent::SetDirection(const glm::vec3& dir)
{
    LookAt(GetAbsolutePosition() + dir, { 0.0f, 1.0f, 0.0f });
}

const glm::mat4& DirectionalLightComponent::GetViewProjectionMatrix() const
{
    return mViewProjectionMatrix;
}

void DirectionalLightComponent::GenerateViewProjectionMatrix()
{
    glm::mat4 view;
    glm::mat4 proj;

    glm::vec3 cameraPosition = GetWorld()->GetActiveCamera()->GetAbsolutePosition();
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
