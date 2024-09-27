#pragma once

#include "Node3d.h"

class Light3D : public Node3D
{
public:

    DECLARE_RTTI(Light3D, Node3D);

    Light3D();
    ~Light3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual bool IsLight3D() const;
    virtual bool IsPointLight3D() const = 0;
    virtual bool IsDirectionalLight3D() const = 0;

    void SetColor(glm::vec4 color);
    glm::vec4 GetColor() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

    void SetLightingDomain(LightingDomain domain);
    LightingDomain GetLightingDomain() const;

    void SetCastShadows(bool castShadows);
    bool ShouldCastShadows() const;

    uint8_t GetLightingChannels() const;
    void SetLightingChannels(uint8_t channels);

protected:

    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    float mIntensity = 1.0f;
    LightingDomain mDomain = LightingDomain::Dynamic;
    bool mCastShadows = true;
    uint8_t mLightingChannels = 0x01;
};