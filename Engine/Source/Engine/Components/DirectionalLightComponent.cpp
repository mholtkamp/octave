#include "Components/DirectionalLightComponent.h"
#include "Renderer.h"
#include "Constants.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

FORCE_LINK_DEF(DirectionalLightComponent);
DEFINE_COMPONENT(DirectionalLightComponent);

DirectionalLightComponent::DirectionalLightComponent() :
    mDirection(0, -1, 0)
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

    if (mOwner != nullptr &&
        mOwner->GetWorld()->GetDirectionalLight() == nullptr)
    {
        mOwner->GetWorld()->SetDirectionalLight(this);
    }
}

void DirectionalLightComponent::Destroy()
{
    LightComponent::Destroy();

    if (mOwner != nullptr &&
        mOwner->GetWorld()->GetDirectionalLight() == this)
    {
        mOwner->GetWorld()->SetDirectionalLight(nullptr);
    }
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

    outProps.push_back(Property(DatumType::Vector, "Direction", this, &mDirection));
}

void DirectionalLightComponent::SaveStream(Stream& stream)
{
    LightComponent::SaveStream(stream);
    stream.WriteVec3(mDirection);
}

void DirectionalLightComponent::LoadStream(Stream& stream)
{
    LightComponent::LoadStream(stream);
    mDirection = stream.ReadVec3();
}

bool DirectionalLightComponent::IsPointLightComponent() const
{
    return false;
}

bool DirectionalLightComponent::IsDirectionalLightComponent() const
{
    return true;
}

const glm::vec3& DirectionalLightComponent::GetDirection() const
{
    return mDirection;
}

void DirectionalLightComponent::SetDirection(const glm::vec3& dir)
{
    mDirection = dir;
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

    glm::vec3 upVector = fabs(mDirection.y) > 0.5f ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    view = glm::lookAtRH(cameraPosition, cameraPosition + mDirection, upVector);
    proj = glm::orthoRH(-SHADOW_RANGE, SHADOW_RANGE, -SHADOW_RANGE, SHADOW_RANGE, -SHADOW_RANGE_Z, SHADOW_RANGE_Z);

    // Needed for adjusting to NDC
    const glm::mat4 clip(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    mViewProjectionMatrix = clip * proj * view;
}
