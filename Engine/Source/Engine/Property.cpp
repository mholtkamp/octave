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
    }
}

// TODO: Replace with individual functions for better type safety.
void Property::PushBackVector(void* value)
{
    assert(mExternal);
    assert(mVector);

    if (mVector)
    {
        switch (mType)
        {
        case DatumType::Integer:
        {
            std::vector<int32_t>& vect = *((std::vector<int32_t>*) mVector);
            vect.push_back(*((int32_t*) value));
            break;
        }
        case DatumType::Float:
        {
            std::vector<float>& vect = *((std::vector<float>*) mVector);
            vect.push_back(*((float*)value));
            break;
        }
        case DatumType::Bool:
        {
            break;
        }
        case DatumType::String:
        {
            std::vector<std::string>& vect = *((std::vector<std::string>*) mVector);
            vect.push_back(*((std::string*)value));
            break;
        }
        case DatumType::Vector2D:
        {
            std::vector<glm::vec2>& vect = *((std::vector<glm::vec2>*) mVector);
            vect.push_back(*((glm::vec2*)value));
            break;
        }
        case DatumType::Vector:
        {
            std::vector<glm::vec3>& vect = *((std::vector<glm::vec3>*) mVector);
            vect.push_back(*((glm::vec3*)value));
            break;
        }
        case DatumType::Color:
        {
            std::vector<glm::vec4>& vect = *((std::vector<glm::vec4>*) mVector);
            vect.push_back(*((glm::vec4*)value));
            break;
        }
        case DatumType::Asset:
        {
            std::vector<AssetRef>& vect = *((std::vector<AssetRef>*) mVector);
            vect.push_back(*((AssetRef*)value));
            break;
        }
        case DatumType::Enum:
        {
            std::vector<uint32_t>& vect = *((std::vector<uint32_t>*) mVector);
            vect.push_back(*((uint32_t*)value));
            break;
        }
        case DatumType::Byte:
        {
            std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
            vect.push_back(*((uint8_t*)value));
            break;
        }
        }
    }
}

void Property::EraseVector(uint32_t index)
{
    assert(mExternal);
    assert(mVector);

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
        case DatumType::Enum:
        {
            std::vector<uint32_t>& vect = *((std::vector<uint32_t>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Byte:
        {
            std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        }
    }
}

Property& Property::MakeVector(uint8_t minCount, uint8_t maxCount)
{
    // Vector properties should only be used with external data.
    assert(mData.vp);
    assert(mExternal);
    assert(!mVector);

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
        assert(0);
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
    case DatumType::Enum:
    {
        std::vector<uint32_t>& vect = *((std::vector<uint32_t>*) mVector);
        mData.e = vect.data();
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
    }

    return *this;
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
