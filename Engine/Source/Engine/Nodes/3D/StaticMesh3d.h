#pragma once

#include "Nodes/3D/Mesh3d.h"
#include "AssetRef.h"

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsTypes.h"

class StaticMesh;

class StaticMesh3D : public Mesh3D
{
public:

    DECLARE_NODE(StaticMesh3D, Mesh3D);

    StaticMesh3D();
    ~StaticMesh3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    StaticMeshCompResource* GetResource();

    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void LoadStream(Stream& stream, Platform platform, uint32_t version) override;

    virtual bool IsStaticMesh3D() const override;
    virtual bool IsSkeletalMesh3D() const override;

    virtual void SetStaticMesh(StaticMesh* staticMesh);
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

    void ClearInstanceColors();
    void SetInstanceColors(const std::vector<uint32_t>& colors, bool bakedLighting);
    std::vector<uint32_t>& GetInstanceColors();
    bool HasBakedLighting() const;
    bool HasInstanceColors() const;

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    void DrawDebugCollision(std::vector<DebugDraw>& inoutDraws, btCollisionShape* colShape, const glm::mat4& parentTransform);
    virtual void RecreateCollisionShape();

    StaticMeshRef mStaticMesh;
    std::vector<uint32_t> mInstanceColors; // e.g. baked lighting color
    bool mUseTriangleCollision;
    bool mBakeLighting;
    bool mHasBakedLighting;

    // Graphics Resource
    StaticMeshCompResource mResource;
};