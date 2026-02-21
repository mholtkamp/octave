#include "NodeGraph/GraphTypes.h"

bool AreGraphPinTypesCompatible(DatumType outputType, DatumType inputType)
{
    if (outputType == inputType)
    {
        return true;
    }

    // Float <-> Integer
    if ((outputType == DatumType::Float && inputType == DatumType::Integer) ||
        (outputType == DatumType::Integer && inputType == DatumType::Float))
    {
        return true;
    }

    // Vector <-> Color (vec3 <-> vec4)
    if ((outputType == DatumType::Vector && inputType == DatumType::Color) ||
        (outputType == DatumType::Color && inputType == DatumType::Vector))
    {
        return true;
    }

    // Float -> Vector2D/Vector/Color (scalar expansion)
    if (outputType == DatumType::Float &&
        (inputType == DatumType::Vector2D || inputType == DatumType::Vector || inputType == DatumType::Color))
    {
        return true;
    }

    return false;
}

const char* GetDatumTypeName(DatumType type)
{
    switch (type)
    {
    case DatumType::Integer:  return "Integer";
    case DatumType::Float:    return "Float";
    case DatumType::Bool:     return "Bool";
    case DatumType::String:   return "String";
    case DatumType::Vector2D: return "Vector2D";
    case DatumType::Vector:   return "Vector";
    case DatumType::Color:    return "Color";
    case DatumType::Asset:    return "Asset";
    default:                  return "Unknown";
    }
}

glm::vec4 GetDatumTypeColor(DatumType type)
{
    switch (type)
    {
    case DatumType::Integer:  return glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
    case DatumType::Float:    return glm::vec4(0.5f, 0.9f, 0.5f, 1.0f);
    case DatumType::Bool:     return glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
    case DatumType::String:   return glm::vec4(0.9f, 0.2f, 0.9f, 1.0f);
    case DatumType::Vector2D: return glm::vec4(0.9f, 0.8f, 0.1f, 1.0f);
    case DatumType::Vector:   return glm::vec4(0.9f, 0.9f, 0.1f, 1.0f);
    case DatumType::Color:    return glm::vec4(0.1f, 0.5f, 0.9f, 1.0f);
    case DatumType::Asset:    return glm::vec4(0.3f, 0.3f, 0.9f, 1.0f);
    default:                  return glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    }
}
