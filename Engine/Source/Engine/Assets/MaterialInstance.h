#pragma once

#include "Assets/Material.h"
#include "Assets/MaterialInterface.h"

class MaterialInstance : public Asset, public MaterialInterface
{
public:

    DECLARE_ASSET(MaterialInstance, Asset);

    static MaterialInstance* New(MaterialInterface* src = nullptr);

    MaterialInstance();
    ~MaterialInstance();

    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream);
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    Material* GetBaseMaterial() const;
    void SetBaseMaterial(Material* material);

    // Begin MaterialInterface
    virtual void MarkDirty() override;
    virtual void ClearDirty(uint32_t frameIndex) override;
    virtual bool IsDirty(uint32_t frameIndex) override;
    virtual Material* AsBase() override;
    virtual MaterialInstance* AsInstance() override;
    virtual bool IsBase() const override;
    virtual bool IsInstance() const override;
    virtual std::vector<ShaderParameter>& GetParameters() override;
    // End MaterialInterface

protected:

    // Property
    Material* mBaseMaterial = nullptr;

    // Shader Parameters
    std::vector<ShaderParameter> mParameters;

    // Misc
    bool mDirty[MAX_FRAMES] = {};

    // Graphics Resource
    MaterialResource mResource;
};
