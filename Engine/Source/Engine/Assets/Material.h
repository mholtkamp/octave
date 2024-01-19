#pragma once

#include "Asset.h"
#include "EngineTypes.h"
#include "Datum.h"

#include "Texture.h"
#include "AssetRef.h"

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

class Material;
class MaterialInstance;
class Texture;

enum class ShaderParameterType
{
    Scalar,
    Vector,
    Texture,

    Count
};

struct ShaderParameter
{
    std::string mName;
    glm::vec4 mFloatValue = { }; // Holds scalar and vector values.
    TextureRef mTextureValue; // Only used when Texture param type.
    ShaderParameterType mType = ShaderParameterType::Count;
    uint32_t mOffset = 0; // Byte offset for uniforms, binding location for textures.
};

class Material : public Asset
{
public:

    DECLARE_ASSET(Material, Asset);

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    MaterialResource* GetResource();

    virtual bool IsBase() const;
    virtual bool IsInstance() const;
    virtual bool IsLite() const;

    std::vector<ShaderParameter>& GetParameters();

    void SetScalarParameter(const std::string& name, float value);
    void SetVectorParameter(const std::string& name, glm::vec4 value);
    void SetTextureParameter(const std::string& name, Texture* value);

    float GetScalarParameter(const std::string& name);
    glm::vec4 GetVectorParameter(const std::string& name);
    Texture* GetTextureParameter(const std::string& name);

    virtual BlendMode GetBlendMode() const;
    virtual float GetMaskCutoff() const;
    virtual int32_t GetSortPriority() const;
    virtual bool IsDepthTestDisabled() const;
    virtual bool ShouldApplyFog() const;
    virtual CullMode GetCullMode() const;

protected:

    std::vector<ShaderParameter> mParameters;

    // Graphics Resource
    MaterialResource mResource;
};

