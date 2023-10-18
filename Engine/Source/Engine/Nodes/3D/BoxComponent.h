#pragma once

#include "Nodes/3D/PrimitiveComponent.h"

class Box3D : public Primitive3D
{
public:

    DECLARE_NODE(Box3D, Primitive3D);

    Box3D();
    ~Box3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    glm::vec3 GetExtents() const;
    void SetExtents(glm::vec3 extents);

protected:

    void UpdateRigidBody();

    static const float sDefaultExtent;
    glm::vec3 mExtents;
};