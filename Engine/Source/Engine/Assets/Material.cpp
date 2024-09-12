#include "Material.h"

#include "MaterialLite.h"
#include "MaterialBase.h"
#include "MaterialInstance.h"

FORCE_LINK_DEF(Material);
DEFINE_ASSET(Material);

void Material::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    uint32_t numParams = stream.ReadUint32();
    mParameters.resize(numParams);
    for (uint32_t i = 0; i < numParams; ++i)
    {
        ShaderParameter& param = mParameters[i];
        stream.ReadString(param.mName);
        param.mType = (ShaderParameterType)stream.ReadUint8();
        param.mOffset = stream.ReadUint32();

        switch (param.mType)
        {
        case ShaderParameterType::Scalar:
            param.mFloatValue.x = stream.ReadFloat();
            break;
        case ShaderParameterType::Vector:
            param.mFloatValue = stream.ReadVec4();
            break;
        case ShaderParameterType::Texture:
            stream.ReadAsset(param.mTextureValue);
            break;
        default:
            LogError("Invalid shader parameter type serialized.");
            OCT_ASSERT(0);
            break;
        }
    }
}

void Material::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteUint32((uint32_t)mParameters.size());

    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        const ShaderParameter& param = mParameters[i];
        stream.WriteString(param.mName);
        stream.WriteUint8((uint8_t)param.mType);
        stream.WriteUint32(param.mOffset);

        switch (param.mType)
        {
        case ShaderParameterType::Scalar:
            stream.WriteFloat(param.mFloatValue.x);
            break;
        case ShaderParameterType::Vector:
            stream.WriteVec4(param.mFloatValue);
            break;
        case ShaderParameterType::Texture:
            stream.WriteAsset(param.mTextureValue);
            break;
        default:
            LogError("Invalid shader parameter type serialized.");
            OCT_ASSERT(0);
            break;
        }
    }
}

void Material::Create()
{
    Asset::Create();
}

void Material::Destroy()
{
    Asset::Destroy();
}

void Material::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);
}

void Material::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
}

glm::vec4 Material::GetTypeColor()
{
    return glm::vec4(0.2f, 1.0f, 0.25f, 1.0f);
}

const char* Material::GetTypeName()
{
    return "Material";
}

MaterialResource* Material::GetResource()
{
    return &mResource;
}

bool Material::IsBase() const
{
    return false;
}

bool Material::IsInstance() const
{
    return false;
}

bool Material::IsLite() const
{
    return false;
}

std::vector<ShaderParameter>& Material::GetParameters()
{
    return mParameters;
}

void Material::SetScalarParameter(const std::string& name, float value)
{
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        ShaderParameter& param = mParameters[i];

        if (param.mName == name)
        {
            if (param.mType == ShaderParameterType::Scalar)
            {
                param.mFloatValue.x = value;
            }
            else
            {
                LogError("Parameter Type mismatch.");
            }
            break;
        }
    }
}

void Material::SetVectorParameter(const std::string& name, glm::vec4 value)
{
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        ShaderParameter& param = mParameters[i];

        if (param.mName == name)
        {
            if (param.mType == ShaderParameterType::Vector)
            {
                param.mFloatValue = value;
            }
            else
            {
                LogError("Parameter Type mismatch.");
            }
            break;
        }
    }
}

void Material::SetTextureParameter(const std::string& name, Texture* value)
{
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        ShaderParameter& param = mParameters[i];

        if (param.mName == name)
        {
            if (param.mType == ShaderParameterType::Texture)
            {
                param.mTextureValue = value;
            }
            else
            {
                LogError("Parameter Type mismatch.");
            }
            break;
        }
    }
}


float Material::GetScalarParameter(const std::string& name)
{
    float ret = 0.0f;

    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        ShaderParameter& param = mParameters[i];

        if (param.mName == name &&
            param.mType == ShaderParameterType::Scalar)
        {
            ret = param.mFloatValue.x;
        }
    }

    return ret;
}

glm::vec4 Material::GetVectorParameter(const std::string& name)
{
    glm::vec4 ret = {};

    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        ShaderParameter& param = mParameters[i];

        if (param.mName == name &&
            param.mType == ShaderParameterType::Vector)
        {
            ret = param.mFloatValue;
        }
    }

    return ret;
}

Texture* Material::GetTextureParameter(const std::string& name)
{
    Texture* ret = nullptr;

    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        ShaderParameter& param = mParameters[i];

        if (param.mName == name &&
            param.mType == ShaderParameterType::Texture)
        {
            ret = param.mTextureValue.Get<Texture>();
        }
    }

    return ret;
}

void Material::WriteShaderUniformParams(uint8_t* outData, uint32_t& outSize)
{
    uint32_t size = 0;

    // Always write vectors first because of alignment issues
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        if (mParameters[i].mType == ShaderParameterType::Vector)
        {
            memcpy(outData + size, &mParameters[i].mFloatValue, 16);
            size += 16;
        }
    }

    // Then write the scalar params
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        if (mParameters[i].mType == ShaderParameterType::Scalar)
        {
            memcpy(outData + size, &mParameters[i].mFloatValue, 4);
            size += 4;
        }
    }

    outSize = size;
}

// These should all be implemented by Material / MaterialInstance / MaterialLite
BlendMode Material::GetBlendMode() const { return BlendMode::Count; }
float Material::GetMaskCutoff() const { return 0.5f; }
int32_t Material::GetSortPriority() const { return 0; }
bool Material::IsDepthTestDisabled() const { return false; }
bool Material::ShouldApplyFog() const { return true; }
CullMode Material::GetCullMode() const { return CullMode::Back; }


MaterialLite* Material::AsLite(Material* material)
{
    MaterialLite* liteMat = nullptr;

    if (material != nullptr)
    {
        TypeId typeId = material->GetType();

        if (typeId == MaterialLite::GetStaticType())
        {
            liteMat = (MaterialLite*) material;
        }
        else if (typeId == MaterialBase::GetStaticType())
        {
            MaterialBase* baseMat = (MaterialBase*)material;
            liteMat = baseMat->GetLiteFallback();
        }
        else if (typeId == MaterialInstance::GetStaticType())
        {
            MaterialInstance* instMat = (MaterialInstance*)material;
            MaterialBase* baseMat = instMat->GetBaseMaterial();
            if (baseMat != nullptr)
            {
                liteMat = baseMat->GetLiteFallback();
            }
        }
    }

    return liteMat;
}

void Material::OverwriteShaderParameters(std::vector<ShaderParameter>& dst, const std::vector<ShaderParameter>& src)
{
    for (uint32_t d = 0; d < dst.size(); ++d)
    {
        ShaderParameter& dstParam = dst[d];

        for (uint32_t s = 0; s < src.size(); ++s)
        {
            const ShaderParameter& srcParam = src[s];

            if (srcParam.mName == dstParam.mName &&
                srcParam.mType == dstParam.mType)
            {
                dstParam = srcParam;
                break;
            }
        }

    }
}
