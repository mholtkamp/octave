#pragma once

#include "Nodes/3D/Primitive3d.h"

class Capsule3D : public Primitive3D
{
public:

    DECLARE_NODE(Capsule3D, Primitive3D);

    Capsule3D();
    ~Capsule3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;

    float GetHeight() const;
    void SetHeight(float height);

    float GetRadius() const;
    void SetRadius(float radius);

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    void UpdateRigidBody();

    static const float sDefaultHeight;
    static const float sDefaultRadius;

    float mHeight = 1.0f;
    float mRadius = 1.0f;
};