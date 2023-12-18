#pragma once

#include "Assets/Material.h"

#include "Graphics/GraphicsConstants.h"
#include "Graphics/GraphicsTypes.h"

class Texture;

class MaterialBase : public Material
{
public:

    DECLARE_ASSET(MaterialBase, Material);

    MaterialBase();
    ~MaterialBase();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    BlendMode GetBlendMode() const;
    void SetBlendMode(BlendMode blendMode);

    float GetMaskCutoff() const;
    void SetMaskCutoff(float cutoff);

    int32_t GetSortPriority() const;
    void SetSortPriority(int32_t priority);

    bool IsDepthTestDisabled() const;
    void SetDepthTestDisabled(bool depthTest);

    bool ShouldApplyFog() const;
    void SetApplyFog(bool applyFog);

    CullMode GetCullMode() const;
    void SetCullMode(CullMode cullMode);

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    // Properties
    std::string mShader;
    BlendMode mBlendMode = BlendMode::Opaque;
    float mMaskCutoff = 0.5f;
    int32_t mSortPriority = 0;
    bool mDisableDepthTest = false;
    bool mApplyFog = true;

    // Misc
    std::vector<uint8_t> mVertexCode;
    std::vector<uint8_t> mFragmentCode;
};