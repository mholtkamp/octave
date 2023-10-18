#include "Nodes/3D/LightComponent.h"

DEFINE_RTTI(Light3D);

static const char* sLightingDomainStrings[] =
{
    "Static",
    "Dynamic",
    "All"
};
static_assert(int32_t(LightingDomain::Count) == 3, "Need to update string conversion table");

Light3D::Light3D() :
    mColor(1,1,1,1),
    mIntensity(1),
    mDomain(LightingDomain::Dynamic),
    mCastShadows(true)
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

    outProps.push_back(Property(DatumType::Color, "Color", this, &mColor));
    outProps.push_back(Property(DatumType::Byte, "Lighting Domain", this, &mDomain, 1, nullptr, 0, int32_t(LightingDomain::Count), sLightingDomainStrings));
    outProps.push_back(Property(DatumType::Bool, "Cast Shadows", this, &mCastShadows));
}

void Light3D::SaveStream(Stream& stream)
{
    Node3D::SaveStream(stream);
    stream.WriteVec4(mColor);
    stream.WriteFloat(mIntensity);
    stream.WriteBool(mCastShadows);
    stream.WriteUint8((uint8_t)mDomain);
}

void Light3D::LoadStream(Stream& stream)
{
    Node3D::LoadStream(stream);
    mColor = stream.ReadVec4();
    mIntensity = stream.ReadFloat();
    mCastShadows = stream.ReadBool();
    mDomain = (LightingDomain) stream.ReadUint8();
}

bool Light3D::IsLightComponent() const
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
