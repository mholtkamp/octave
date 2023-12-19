#pragma once

#include "Assets/Material.h"

class MaterialBase;

class MaterialInstance : public Material
{
public:

    DECLARE_ASSET(MaterialInstance, Material);

    static MaterialInstance* New(Material* src = nullptr);

    MaterialInstance();
    ~MaterialInstance();

    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    virtual bool IsInstance() const override;

    MaterialBase* GetBaseMaterial() const;
    void SetBaseMaterial(MaterialBase* material);

    void LinkToBase();

    virtual BlendMode GetBlendMode() const override;
    virtual float GetMaskCutoff() const override;
    virtual int32_t GetSortPriority() const override;
    virtual bool IsDepthTestDisabled() const override;
    virtual bool ShouldApplyFog() const override;
    virtual CullMode GetCullMode() const override;

protected:


    // Property
    MaterialRef mBaseMaterial;
};
