#include "TableDatum.h"
#include "Utilities.h"
#include "Log.h"

TableDatum::TableDatum()
{

}

TableDatum::~TableDatum()
{

}

TableDatum::TableDatum(const char* key, int32_t value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, float value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, bool value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, const char* value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, const std::string& value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, glm::vec2 value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, glm::vec3 value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, glm::vec4 value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, Asset* value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, uint32_t value)
{
    PushBack((int32_t)value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, Object* value)
{
    PushBack(value);
    SetStringKey(key);
}

TableDatum::TableDatum(const char* key, const ScriptFunc& value)
{
    PushBack(value);
    SetStringKey(key);
}

//TableDatum::TableDatum(const char* key, const TableDatum& value)
//{
//    PushBackTableDatum(value);
//    SetStringKey(key);
//}

TableDatum::TableDatum(int32_t key, int32_t value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, float value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, bool value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, const char* value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, const std::string& value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, glm::vec2 value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, glm::vec3 value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, glm::vec4 value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, Asset* value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, uint32_t value)
{
    PushBack((int32_t)value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, Object* value)
{
    PushBack(value);
    SetIntegerKey(key);
}

TableDatum::TableDatum(int32_t key, const ScriptFunc& value)
{
    PushBack(value);
    SetIntegerKey(key);
}

//TableDatum::TableDatum(int32_t key, const TableDatum& value)
//{
//    PushBackTableDatum(value);
//    SetIntegerKey(key);
//}

TableDatum::TableDatum(const TableDatum& src) :
    Datum(src)
{
    mStringKey = src.mStringKey;
    mIntegerKey = src.mIntegerKey;
}

TableDatum& TableDatum::operator=(const TableDatum& src)
{
    if (this != &src)
    {
        Datum::operator=(src);
    }

    return *this;
}

void TableDatum::ReadStream(Stream& stream, bool net, bool external)
{
    Datum::ReadStream(stream, net, external);
    stream.ReadString(mStringKey);
    mIntegerKey = stream.ReadInt32();
}

void TableDatum::WriteStream(Stream& stream, bool net) const
{
    Datum::WriteStream(stream, net);
    stream.WriteString(mStringKey);
    stream.WriteInt32(mIntegerKey);
}

uint32_t TableDatum::GetSerializationSize(bool net) const
{
    return Datum::GetSerializationSize(net) +
        GetStringSerializationSize(mStringKey) +
        sizeof(mIntegerKey);
}

bool TableDatum::IsTableDatum() const
{
    return true;
}

void TableDatum::DeepCopy(const Datum& src, bool forceInternalStorage)
{
    Datum::DeepCopy(src, forceInternalStorage);

    if (src.IsTableDatum())
    {
        const TableDatum& srcTableDatum = (const TableDatum&)src;
        mStringKey = srcTableDatum.mStringKey;
        mIntegerKey = srcTableDatum.mIntegerKey;
    }
}

const char* TableDatum::GetStringKey() const
{
    return mStringKey.c_str();
}

int32_t TableDatum::GetIntegerKey() const
{
    return mIntegerKey;
}

bool TableDatum::IsStringKey() const
{
    return mStringKey != "";
}

bool TableDatum::IsIntegerKey() const
{
    return mStringKey == "";
}

void TableDatum::SetStringKey(const char* stringKey)
{
    if (stringKey != nullptr)
    {
        mStringKey = stringKey;
        mIntegerKey = 0;
    }
    else
    {
        mStringKey = "";
    }
}

void TableDatum::SetIntegerKey(int32_t integerKey)
{
    mIntegerKey = integerKey;
    mStringKey = "";
}
