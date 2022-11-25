#include "Property.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Log.h"

Property::Property()
{
    
}

Property::Property(
    DatumType type,
    const std::string& name,
    void* owner,
    void* data,
    uint32_t count,
    DatumChangeHandlerFP changeHandler,
    int32_t extra,
    int32_t enumCount,
    const char** enumStrings) :
    Datum(type, owner, data, count, changeHandler)
{
    mName = name;
    mExtra = extra;
    mEnumCount = enumCount;
    mEnumStrings = enumStrings;
}

Property::Property(const Property& src) :
    Datum(src)
{
    mName = src.mName;
    mExtra = src.mExtra;
    mEnumCount = src.mEnumCount;
    mEnumStrings = src.mEnumStrings;

    if (mExternal && src.IsVector())
    {
        mVector = src.mVector;
        mMinCount = src.mMinCount;
        mMaxCount = src.mMaxCount;
    }
}

Property& Property::operator=(const Property& src)
{
    if (this != &src)
    {
        Datum::operator=(src);
    }

    return *this;
}

void Property::ReadStream(Stream& stream, bool external)
{
    Datum::ReadStream(stream, external);
    stream.ReadString(mName);
    mExtra = stream.ReadInt32();
}

void Property::WriteStream(Stream& stream) const
{
    Datum::WriteStream(stream);
    stream.WriteString(mName);
    stream.WriteInt32(mExtra);
}

uint32_t Property::GetSerializationSize() const
{
    return Datum::GetSerializationSize() + 
        GetStringSerializationSize(mName) + 
        sizeof(mExtra);
}

bool Property::IsProperty() const
{
    return true;
}

void Property::DeepCopy(const Datum& src, bool forceInternalStorage)
{
    Datum::DeepCopy(src, forceInternalStorage);

    if (src.IsProperty())
    {
        const Property& srcProp = (const Property&)src;
        mName = srcProp.mName;
        mExtra = srcProp.mExtra;
        mEnumCount = srcProp.mEnumCount;
        mEnumStrings = srcProp.mEnumStrings;

        if (mExternal && srcProp.IsVector())
        {
            mVector = srcProp.mVector;
            mMinCount = srcProp.mMinCount;
            mMaxCount = srcProp.mMaxCount;
        }
    }
}

// TODO: Replace with individual functions for better type safety.
void Property::PushBackVector(void* value)
{
    OCT_ASSERT(mExternal);
    OCT_ASSERT(mVector);

    if (mVector)
    {
        switch (mType)
        {
        case DatumType::Integer:
        {
            std::vector<int32_t>& vect = *((std::vector<int32_t>*) mVector);
            vect.push_back(value ? *((int32_t*) value) : 0);
            mData.i = vect.data();
            break;
        }
        case DatumType::Float:
        {
            std::vector<float>& vect = *((std::vector<float>*) mVector);
            vect.push_back(value ? *((float*)value) : 0.0f);
            mData.f = vect.data();
            break;
        }
        case DatumType::Bool:
        {
            // Bool not supported yet.
            OCT_ASSERT(0);
            break;
        }
        case DatumType::String:
        {
            std::vector<std::string>& vect = *((std::vector<std::string>*) mVector);
            vect.push_back(value ? *((std::string*)value) : std::string());
            mData.s = vect.data();
            break;
        }
        case DatumType::Vector2D:
        {
            std::vector<glm::vec2>& vect = *((std::vector<glm::vec2>*) mVector);
            vect.push_back(value ? *((glm::vec2*)value) : glm::vec2());
            mData.v2 = vect.data();
            break;
        }
        case DatumType::Vector:
        {
            std::vector<glm::vec3>& vect = *((std::vector<glm::vec3>*) mVector);
            vect.push_back(value ? *((glm::vec3*)value) : glm::vec3());
            mData.v3 = vect.data();
            break;
        }
        case DatumType::Color:
        {
            std::vector<glm::vec4>& vect = *((std::vector<glm::vec4>*) mVector);
            vect.push_back(value ? *((glm::vec4*)value) : glm::vec4());
            mData.v4 = vect.data();
            break;
        }
        case DatumType::Asset:
        {
            std::vector<AssetRef>& vect = *((std::vector<AssetRef>*) mVector);
            vect.push_back(value ? *((AssetRef*)value) : AssetRef());
            mData.as = vect.data();
            break;
        }
        case DatumType::Byte:
        {
            std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
            vect.push_back(value ? *((uint8_t*)value) : (uint8_t)0);
            mData.by = vect.data();
            break;
        }
        case DatumType::Table:
        {
            // Table not supported as vector.
            OCT_ASSERT(0);
            break;
        }
        case DatumType::Pointer:
        {
            // Pointer not supported as vector.
            OCT_ASSERT(0);
            break;
        }

        default: break;
        }

        mCount++;
    }
}

