#include "Components/LightComponent.h"

DEFINE_RTTI(LightComponent);

LightComponent::LightComponent() :
    mColor(1,1,1,1),
    mIntensity(1),
    mCastShadows(true)
{

}

LightComponent::~LightComponent()
{

}

const char* LightComponent::GetTypeName() const
{
    return "Light";
}

void LightComponent::GatherProperties(std::vector<Property>& outProps)
{
    TransformComponent::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Color, "Color", this, &mColor));
    outProps.push_back(Property(DatumType::Bool, "Cast Shadows", this, &mCastShadows));
}

void LightComponent::SaveStream(Stream& stream)
{
    TransformComponent::SaveStream(stream);
    stream.WriteVec4(mColor);
    stream.WriteFloat(mIntensity);
    stream.WriteBool(mCastShadows);
}

void LightComponent::LoadStream(Stream& stream)
{
    TransformComponent::LoadStream(stream);
    mColor = stream.ReadVec4();
    mIntensity = stream.ReadFloat();
    mCastShadows = stream.ReadBool();
}

bool LightComponent::IsLightComponent() const
{
    return true;
}

void LightComponent::SetColor(glm::vec4 color)
{
    mColor = color;
}

glm::vec4 LightComponent::GetColor() const
{
    return mColor;
}

void LightComponent::SetIntensity(float intensity)
{
    mIntensity = intensity;
}

float LightComponent::GetIntensity() const
{
    return mIntensity;
}

void LightComponent::SetCastShadows(bool castShadows)
{
    mCastShadows = castShadows;
}

bool LightComponent::ShouldCastShadows() const
{
    return mCastShadows;
}
