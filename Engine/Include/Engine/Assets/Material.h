#pragma once

#include <glm/glm.hpp>
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

struct MaterialParams
{
    ShadingModel mShadingModel = ShadingModel::Lit;
    BlendMode mBlendMode = BlendMode::Opaque;
    TextureRef mTextures[MATERIAL_MAX_TEXTURES];
    glm::vec2 mUvOffset = { 0.0f, 0.0f };
    glm::vec2 mUvScale = { 1.0f, 1.0f };
    glm::vec4 mColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 mFresnelColor = { 1.0f, 0.0f, 0.0f, 0.0f };
    float mFresnelPower = 1.0f;
    float mSpecular = 0.0f;
    uint32_t mToonSteps = 2;
    float mOpacity = 1.0f;
    float mMaskCutoff = 0.5f;
    float mShininess = 32.0f;
    int32_t mSortPriority = 0;
    bool mDisableDepthTest = false;
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
    virtual void Import(const std::string& path) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    virtual bool IsMaterialInstance() const;

    void MarkDirty();
    void ClearDirty(uint32_t frameIndex);
    bool IsDirty(uint32_t frameIndex);

    const MaterialParams& GetParams() const;
    void SetParams(const MaterialParams& params);

    void SetTexture(TextureSlot slot, Texture* texture);
    Texture* GetTexture(TextureSlot slot);

    ShadingModel GetShadingModel() const;
    void SetShadingModel(ShadingModel shadingModel);

    BlendMode GetBlendMode() const;
    void SetBlendMode(BlendMode blendMode);

    glm::vec2 GetUvOffset() const;
    void SetUvOffset(glm::vec2 offset);

    glm::vec2 GetUvScale() const;
    void SetUvScale(glm::vec2 scale);

    glm::vec4 GetColor() const;
    void SetColor(const glm::vec4& color);

    glm::vec4 GetFresnelColor() const;
    void SetFresnelColor(const glm::vec4& color);

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

protected:

    // Properties
    MaterialParams mParams;

    bool mDirty[MAX_FRAMES] = {};

    // Graphics Resource
    MaterialResource mResource;
};