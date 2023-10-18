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

    virtual void SaveStream(Stream& stream) override;
	virtual void LoadStream(Stream& stream) override;

    virtual bool IsLightComponent() const;
    virtual bool IsPointLightComponent() const = 0;
    virtual bool IsDirectionalLightComponent() const = 0;

    void SetColor(glm::vec4 color);
    glm::vec4 GetColor() const;

    void SetIntensity(float intensity);
    float GetIntensity() const;

    void SetLightingDomain(LightingDomain domain);
    LightingDomain GetLightingDomain() const;

    void SetCastShadows(bool castShadows);
    bool ShouldCastShadows() const;

protected:

    glm::vec4 mColor;
    float mIntensity;
    LightingDomain mDomain;
    bool mCastShadows;
};