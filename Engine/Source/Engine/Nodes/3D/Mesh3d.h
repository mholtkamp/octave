#pragma once

#include "Nodes/3D/Primitive3d.h"
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

    virtual bool IsStaticMesh3D() const;
    virtual bool IsSkeletalMesh3D() const;
    virtual bool IsInstancedMesh3D() const;
    virtual bool IsShadowMesh3D() const;

    virtual Material* GetMaterial() = 0;
    Material* GetMaterialOverride();
    virtual void SetMaterialOverride(Material* material);
    Material* InstantiateMaterial();

    bool IsBillboard() const;
    void SetBillboard(bool billboard);
    glm::mat4 ComputeBillboardTransform();
    glm::mat4 GetRenderTransform();

    virtual DrawData GetDrawData() override;

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    MaterialRef mMaterialOverride;
    bool mBillboard = false;
};