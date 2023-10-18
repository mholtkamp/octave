#pragma once

#include "Nodes/3D/StaticMesh3d.h"

class ShadowMesh3D : public StaticMesh3D
{
public:

    DECLARE_NODE(ShadowMesh3D, StaticMesh3D);

    ShadowMesh3D();
    ~ShadowMesh3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;

    virtual void Render() override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual bool IsShadowMeshComponent() override;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;
};
