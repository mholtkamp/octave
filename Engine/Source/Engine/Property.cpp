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

void Property::Reset()
{
    Datum::Reset();
    mName = "";
    mExtra = 0;
    mEnumCount = 0;
    mEnumStrings = nullptr;
}
