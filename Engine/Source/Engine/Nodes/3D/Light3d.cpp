#include "Nodes/3D/Light3d.h"

DEFINE_RTTI(Light3D);

static const char* sLightingDomainStrings[] =
{
    "Static",
    "Dynamic",
    "All"
};
static_assert(int32_t(LightingDomain::Count) == 3, "Need to update string conversion table");

Light3D::Light3D()
{

}

Light3D::~Light3D()
{

}

const char* Light3D::GetTypeName() const
{
    return "Light";
}

void Light3D::GatherProperties(std::vector<Property>& outProps)
{
    Node3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Light");

    outProps.push_back(Property(DatumType::Color, "Color", this, &mColor));
    outProps.push_back(Property(DatumType::Float, "Intensity", this, &mIntensity));
    outProps.push_back(Property(DatumType::Byte, "Lighting Domain", this, &mDomain, 1, nullptr, 0, int32_t(LightingDomain::Count), sLightingDomainStrings));
    outProps.push_back(Property(DatumType::Bool, "Cast Shadows", this, &mCastShadows));
    outProps.push_back(Property(DatumType::Byte, "Lighting Channels", this, &mLightingChannels, 1, nullptr, (int32_t)ByteExtra::FlagWidget));
}

bool Light3D::IsLight3D() const
{
    return true;
}

void Light3D::SetColor(glm::vec4 color)
{
    mColor = color;
}

glm::vec4 Light3D::GetColor() const
{
    return mColor;
}

void Light3D::SetIntensity(float intensity)
{
    mIntensity = intensity;
}

float Light3D::GetIntensity() const
{
    return mIntensity;
}

void Light3D::SetLightingDomain(LightingDomain domain)
{
    mDomain = domain;
}

LightingDomain Light3D::GetLightingDomain() const
{
    return mDomain;
}

void Light3D::SetCastShadows(bool castShadows)
{
    mCastShadows = castShadows;
}

bool Light3D::ShouldCastShadows() const
{
    return mCastShadows;
}

uint8_t Light3D::GetLightingChannels() const
{
    return mLightingChannels;
}

void Light3D::SetLightingChannels(uint8_t channels)
{
    mLightingChannels = channels;
}
