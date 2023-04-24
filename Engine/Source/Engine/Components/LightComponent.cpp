#include "Components/LightComponent.h"

DEFINE_RTTI(LightComponent);

static const char* sLightingDomainStrings[] =
{
    "Static",
    "Dynamic",
    "All"
};
static_assert(int32_t(LightingDomain::Count) == 3, "Need to update string conversion table");

LightComponent::LightComponent() :
    mColor(1,1,1,1),
    mIntensity(1),
    mDomain(LightingDomain::All),
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
    outProps.push_back(Property(DatumType::Byte, "Lighting Domain", this, &mDomain, 1, nullptr, 0, int32_t(LightingDomain::Count), sLightingDomainStrings));
    outProps.push_back(Property(DatumType::Bool, "Cast Shadows", this, &mCastShadows));
}

void LightComponent::SaveStream(Stream& stream)
{
    TransformComponent::SaveStream(stream);
    stream.WriteVec4(mColor);
    stream.WriteFloat(mIntensity);
    stream.WriteBool(mCastShadows);
    stream.WriteUint8((uint8_t)mDomain);
}

void LightComponent::LoadStream(Stream& stream)
{
    TransformComponent::LoadStream(stream);
    mColor = stream.ReadVec4();
    mIntensity = stream.ReadFloat();
    mCastShadows = stream.ReadBool();
    mDomain = (LightingDomain) stream.ReadUint8();
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

void LightComponent::SetLightingDomain(LightingDomain domain)
{
    mDomain = domain;
}

LightingDomain LightComponent::GetLightingDomain() const
{
    return mDomain;
}

void LightComponent::SetCastShadows(bool castShadows)
{
    mCastShadows = castShadows;
}

bool LightComponent::ShouldCastShadows() const
{
    return mCastShadows;
}
