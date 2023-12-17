#pragma once

#include "EngineTypes.h"
#include "Datum.h"

#include "Texture.h"
#include "AssetRef.h"

class Material;
class MaterialInstance;
class Texture;

#if 0
struct ScalarParameter
{
    std::string mName;
    float mValue;
};

struct VectorParameter
{
    std::string mName;
    glm::vec4 mValue;
};

struct TextureParameter
{
    std::string mName;
    TextureRef mValue;
};
#endif

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
    glm::vec4 mFloatValue; // Holds scalar and vector values.
    TextureRef mTextureValue; // Only used when Texture param type.
    ShaderParameterType mType = ShaderParameterType::Count;
    //uint32_t mOffset = 0;
};

class MaterialInterface
{
public:

    virtual void MarkDirty() = 0;
    virtual void ClearDirty(uint32_t frameIndex) = 0;
    virtual bool IsDirty(uint32_t frameIndex) = 0;
    virtual Material* AsBase() = 0;
    virtual MaterialInstance* AsInstance() = 0;
    virtual bool IsBase() const = 0;
    virtual bool IsInstance() const = 0;
    virtual std::vector<ShaderParameter>& GetParameters() = 0;

public:

    void SetScalarParameter(const std::string& name, float value);
    void SetVectorParameter(const std::string& name, glm::vec4 value);
    void SetTextureParameter(const std::string& name, Texture* value);

    float GetScalarParameter(const std::string& name);
    glm::vec4 GetVectorParameter(const std::string& name);
    Texture* GetTextureParameter(const std::string& name);
};

