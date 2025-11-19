#pragma once

#include "Assets/Material.h"

struct MaterialLiteParams
{
    ShadingModel mShadingModel = ShadingModel::Lit;
    BlendMode mBlendMode = BlendMode::Opaque;
    VertexColorMode mVertexColorMode = VertexColorMode::Modulate;
    TextureRef mTextures[MATERIAL_LITE_MAX_TEXTURES];
    uint8_t mUvMaps[MATERIAL_LITE_MAX_TEXTURES] = { };
    TevMode mTevModes[MATERIAL_LITE_MAX_TEXTURES] = { TevMode::Replace, TevMode::Pass, TevMode::Pass, TevMode::Pass };
    glm::vec2 mUvOffsets[MAX_UV_MAPS] = { { 0.0f, 0.0f }, {0.0f, 0.0f} };
    glm::vec2 mUvScales[MAX_UV_MAPS] = { { 1.0f, 1.0f }, {1.0f, 1.0f} };
    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 mFresnelColor = { 1.0f, 0.0f, 0.0f, 0.0f };
    float mEmission = 0.0f;
    float mWrapLighting = 0.0f;
    float mFresnelPower = 1.0f;
    float mSpecular = 0.0f;
    uint32_t mToonSteps = 2;
    float mOpacity = 1.0f;
    float mMaskCutoff = 0.5f;
    float mShininess = 32.0f;
    int32_t mSortPriority = 0;
    bool mDisableDepthTest = false;
    bool mFresnelEnabled = false;
    bool mApplyFog = true;
    CullMode mCullMode = CullMode::Back;
};

class MaterialLite : public Material
{
public:

    DECLARE_ASSET(MaterialLite, Material);

    static MaterialLite* New(Material* src = nullptr);

    MaterialLite();
    ~MaterialLite();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual bool Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    virtual bool IsLite() const override;

    const MaterialLiteParams& GetLiteParams() const;
    void SetLiteParams(const MaterialLiteParams& params);

    void SetTexture(TextureSlot slot, Texture* texture);
    Texture* GetTexture(TextureSlot slot);

    ShadingModel GetShadingModel() const;
    void SetShadingModel(ShadingModel shadingModel);

    virtual BlendMode GetBlendMode() const override;
    void SetBlendMode(BlendMode blendMode);

    VertexColorMode GetVertexColorMode() const;
    void SetVertexColorMode(VertexColorMode mode);

    glm::vec2 GetUvOffset(int32_t uvIndex = 0) const;
    void SetUvOffset(glm::vec2 offset, int32_t uvIndex = 0);

    glm::vec2 GetUvScale(int32_t uvIndex = 0) const;
    void SetUvScale(glm::vec2 scale, int32_t uvIndex = 0);

    glm::vec4 GetColor() const;
    void SetColor(const glm::vec4& color);

    glm::vec4 GetFresnelColor() const;
    void SetFresnelColor(const glm::vec4& color);

    float GetEmission() const;
    void SetEmission(float emission);

    float GetWrapLighting() const;
    void SetWrapLighting(float wrapLighting);

    float GetFresnelPower() const;
    void SetFresnelPower(float power);

    float GetSpecular() const;
    void SetSpecular(float specular);

    uint32_t GetToonSteps() const;
    void SetToonSteps(uint32_t steps);

    float GetOpacity() const;
    void SetOpacity(float opacity);

    virtual float GetMaskCutoff() const override;
    void SetMaskCutoff(float cutoff);

    float GetShininess() const;
    void SetShininess(float shininess);

    virtual int32_t GetSortPriority() const override;
    void SetSortPriority(int32_t priority);

    virtual bool IsDepthTestDisabled() const override;
    void SetDepthTestDisabled(bool depthTest);

    bool IsFresnelEnabled() const;
    void SetFresnelEnabled(bool enable);

    virtual bool ShouldApplyFog() const override;
    void SetApplyFog(bool applyFog);

    virtual CullMode GetCullMode() const override;
    void SetCullMode(CullMode cullMode);

    uint32_t GetUvMap(uint32_t textureSlot);
    void SetUvMap(uint32_t textureSlot, uint32_t uvMapIndex);

    TevMode GetTevMode(uint32_t textureSlot);
    void SetTevMode(uint32_t textureSlot, TevMode mode);

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    // Properties
    MaterialLiteParams mLiteParams;
};

class MaterialEmbedded : public MaterialLite
{
public:
    DECLARE_ASSET(MaterialEmbedded, MaterialLite);

    static MaterialEmbedded* New(Material* src = nullptr);

    virtual const char* GetTypeName() override;

    MaterialEmbedded();

};