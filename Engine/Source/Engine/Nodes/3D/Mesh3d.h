#pragma once

#include "Nodes/3D/PrimitiveComponent.h"
#include "Assets/Material.h"
#include "Assets/MaterialInstance.h"

#if EDITOR
#include <assimp/scene.h>
#endif

class Mesh3D : public Primitive3D
{
public:

    DECLARE_RTTI(Mesh3D, Primitive3D);

    Mesh3D();
    virtual ~Mesh3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual bool IsStaticMeshComponent() const = 0;
    virtual bool IsSkeletalMeshComponent() const = 0;
    virtual bool IsShadowMeshComponent();

    virtual Material* GetMaterial() = 0;
    Material* GetMaterialOverride();
    virtual void SetMaterialOverride(Material* material);
    MaterialInstance* InstantiateMaterial();

    bool IsBillboard() const;
    void SetBillboard(bool billboard);
    glm::mat4 ComputeBillboardTransform();
    glm::mat4 GetRenderTransform();

    virtual DrawData GetDrawData() override;

protected:

    MaterialRef mMaterialOverride;
    bool mBillboard = false;
};