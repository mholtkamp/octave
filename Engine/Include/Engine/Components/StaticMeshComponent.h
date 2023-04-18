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

    void SetBakeLighting(bool bake);
    bool GetBakeLighting() const;

    virtual Material* GetMaterial() override;
    virtual void Render() override;

    virtual VertexType GetVertexType() const override;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual Bounds GetLocalBounds() const override;

protected:

    virtual void RecreateCollisionShape();

    StaticMeshRef mStaticMesh;
    std::vector<uint32_t> mInstanceColors; // e.g. baked lighting color
    bool mUseTriangleCollision;
    bool mBakeLighting;

    // Graphics Resource
    StaticMeshCompResource mResource;
};