#pragma once

#include "LightComponent.h"
#include "AssetRef.h"

struct PointLightData
{
    glm::mat4 mWVP;
    glm::vec4 mPosition;
    glm::vec4 mColor;
    float mRadius;
    float mPadding0;
    float mPadding1;
    float mPadding2;

    PointLightData() :
        mRadius(1.0f),
        mPadding0(0.0f),
        mPadding1(0.0f),
        mPadding2(0.0f)
    {
    }
};

class PointLightComponent : public LightComponent
{
public:

    DECLARE_COMPONENT(PointLightComponent, LightComponent);

    PointLightComponent();
    ~PointLightComponent();

    virtual void Create() override;
    virtual void Destroy() override;

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void SaveStream(Stream& stream) override;
	virtual void LoadStream(Stream& stream) override;

    virtual bool IsPointLightComponent() const override;
    virtual bool IsDirectionalLightComponent() const override;

    void SetRadius(float radius);
    float GetRadius() const;

protected:

    float mRadius;
};