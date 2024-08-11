#pragma once

#include "Nodes/3D/Primitive3d.h"

class Sphere3D : public Primitive3D
{
public:

    DECLARE_NODE(Sphere3D, Primitive3D);

    Sphere3D();
    ~Sphere3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;

    float GetRadius() const;
    void SetRadius(float radius);

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    void UpdateRigidBody();

    static const float sDefaultRadius;
    float mRadius;
};
