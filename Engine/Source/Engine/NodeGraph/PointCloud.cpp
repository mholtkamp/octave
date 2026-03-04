#include "NodeGraph/PointCloud.h"
#include "Log.h"

static const std::string sEmptyString;

uint32_t PointCloud::GetNumPoints() const
{
    return (uint32_t)mPositions.size();
}

void PointCloud::Resize(uint32_t count)
{
    mPositions.resize(count, glm::vec3(0.0f));

    for (auto& pair : mAttributes)
    {
        AttribArray& attr = pair.second;
        switch (attr.mType)
        {
        case DatumType::Float: attr.mFloats.resize(count, 0.0f); break;
        case DatumType::Integer: attr.mInts.resize(count, 0); break;
        case DatumType::Vector: attr.mVectors.resize(count, glm::vec3(0.0f)); break;
        case DatumType::Color: attr.mColors.resize(count, glm::vec4(1.0f)); break;
        case DatumType::String: attr.mStrings.resize(count); break;
        default: break;
        }
    }
}

void PointCloud::AddPoint(const glm::vec3& pos)
{
    mPositions.push_back(pos);

    for (auto& pair : mAttributes)
    {
        AttribArray& attr = pair.second;
        switch (attr.mType)
        {
        case DatumType::Float: attr.mFloats.push_back(0.0f); break;
        case DatumType::Integer: attr.mInts.push_back(0); break;
        case DatumType::Vector: attr.mVectors.push_back(glm::vec3(0.0f)); break;
        case DatumType::Color: attr.mColors.push_back(glm::vec4(1.0f)); break;
        case DatumType::String: attr.mStrings.push_back(""); break;
        default: break;
        }
    }
}

void PointCloud::RemovePoint(uint32_t index)
{
    if (index >= mPositions.size())
        return;

    mPositions.erase(mPositions.begin() + index);

    for (auto& pair : mAttributes)
    {
        AttribArray& attr = pair.second;
        switch (attr.mType)
        {
        case DatumType::Float: attr.mFloats.erase(attr.mFloats.begin() + index); break;
        case DatumType::Integer: attr.mInts.erase(attr.mInts.begin() + index); break;
        case DatumType::Vector: attr.mVectors.erase(attr.mVectors.begin() + index); break;
        case DatumType::Color: attr.mColors.erase(attr.mColors.begin() + index); break;
        case DatumType::String: attr.mStrings.erase(attr.mStrings.begin() + index); break;
        default: break;
        }
    }
}

void PointCloud::Clear()
{
    mPositions.clear();
    mAttributes.clear();
}

const glm::vec3& PointCloud::GetPosition(uint32_t i) const
{
    return mPositions[i];
}

void PointCloud::SetPosition(uint32_t i, const glm::vec3& p)
{
    mPositions[i] = p;
}

const std::vector<glm::vec3>& PointCloud::GetPositions() const
{
    return mPositions;
}

void PointCloud::AddAttribute(const std::string& name, DatumType type)
{
    if (mAttributes.find(name) != mAttributes.end())
        return;

    AttribArray attr;
    attr.mType = type;
    uint32_t count = (uint32_t)mPositions.size();

    switch (type)
    {
    case DatumType::Float: attr.mFloats.resize(count, 0.0f); break;
    case DatumType::Integer: attr.mInts.resize(count, 0); break;
    case DatumType::Vector: attr.mVectors.resize(count, glm::vec3(0.0f)); break;
    case DatumType::Color: attr.mColors.resize(count, glm::vec4(1.0f)); break;
    case DatumType::String: attr.mStrings.resize(count); break;
    default:
        LogWarning("PointCloud::AddAttribute - unsupported type");
        return;
    }

    mAttributes[name] = std::move(attr);
}

bool PointCloud::HasAttribute(const std::string& name) const
{
    return mAttributes.find(name) != mAttributes.end();
}

DatumType PointCloud::GetAttributeType(const std::string& name) const
{
    auto it = mAttributes.find(name);
    if (it != mAttributes.end())
        return it->second.mType;
    return DatumType::Count;
}

float PointCloud::GetAttributeFloat(uint32_t pointIndex, const std::string& name) const
{
    auto it = mAttributes.find(name);
    if (it != mAttributes.end() && it->second.mType == DatumType::Float && pointIndex < it->second.mFloats.size())
        return it->second.mFloats[pointIndex];
    return 0.0f;
}

