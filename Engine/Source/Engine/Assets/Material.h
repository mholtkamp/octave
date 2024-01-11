#pragma once

#include "Maths.h"
#include <string>
#include <vector>
#include <map>

#include "Enums.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Constants.h"
#include "EngineTypes.h"
#include "Vertex.h"

#include "Graphics/GraphicsConstants.h"
#include "Graphics/GraphicsTypes.h"

class Texture;

enum class TevMode : uint32_t
{
    Replace,
    Modulate,
    Decal,
    Add,
    SignedAdd,
    Subtract,
    Interpolate,
    Pass,

    Count
};

struct MaterialParams
{
    ShadingModel mShadingModel = ShadingModel::Lit;
    BlendMode mBlendMode = BlendMode::Opaque;
    VertexColorMode mVertexColorMode = VertexColorMode::Modulate;
    TextureRef mTextures[MATERIAL_MAX_TEXTURES];
    uint8_t mUvMaps[MATERIAL_MAX_TEXTURES] = { };
    TevMode mTevModes[MATERIAL_MAX_TEXTURES] = { TevMode::Replace, TevMode::Pass, TevMode::Pass, TevMode::Pass };
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

class Material : public Asset
{
public:

    DECLARE_ASSET(Material, Asset);

    Material();
    ~Material();

    MaterialResource* GetResource();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    virtual bool IsMaterialInstance() const;

    const MaterialParams& GetParams() const;
    void SetParams(const MaterialParams& params);

    void SetTexture(TextureSlot slot, Texture* texture);
    Texture* GetTexture(TextureSlot slot);

    ShadingModel GetShadingModel() const;
    void SetShadingModel(ShadingModel shadingModel);

    BlendMode GetBlendMode() const;
    void SetBlendMode(BlendMode blendMode);

    VertexColorMode GetVertexColorMode() const;
    void SetVertexColorMode(VertexColorMode mode);

    glm::vec2 GetUvOffset(uint32_t uvIndex = 0) const;
    void SetUvOffset(glm::vec2 offset, uint32_t uvIndex = 0);

    glm::vec2 GetUvScale(uint32_t uvIndex = 0) const;
    void SetUvScale(glm::vec2 scale, uint32_t uvIndex = 0);

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

    float GetMaskCutoff() const;
    void SetMaskCutoff(float cutoff);

    float GetShininess() const;
    void SetShininess(float shininess);

    int32_t GetSortPriority() const;
    void SetSortPriority(int32_t priority);

    bool IsDepthTestDisabled() const;
    void SetDepthTestDisabled(bool depthTest);

    bool IsFresnelEnabled() const;
    void SetFresnelEnabled(bool enable);

    bool ShouldApplyFog() const;
    void SetApplyFog(bool applyFog);

    CullMode GetCullMode() const;
    void SetCullMode(CullMode cullMode);

    uint32_t GetUvMap(uint32_t textureSlot);
    void SetUvMap(uint32_t textureSlot, uint32_t uvMapIndex);

    TevMode GetTevMode(uint32_t textureSlot);
    void SetTevMode(uint32_t textureSlot, TevMode mode);

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    // Properties
    MaterialParams mParams;

    // Graphics Resource
    MaterialResource mResource;
};