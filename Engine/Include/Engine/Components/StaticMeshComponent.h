#pragma once

#include "Components/MeshComponent.h"
#include "AssetRef.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsTypes.h"

class StaticMesh;

class StaticMeshComponent : public MeshComponent
{
public:

    DECLARE_COMPONENT(StaticMeshComponent, MeshComponent);

    StaticMeshComponent();
    ~StaticMeshComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    StaticMeshCompResource* GetResource();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual bool IsStaticMeshComponent() const override;
    virtual bool IsSkeletalMeshComponent() const override;

    void SetStaticMesh(StaticMesh* staticMesh);
    StaticMesh* GetStaticMesh();

    void SetUseTriangleCollision(bool triangleCol);
    bool GetUseTriangleCollision() const;

    virtual Material* GetMaterial() override;
    virtual void Render() override;

    virtual VertexType GetVertexType() const override;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual Bounds GetLocalBounds() const override;

protected:

    virtual void RecreateCollisionShape();

    StaticMeshRef mStaticMesh;
    bool mUseTriangleCollision;

    // Graphics Resource
    StaticMeshCompResource mResource;
};