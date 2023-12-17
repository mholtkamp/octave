#include "MaterialInterface.h"


void MaterialInterface::SetScalarParameter(const std::string& name, float value)
{
    std::vector<ShaderParameter>& params = GetParameters();

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

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

void MaterialInterface::SetVectorParameter(const std::string& name, glm::vec4 value)
{
    std::vector<ShaderParameter>& params = GetParameters();

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

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

void MaterialInterface::SetTextureParameter(const std::string& name, Texture* value)
{
    std::vector<ShaderParameter>& params = GetParameters();

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

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


float MaterialInterface::GetScalarParameter(const std::string& name)
{
    float ret = 0.0f;

    std::vector<ShaderParameter>& params = GetParameters();

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

        if (param.mName == name &&
            param.mType == ShaderParameterType::Scalar)
        {
            ret = param.mFloatValue.x;
        }
    }

    return ret;
}

glm::vec4 MaterialInterface::GetVectorParameter(const std::string& name)
{
    glm::vec4 ret = {};

    std::vector<ShaderParameter>& params = GetParameters();

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

        if (param.mName == name &&
            param.mType == ShaderParameterType::Vector)
        {
            ret = param.mFloatValue;
        }
    }

    return ret;
}

Texture* MaterialInterface::GetTextureParameter(const std::string& name)
{
    Texture* ret = nullptr;

    std::vector<ShaderParameter>& params = GetParameters();

    for (uint32_t i = 0; i < params.size(); ++i)
    {
        ShaderParameter& param = params[i];

        if (param.mName == name &&
            param.mType == ShaderParameterType::Texture)
        {
            ret = param.mTextureValue.Get<Texture>();
        }
    }

    return ret;
}