int32_t PointCloud::GetAttributeInt(uint32_t pointIndex, const std::string& name) const
{
    auto it = mAttributes.find(name);
    if (it != mAttributes.end() && it->second.mType == DatumType::Integer && pointIndex < it->second.mInts.size())
        return it->second.mInts[pointIndex];
    return 0;
}

glm::vec3 PointCloud::GetAttributeVector(uint32_t pointIndex, const std::string& name) const
{
    auto it = mAttributes.find(name);
    if (it != mAttributes.end() && it->second.mType == DatumType::Vector && pointIndex < it->second.mVectors.size())
        return it->second.mVectors[pointIndex];
    return glm::vec3(0.0f);
}

glm::vec4 PointCloud::GetAttributeColor(uint32_t pointIndex, const std::string& name) const
{
    auto it = mAttributes.find(name);
    if (it != mAttributes.end() && it->second.mType == DatumType::Color && pointIndex < it->second.mColors.size())
        return it->second.mColors[pointIndex];
    return glm::vec4(1.0f);
}

const std::string& PointCloud::GetAttributeString(uint32_t pointIndex, const std::string& name) const
{
    auto it = mAttributes.find(name);
    if (it != mAttributes.end() && it->second.mType == DatumType::String && pointIndex < it->second.mStrings.size())
        return it->second.mStrings[pointIndex];
    return sEmptyString;
}

void PointCloud::SetAttributeFloat(uint32_t pointIndex, const std::string& name, float value)
{
    auto it = mAttributes.find(name);
    if (it == mAttributes.end())
    {
        AddAttribute(name, DatumType::Float);
        it = mAttributes.find(name);
    }

    if (it->second.mType == DatumType::Float && pointIndex < it->second.mFloats.size())
        it->second.mFloats[pointIndex] = value;
}

void PointCloud::SetAttributeInt(uint32_t pointIndex, const std::string& name, int32_t value)
{
    auto it = mAttributes.find(name);
    if (it == mAttributes.end())
    {
        AddAttribute(name, DatumType::Integer);
        it = mAttributes.find(name);
    }

    if (it->second.mType == DatumType::Integer && pointIndex < it->second.mInts.size())
        it->second.mInts[pointIndex] = value;
}

void PointCloud::SetAttributeVector(uint32_t pointIndex, const std::string& name, const glm::vec3& value)
{
    auto it = mAttributes.find(name);
    if (it == mAttributes.end())
    {
        AddAttribute(name, DatumType::Vector);
        it = mAttributes.find(name);
    }

    if (it->second.mType == DatumType::Vector && pointIndex < it->second.mVectors.size())
        it->second.mVectors[pointIndex] = value;
}

void PointCloud::SetAttributeColor(uint32_t pointIndex, const std::string& name, const glm::vec4& value)
{
    auto it = mAttributes.find(name);
    if (it == mAttributes.end())
    {
        AddAttribute(name, DatumType::Color);
        it = mAttributes.find(name);
    }

    if (it->second.mType == DatumType::Color && pointIndex < it->second.mColors.size())
        it->second.mColors[pointIndex] = value;
}

void PointCloud::SetAttributeString(uint32_t pointIndex, const std::string& name, const std::string& value)
{
    auto it = mAttributes.find(name);
    if (it == mAttributes.end())
    {
        AddAttribute(name, DatumType::String);
        it = mAttributes.find(name);
    }

    if (it->second.mType == DatumType::String && pointIndex < it->second.mStrings.size())
        it->second.mStrings[pointIndex] = value;
}

float PointCloud::GetPScale(uint32_t i) const
{
    float val = GetAttributeFloat(i, "pscale");
    if (!HasAttribute("pscale"))
        return 1.0f;
    return val;
}

glm::vec4 PointCloud::GetCd(uint32_t i) const
{
    if (!HasAttribute("Cd"))
        return glm::vec4(1.0f);
    return GetAttributeColor(i, "Cd");
}

glm::vec3 PointCloud::GetNormal(uint32_t i) const
{
    if (!HasAttribute("N"))
        return glm::vec3(0.0f, 1.0f, 0.0f);
    return GetAttributeVector(i, "N");
}

