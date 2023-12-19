#pragma once

#include "Assets/Material.h"

#include "Vertex.h"

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

    virtual bool IsBase() const override;

    void Compile();
    void MarkStale();

    virtual BlendMode GetBlendMode() const override;
    virtual float GetMaskCutoff() const override;
    virtual int32_t GetSortPriority() const override;
    virtual bool IsDepthTestDisabled() const override;
    virtual bool ShouldApplyFog() const override;
    virtual CullMode GetCullMode() const override;

    void SetBlendMode(BlendMode blendMode);
    void SetMaskCutoff(float cutoff);
    void SetSortPriority(int32_t priority);
    void SetDepthTestDisabled(bool depthTest);
    void SetApplyFog(bool applyFog);
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
    CullMode mCullMode = CullMode::Back;

    // Misc
    std::vector<uint8_t> mVertexCode[(uint32_t)VertexType::Max];
    std::vector<uint8_t> mFragmentCode;
    bool mCompiled = false;
    bool mCompilationStale = false;
};