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

#include "Assets/MaterialInterface.h"

#include "Graphics/GraphicsConstants.h"
#include "Graphics/GraphicsTypes.h"

class Texture;

class Material : public Asset, public MaterialInterface
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

    // Shader Parameters
    std::vector<ShaderParameter> mParameters;

    // Misc
    std::vector<uint8_t> mVertexCode;
    std::vector<uint8_t> mFragmentCode;

    bool mDirty[MAX_FRAMES] = {};

    // Graphics Resource
    MaterialResource mResource;
};