glm::vec3 PointCloud::GetVelocity(uint32_t i) const
{
    if (!HasAttribute("v"))
        return glm::vec3(0.0f);
    return GetAttributeVector(i, "v");
}

void PointCloud::Merge(const PointCloud& other)
{
    uint32_t oldCount = GetNumPoints();
    uint32_t addCount = other.GetNumPoints();

    // Append positions
    mPositions.insert(mPositions.end(), other.mPositions.begin(), other.mPositions.end());

    // Merge attributes
    for (const auto& pair : other.mAttributes)
    {
        const std::string& name = pair.first;
        const AttribArray& srcAttr = pair.second;

        if (!HasAttribute(name))
        {
            AddAttribute(name, srcAttr.mType);
            // Resize to include old points (filled with defaults)
            auto it = mAttributes.find(name);
            switch (srcAttr.mType)
            {
            case DatumType::Float: it->second.mFloats.resize(oldCount + addCount, 0.0f); break;
            case DatumType::Integer: it->second.mInts.resize(oldCount + addCount, 0); break;
            case DatumType::Vector: it->second.mVectors.resize(oldCount + addCount, glm::vec3(0.0f)); break;
            case DatumType::Color: it->second.mColors.resize(oldCount + addCount, glm::vec4(1.0f)); break;
            case DatumType::String: it->second.mStrings.resize(oldCount + addCount); break;
            default: break;
            }
        }
        else
        {
            // Ensure existing attribute has space for new points
            auto it = mAttributes.find(name);
            switch (it->second.mType)
            {
            case DatumType::Float: it->second.mFloats.resize(oldCount + addCount, 0.0f); break;
            case DatumType::Integer: it->second.mInts.resize(oldCount + addCount, 0); break;
            case DatumType::Vector: it->second.mVectors.resize(oldCount + addCount, glm::vec3(0.0f)); break;
            case DatumType::Color: it->second.mColors.resize(oldCount + addCount, glm::vec4(1.0f)); break;
            case DatumType::String: it->second.mStrings.resize(oldCount + addCount); break;
            default: break;
            }
        }

        // Copy source attribute data
        auto it = mAttributes.find(name);
        if (it != mAttributes.end() && it->second.mType == srcAttr.mType)
        {
            switch (srcAttr.mType)
            {
            case DatumType::Float:
                for (uint32_t i = 0; i < addCount; ++i)
                    it->second.mFloats[oldCount + i] = srcAttr.mFloats[i];
                break;
            case DatumType::Integer:
                for (uint32_t i = 0; i < addCount; ++i)
                    it->second.mInts[oldCount + i] = srcAttr.mInts[i];
                break;
            case DatumType::Vector:
                for (uint32_t i = 0; i < addCount; ++i)
                    it->second.mVectors[oldCount + i] = srcAttr.mVectors[i];
                break;
            case DatumType::Color:
                for (uint32_t i = 0; i < addCount; ++i)
                    it->second.mColors[oldCount + i] = srcAttr.mColors[i];
                break;
            case DatumType::String:
                for (uint32_t i = 0; i < addCount; ++i)
                    it->second.mStrings[oldCount + i] = srcAttr.mStrings[i];
                break;
            default: break;
            }
        }
    }

    // Ensure all existing attributes also cover the new points
    for (auto& pair : mAttributes)
    {
        AttribArray& attr = pair.second;
        uint32_t totalCount = oldCount + addCount;
        switch (attr.mType)
        {
        case DatumType::Float: if (attr.mFloats.size() < totalCount) attr.mFloats.resize(totalCount, 0.0f); break;
        case DatumType::Integer: if (attr.mInts.size() < totalCount) attr.mInts.resize(totalCount, 0); break;
        case DatumType::Vector: if (attr.mVectors.size() < totalCount) attr.mVectors.resize(totalCount, glm::vec3(0.0f)); break;
        case DatumType::Color: if (attr.mColors.size() < totalCount) attr.mColors.resize(totalCount, glm::vec4(1.0f)); break;
        case DatumType::String: if (attr.mStrings.size() < totalCount) attr.mStrings.resize(totalCount); break;
        default: break;
        }
    }
}

PointCloud* PointCloud::Clone() const
{
    PointCloud* clone = new PointCloud();
    clone->mPositions = mPositions;
    clone->mAttributes = mAttributes;
    return clone;
}