void Property::EraseVector(uint32_t index)
{
    OCT_ASSERT(mExternal);
    OCT_ASSERT(mVector);

    if (mVector)
    {
        switch (mType)
        {
        case DatumType::Integer:
        {
            std::vector<int32_t>& vect = *((std::vector<int32_t>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Float:
        {
            std::vector<float>& vect = *((std::vector<float>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Bool:
        {
            // Bool not supported as vector
            OCT_ASSERT(0);
            break;
        }
        case DatumType::String:
        {
            std::vector<std::string>& vect = *((std::vector<std::string>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Vector2D:
        {
            std::vector<glm::vec2>& vect = *((std::vector<glm::vec2>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Vector:
        {
            std::vector<glm::vec3>& vect = *((std::vector<glm::vec3>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Color:
        {
            std::vector<glm::vec4>& vect = *((std::vector<glm::vec4>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Asset:
        {
            std::vector<AssetRef>& vect = *((std::vector<AssetRef>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Byte:
        {
            std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Table:
        {
            // Table not supported as Vector
            OCT_ASSERT(0);
            break;
        }
        case DatumType::Pointer:
        {
            // Pointer not supported as Vector
            OCT_ASSERT(0);
            break;
        }

        default: break;
        }

        mCount--;
    }
}

void Property::ResizeVector(uint32_t count)
{
    OCT_ASSERT(mExternal);
    OCT_ASSERT(mVector);

    count = glm::clamp<uint32_t>(count, 0u, 255u);

    if (mVector)
    {
        switch (mType)
        {
        case DatumType::Integer:
        {
            std::vector<int32_t>& vect = *((std::vector<int32_t>*) mVector);
            vect.resize(count);
            mData.i = vect.data();
            break;
        }
        case DatumType::Float:
        {
            std::vector<float>& vect = *((std::vector<float>*) mVector);
            vect.resize(count);
            mData.f = vect.data();
            break;
        }
        case DatumType::Bool:
        {
            // Bool not supported yet.
            OCT_ASSERT(0);
            break;
        }
        case DatumType::String:
        {
            std::vector<std::string>& vect = *((std::vector<std::string>*) mVector);
            vect.resize(count);
            mData.s = vect.data();
            break;
        }
        case DatumType::Vector2D:
        {
            std::vector<glm::vec2>& vect = *((std::vector<glm::vec2>*) mVector);
            vect.resize(count);
            mData.v2 = vect.data();
            break;
        }
        case DatumType::Vector:
        {
            std::vector<glm::vec3>& vect = *((std::vector<glm::vec3>*) mVector);
            vect.resize(count);
            mData.v3 = vect.data();
            break;
        }
        case DatumType::Color:
        {
            std::vector<glm::vec4>& vect = *((std::vector<glm::vec4>*) mVector);
            vect.resize(count);
            mData.v4 = vect.data();
            break;
        }
        case DatumType::Asset:
        {
            std::vector<AssetRef>& vect = *((std::vector<AssetRef>*) mVector);
            vect.resize(count);
            mData.as = vect.data();
            break;
        }
        case DatumType::Byte:
        {
            std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
            vect.resize(count);
            mData.by = vect.data();
            break;
        }
        case DatumType::Table:
        {
            // Table not supported as Vector
            OCT_ASSERT(0);
            break;
        }
        case DatumType::Pointer:
        {
            // Pointer not supported as Vector
            OCT_ASSERT(0);
            break;
        }

        default: break;
        }

        mCount = count;
    }
}

Property& Property::MakeVector(uint8_t minCount, uint8_t maxCount)
{
    // Vector properties should only be used with external data.
    OCT_ASSERT(mData.vp);
    OCT_ASSERT(mExternal);
    OCT_ASSERT(!mVector);

    mVector = mData.vp;
    mMinCount = minCount;
    mMaxCount = maxCount;

    switch (mType)
    {
    case DatumType::Integer: 
    {
        std::vector<int32_t>& vect = *((std::vector<int32_t>*) mVector);
        mData.i = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Float:
    {
        std::vector<float>& vect = *((std::vector<float>*) mVector);
        mData.f = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Bool:
    {
        // Bool is not supported because std::vector<bool> is dumb.
        // Maybe use byte type instead? Sorry.
        OCT_ASSERT(0);
        break;
    }
    case DatumType::String:
    {
        std::vector<std::string>& vect = *((std::vector<std::string>*) mVector);
        mData.s = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Vector2D:
    {
        std::vector<glm::vec2>& vect = *((std::vector<glm::vec2>*) mVector);
        mData.v2 = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Vector:
    {
        std::vector<glm::vec3>& vect = *((std::vector<glm::vec3>*) mVector);
        mData.v3 = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Color:
    {
        std::vector<glm::vec4>& vect = *((std::vector<glm::vec4>*) mVector);
        mData.v4 = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Asset:
    {
        std::vector<AssetRef>& vect = *((std::vector<AssetRef>*) mVector);
        mData.as = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Byte:
    {
        std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
        mData.by = vect.data();
        mCount = (uint8_t) vect.size();
        break;
    }
    case DatumType::Table:
    {
        // Table not supported as vector
        OCT_ASSERT(0);
        break;
    }
    case DatumType::Pointer:
    {
        // Pointer not supported as vector
        OCT_ASSERT(0);
        break;
    }

    default: break;
    }

    return *this;
}

bool Property::IsVector() const
{
    return (mVector != nullptr);
}

bool Property::IsArray() const
{
    return (IsVector() || mCount > 1);
}

void Property::Reset()
{
    Datum::Reset();
    mName = "";
    mExtra = 0;
    mEnumCount = 0;
    mEnumStrings = nullptr;
    mVector = nullptr;
    mMinCount = 0;
    mMaxCount = 255;
}
