#pragma once

#include "Components/PrimitiveComponent.h"

class SphereComponent : public PrimitiveComponent
{
public:

    DECLARE_NODE(SphereComponent, PrimitiveComponent);

    SphereComponent();
    ~SphereComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    float GetRadius() const;
    void SetRadius(float radius);

protected:

    void UpdateRigidBody();

    static const float sDefaultRadius;
    float mRadius;
};
