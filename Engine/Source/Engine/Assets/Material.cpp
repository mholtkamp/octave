#include "Material.h"

void Material::MarkDirty()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mDirty[i] = true;
    }
}

void Material::ClearDirty(uint32_t frameIndex)
{
    mDirty[frameIndex] = false;
}

bool Material::IsDirty(uint32_t frameIndex)
{
    return mDirty[frameIndex];
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
                MarkDirty();
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
                MarkDirty();
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
                MarkDirty();
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

// These should all be implemented by Material / MaterialInstance / MaterialLite
BlendMode Material::GetBlendMode() const { return BlendMode::Count; }
float Material::GetMaskCutoff() const { return 0.5f; }
int32_t Material::GetSortPriority() const { return 0; }
bool Material::IsDepthTestDisabled() const { return false; }
bool Material::ShouldApplyFog() const { return true; }
CullMode Material::GetCullMode() const { return CullMode::Back; }
