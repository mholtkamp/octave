#pragma once

#include "Nodes/3D/PrimitiveComponent.h"

class CapsuleComponent : public PrimitiveComponent
{
public:

    DECLARE_NODE(CapsuleComponent, PrimitiveComponent);

    CapsuleComponent();
    ~CapsuleComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    float GetHeight() const;
    void SetHeight(float height);

    float GetRadius() const;
    void SetRadius(float radius);

protected:

    void UpdateRigidBody();

    static const float sDefaultHeight;
    static const float sDefaultRadius;

    float mHeight = 1.0f;
    float mRadius = 1.0f;
};