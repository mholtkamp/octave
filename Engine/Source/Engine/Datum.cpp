#include "Datum.h"
#include "TableDatum.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Log.h"
#include "Stream.h"

#include "System/System.h"

Datum::Datum()
{

}

Datum::Datum(
    DatumType type,
    void* owner,
    void* data,
    uint32_t count,
    DatumChangeHandlerFP changeHandler)
{
    mType = type;
    mOwner = owner;
    mData.vp = data;
    mCount = (uint8_t) count;
    mChangeHandler = changeHandler;

    assert(count <= 255);

    // Datums constructed with this constructor are for external data only right now.
    mExternal = true;
}

Datum::Datum(const Datum& src)
{
    DeepCopy(src, false);
}

Datum::~Datum()
{
    Destroy();
}

Datum::Datum(int32_t value)
{
    PushBack(value);
}

Datum::Datum(float value)
{
    PushBack(value);
}

Datum::Datum(bool value)
{
    PushBack(value);
}

Datum::Datum(const char* value)
{
    std::string stringVal = value;
    PushBack(stringVal);
}

Datum::Datum(const std::string& value)
{
    PushBack(value);
}

Datum::Datum(glm::vec2 value)
{
    PushBack(value);
}

Datum::Datum(glm::vec3 value)
{
    PushBack(value);
}

Datum::Datum(glm::vec4 value)
{
    PushBack(value);
}

Datum::Datum(Asset* value)
{
    PushBack(value);
}

Datum::Datum(const AssetRef& value)
{
    PushBack(value.Get());
}

Datum::Datum(uint8_t value)
{
    PushBack(value);
}

DatumType Datum::GetType() const
{
    return mType;
}

void Datum::SetType(DatumType type)
{
    if (type == DatumType::Count)
    {
        LogError("Cannot set datum type to Count");
        assert(0);
    }

    if (mType == DatumType::Count ||
        mType == type)
    {
        mType = type;
    }
    else
    {
        LogError("Datum type has already been set and cannot be changed with SetType()")
        assert(0);
    }
}


uint32_t Datum::GetCount() const
{
    return mCount;
}

void Datum::SetCount(uint32_t count)
{
    if (mExternal)
    {
        LogError("Cannot set size on Datum with external storage.");
        assert(0);
    }

    if (mType == DatumType::Count)
    {
        LogError("Cannot set size on a Datum with no type associated with it.");
        assert(0);
    }

    if (count > mCapacity)
    {
        Reserve(count);
    }

    if (count < mCount)
    {
        assert(mData.vp != nullptr);

        for (uint32_t i = count; i < mCount; i++)
        {
            switch (mType)
            {
            case DatumType::String:
                mData.s[i].std::string::~string();
                break;
            case DatumType::Asset:
                mData.as[i].~AssetRef();
                break;
            case DatumType::Table:
                mData.t[i].~TableDatum();
                break;

            default: break;
            }
        }
    }

    // Default construct new elements, using placement new where needed.
    if (count > mCount)
    {
        assert(mData.vp != nullptr);

        for (uint32_t i = mCount; i < count; i++)
        {
            ConstructData(mData, i);
        }
    }

    mCount = count;
}


uint32_t Datum::GetDataTypeSize() const
{
    uint32_t size = 0;

    switch (mType)
    {
        case DatumType::Integer: size = sizeof(int32_t); break;
        case DatumType::Float: size = sizeof(float); break;
        case DatumType::Bool: size = sizeof(bool); break;
        case DatumType::String: size = sizeof(std::string); break;
        case DatumType::Vector2D: size = sizeof(glm::vec2); break;
        case DatumType::Vector: size = sizeof(glm::vec3); break;
        case DatumType::Color: size = sizeof(glm::vec4); break;
        case DatumType::Asset: size = sizeof(AssetRef); break;
        case DatumType::Enum: size = sizeof(uint32_t); break;
        case DatumType::Byte: size = sizeof(uint8_t); break;
        case DatumType::Table: size = sizeof(TableDatum); break;
        
        case DatumType::Count: size = 0; break;
    }

    return size;
}

uint32_t Datum::GetDataTypeSerializationSize() const
{
    uint32_t retSize = 0;

    if (mType == DatumType::String ||
        mType == DatumType::Asset ||
        mType == DatumType::Table)
    {
        for (uint32_t i = 0; i < mCount; ++i)
        {
            // These use custom serialization so we need to account for that.
            if (mType == DatumType::Table)
            {
                // Tables rely on recursion to determine their total size
                retSize += mData.t[i].GetDataTypeSerializationSize();
            }
            else
            {
                // Both String and Asset datum types are serialized as string size + string data
                retSize += sizeof(uint32_t);
                const std::string* stringPtr = nullptr;

                if (mType == DatumType::String)
                {
                    stringPtr = &mData.s[i];
                }
                else if (mType == DatumType::Asset)
                {
                    stringPtr = (mData.as[i].Get()) ? &(mData.as[i].Get()->GetName()) : nullptr;
                }

                if (stringPtr != nullptr)
                {
                    retSize += uint32_t(stringPtr->size());
                }
            }
        }
    }
    else
    {
        // Other types are basic and their serialization size is equivalent to their data type size * count;
        retSize += (mCount * GetDataTypeSize());
    }

    return retSize;
}

bool Datum::IsExternal() const
{
    return mExternal;
}

void Datum::ReadStream(Stream& stream, bool external)
{
    // If the datum was previously in use, destroy it.
    Destroy();

    mType = (DatumType) stream.ReadUint8();
    uint8_t count = stream.ReadUint8();
    mExternal = external;

    assert(mType != DatumType::Count);

    if (mExternal)
    {
        mCount = count;
        
        // This is assuming that the stream data will persist!!
        mData.vp = (stream.GetData() + stream.GetPos());

        // Skip ahead as if we read each element
        stream.SetPos(stream.GetPos() + GetDataTypeSize() * mCount);
    }
    else
    {
        // Allocate internal storage if not external.
        Reserve(count);
        for (uint32_t i = 0; i < count; ++i)
        {
            switch (mType)
            {
                case DatumType::Integer: PushBack(stream.ReadInt32()); break;
                case DatumType::Float: PushBack(stream.ReadFloat()); break;
                case DatumType::Bool: PushBack(stream.ReadBool()); break;
                case DatumType::String:
                {
                    PushBack(std::string());
                    stream.ReadString(mData.s[i]);
                    break;
                }
                case DatumType::Vector2D: PushBack(stream.ReadVec2()); break;
                case DatumType::Vector: PushBack(stream.ReadVec3()); break;
                case DatumType::Color: PushBack(stream.ReadVec4()); break;
                case DatumType::Asset:
                {
                    PushBack((Asset*) nullptr);
                    stream.ReadAsset(mData.as[i]);
                    break;
                }
                case DatumType::Enum: PushBack(stream.ReadUint32()); break;
                case DatumType::Byte: PushBack(stream.ReadUint8()); break;

                case DatumType::Table:
                {
                    PushBackTableDatum((TableDatum()));
                    mData.t[i].ReadStream(stream, external);
                    break;
                }

                case DatumType::Count: break;
            }
        }
    }
}

void Datum::WriteStream(Stream& stream) const
{
    stream.WriteUint8(uint8_t(mType));
    stream.WriteUint8(mCount);

    assert(mType != DatumType::Count);

    for (uint32_t i = 0; i < mCount; ++i)
    {
        switch (mType)
        {
            case DatumType::Integer: stream.WriteInt32(mData.i[i]); break;
            case DatumType::Float: stream.WriteFloat(mData.f[i]); break;
            case DatumType::Bool: stream.WriteBool(mData.b[i]); break;
            case DatumType::String: stream.WriteString(mData.s[i]); break;
            case DatumType::Vector2D: stream.WriteVec2(mData.v2[i]); break;
            case DatumType::Vector: stream.WriteVec3(mData.v3[i]); break;
            case DatumType::Color: stream.WriteVec4(mData.v4[i]); break;
            case DatumType::Asset: stream.WriteAsset(mData.as[i]); break;
            case DatumType::Enum: stream.WriteUint32(mData.e[i]); break;
            case DatumType::Byte: stream.WriteUint8(mData.by[i]); break;
            case DatumType::Table: mData.t[i].WriteStream(stream); break;

            case DatumType::Count: assert(0); break;
        }
    }
}

uint32_t Datum::GetSerializationSize() const
{
    uint32_t retSize = 
        sizeof(mType) + 
        sizeof(mCount) + 
        GetDataTypeSerializationSize();

    return retSize;
}

void Datum::SetInteger(int32_t value, uint32_t index)
{
    PreSet(index, DatumType::Integer);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.i[index] = value;
}

void Datum::SetFloat(float value, uint32_t index)
{
    PreSet(index, DatumType::Float);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.f[index] = value;
}

void Datum::SetBool(bool value, uint32_t index)
{
    PreSet(index, DatumType::Bool);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.b[index] = value;
}

void Datum::SetString(const std::string& value, uint32_t index)
{
    PreSet(index, DatumType::String);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.s[index] = value;
}

void Datum::SetVector2D(const glm::vec2& value, uint32_t index)
{
    PreSet(index, DatumType::Vector2D);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.v2[index] = value;
}

void Datum::SetVector(const glm::vec3& value, uint32_t index)
{
    PreSet(index, DatumType::Vector);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.v3[index] = value;
}

void Datum::SetColor(const glm::vec4& value, uint32_t index)
{
    PreSet(index, DatumType::Color);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.v4[index] = value;
}

void Datum::SetAsset(const Asset* value, uint32_t index)
{
    PreSet(index, DatumType::Asset);
    if (!mChangeHandler || !mChangeHandler(this, const_cast<Asset**>(&value)))
        mData.as[index] = value;
}

void Datum::SetEnum(uint32_t value, uint32_t index)
{
    PreSet(index, DatumType::Enum);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.e[index] = value;
}

void Datum::SetByte(uint8_t value, uint32_t index)
{
    PreSet(index, DatumType::Byte);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.by[index] = value;
}

void Datum::SetTableDatum(const TableDatum& value, uint32_t index)
{
    PreSet(index, DatumType::Table);
    if (!mChangeHandler || !mChangeHandler(this, &value))
        mData.t[index] = value;
}

void Datum::SetValue(const void* value, uint32_t index, uint32_t count)
{
    assert(mType != DatumType::Count);
    assert(index + count <= mCount);

    if (mType != DatumType::Count &&
        value != nullptr)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            // Out of bounds?
            if (index + i >= mCount)
                continue;

            switch (mType)
            {
            case DatumType::Bool: SetBool(*(reinterpret_cast<const bool*>(value) + i),                index + i); break;
            case DatumType::Color: SetColor(*(reinterpret_cast<const glm::vec4*>(value) + i),         index + i); break;
            case DatumType::Float: SetFloat(*(reinterpret_cast<const float*>(value) + i),             index + i); break;
            case DatumType::Integer: SetInteger(*(reinterpret_cast<const int32_t*>(value) + i),       index + i); break;
            case DatumType::String: SetString(*(reinterpret_cast<const std::string*>(value) + i),     index + i); break;
            case DatumType::Vector2D: SetVector2D(*(reinterpret_cast<const glm::vec2*>(value) + i),   index + i); break;
            case DatumType::Vector: SetVector(*(reinterpret_cast<const glm::vec3*>(value) + i),       index + i); break;
            case DatumType::Asset: SetAsset(*(reinterpret_cast<const Asset* const*>(value) + i),      index + i); break;
            case DatumType::Enum: SetEnum(*(reinterpret_cast<const uint32_t*>(value) + i),            index + i); break;
            case DatumType::Byte: SetByte(*(reinterpret_cast<const uint8_t*>(value) + i),             index + i); break;
            case DatumType::Table: SetTableDatum(*(reinterpret_cast<const TableDatum*>(value) + i),   index + i); break;
            case DatumType::Count: break;
            }
        }
    }
}

void Datum::SetValueRaw(const void* value, uint32_t index)
{
    switch (mType)
    {
    case DatumType::Bool: mData.b[index] = *reinterpret_cast<const bool*>(value); break;
    case DatumType::Color: mData.v4[index] = *reinterpret_cast<const glm::vec4*>(value); break;
    case DatumType::Float: mData.f[index] = *reinterpret_cast<const float*>(value); break;
    case DatumType::Integer: mData.i[index] = *reinterpret_cast<const int32_t*>(value); break;
    case DatumType::String: mData.s[index] = *reinterpret_cast<const std::string*>(value); break;
    case DatumType::Vector2D: mData.v2[index] = *reinterpret_cast<const glm::vec2*>(value); break;
    case DatumType::Vector: mData.v3[index] = *reinterpret_cast<const glm::vec3*>(value); break;
    case DatumType::Asset: mData.as[index] = *reinterpret_cast<const Asset* const*>(value); break;
    case DatumType::Enum: mData.e[index] = *reinterpret_cast<const uint32_t*>(value); break;
    case DatumType::Byte: mData.by[index] = *reinterpret_cast<const uint8_t*>(value); break;
    case DatumType::Table: mData.t[index] = *reinterpret_cast<const TableDatum*>(value); break;

    case DatumType::Count: break;
    }
}

void Datum::SetExternal(int32_t* data,  uint32_t count)
{
    PreSetExternal(DatumType::Integer);
    mData.i = data;
    PostSetExternal(DatumType::Integer, count);
}
void Datum::SetExternal(float* data,  uint32_t count)
{
    PreSetExternal(DatumType::Float);
    mData.f = data;
    PostSetExternal(DatumType::Float, count);
}
void Datum::SetExternal(bool* data,  uint32_t count)
{
    PreSetExternal(DatumType::Bool);
    mData.b = data;
    PostSetExternal(DatumType::Bool, count);
}
void Datum::SetExternal(std::string* data,  uint32_t count)
{
    PreSetExternal(DatumType::String);
    mData.s = data;
    PostSetExternal(DatumType::String, count);
}
void Datum::SetExternal(glm::vec2* data,  uint32_t count)
{
    PreSetExternal(DatumType::Vector2D);
    mData.v2 = data;
    PostSetExternal(DatumType::Vector2D, count);
}
void Datum::SetExternal(glm::vec3* data,  uint32_t count)
{
    PreSetExternal(DatumType::Vector);
    mData.v3 = data;
    PostSetExternal(DatumType::Vector, count);
}
void Datum::SetExternal(glm::vec4* data,  uint32_t count)
{
    PreSetExternal(DatumType::Color);
    mData.v4 = data;
    PostSetExternal(DatumType::Color, count);
}
void Datum::SetExternal(AssetRef* data,  uint32_t count)
{
    PreSetExternal(DatumType::Asset);
    mData.as = data;
    PostSetExternal(DatumType::Asset, count);
}
void Datum::SetExternal(uint32_t* data,  uint32_t count)
{
    PreSetExternal(DatumType::Enum);
    mData.e = data;
    PostSetExternal(DatumType::Enum, count);
}
void Datum::SetExternal(uint8_t* data,  uint32_t count)
{
    PreSetExternal(DatumType::Byte);
    mData.by = data;
    PostSetExternal(DatumType::Byte, count);
}

void Datum::SetExternal(TableDatum* data, uint32_t count)
{
    PreSetExternal(DatumType::Table);
    mData.t = data;
    PostSetExternal(DatumType::Table, count);
}

int32_t Datum::GetInteger(uint32_t index) const
{
    PreGet(index, DatumType::Integer);
    return mData.i[index];
}

float Datum::GetFloat(uint32_t index) const
{
    PreGet(index, DatumType::Float);
    return mData.f[index];
}

bool Datum::GetBool(uint32_t index) const
{
    PreGet(index, DatumType::Bool);
    return mData.b[index];
}

const std::string& Datum::GetString(uint32_t index) const
{
    PreGet(index, DatumType::String);
    return mData.s[index];
}

const glm::vec2& Datum::GetVector2D(uint32_t index) const
{
    PreGet(index, DatumType::Vector2D);
    return mData.v2[index];
}

const glm::vec3& Datum::GetVector(uint32_t index) const
{
    PreGet(index, DatumType::Vector);
    return mData.v3[index];
}

const glm::vec4& Datum::GetColor(uint32_t index) const
{
    PreGet(index, DatumType::Color);
    return mData.v4[index];
}

Asset* Datum::GetAsset(uint32_t index) const
{
    PreGet(index, DatumType::Asset);
    return mData.as[index].Get();
}

uint32_t Datum::GetEnum(uint32_t index) const
{
    PreGet(index, DatumType::Enum);
    return mData.e[index];
    //assert(mEnumCount > 0);
    //return *reinterpret_cast<uint32_t*>(mData) % mEnumCount;
}

uint8_t Datum::GetByte(uint32_t index) const
{
    PreGet(index, DatumType::Byte);
    return mData.by[index];
}

TableDatum& Datum::GetTableDatum(uint32_t index)
{
    PreGet(index, DatumType::Table);
    return mData.t[index];
}

const TableDatum& Datum::GetTableDatum(uint32_t index) const
{
    PreGet(index, DatumType::Table);
    return mData.t[index];
}

void Datum::PushBack(int32_t value)
{
    PrePushBack(DatumType::Integer);
    new (mData.i + mCount) int32_t(value);
    mCount++;
}

void Datum::PushBack(float value)
{
    PrePushBack(DatumType::Float);
    new (mData.f + mCount) float(value);
    mCount++;
}

void Datum::PushBack(bool value)
{
    PrePushBack(DatumType::Bool);
    new (mData.b + mCount) bool(value);
    mCount++;
}

void Datum::PushBack(const std::string& value)
{
    PrePushBack(DatumType::String);
    new (mData.s + mCount) std::string(value);
    mCount++;
}

void Datum::PushBack(const char* value)
{
    PrePushBack(DatumType::String);
    new (mData.s + mCount) std::string(value);
    mCount++;
}

void Datum::PushBack(const glm::vec2& value)
{
    PrePushBack(DatumType::Vector2D);
    new (mData.v2 + mCount) glm::vec2(value);
    mCount++;
}

void Datum::PushBack(const glm::vec3& value)
{
    PrePushBack(DatumType::Vector);
    new (mData.v3 + mCount) glm::vec3(value);
    mCount++;
}

void Datum::PushBack(const glm::vec4& value)
{
    PrePushBack(DatumType::Color);
    new (mData.v4 + mCount) glm::vec4(value);
    mCount++;
}

void Datum::PushBack(Asset* value)
{
    PrePushBack(DatumType::Asset);
    new (mData.as + mCount) AssetRef(value);
    mCount++;
}

void Datum::PushBack(uint32_t value)
{
    PrePushBack(DatumType::Enum);
    new (mData.e + mCount) uint32_t(value);
    mCount++;
}

void Datum::PushBack(uint8_t value)
{
    PrePushBack(DatumType::Byte);
    new (mData.by + mCount) uint8_t(value);
    mCount++;
}

void Datum::PushBackTableDatum(const TableDatum& value)
{
    PrePushBack(DatumType::Table);
    new (mData.t + mCount) TableDatum(value);
    mCount++;
}

Datum& Datum::operator=(const Datum& src)
{
    if (this != &src)
    {
        Destroy();
        DeepCopy(src, false);
    }

    return *this;
}

Datum& Datum::operator=(int32_t src)
{
    PreAssign(DatumType::Integer);
    mData.i[0] = src;
    return *this;
}

Datum& Datum::operator=(float src)
{
    PreAssign(DatumType::Float);
    mData.f[0] = src;
    return *this;
}

Datum& Datum::operator=(bool src)
{
    PreAssign(DatumType::Bool);
    mData.b[0] = src;
    return *this;
}

Datum& Datum::operator=(const std::string& src)
{
    PreAssign(DatumType::String);
    mData.s[0] = src;
    return *this;
}

Datum& Datum::operator=(const char* src)
{
    PreAssign(DatumType::String);
    mData.s[0] = src;
    return *this;
}

Datum& Datum::operator=(const glm::vec2 src)
{
    PreAssign(DatumType::Vector2D);
    mData.v2[0] = src;
    return *this;
}

Datum& Datum::operator=(const glm::vec3& src)
{
    PreAssign(DatumType::Vector);
    mData.v3[0] = src;
    return *this;
}

Datum& Datum::operator=(const glm::vec4& src)
{
    PreAssign(DatumType::Color);
    mData.v4[0] = src;
    return *this;
}

Datum& Datum::operator=(Asset* src)
{
    PreAssign(DatumType::Asset);
    mData.as[0] = src;
    return *this;
}

Datum& Datum::operator=(uint8_t src)
{
    PreAssign(DatumType::Byte);
    mData.by[0] = src;
    return *this;
}

bool Datum::operator==(const Datum& other) const
{
    if (mType != other.mType ||
        mCount != other.mCount)
    {
        return false;
    }

    uint32_t dataSize = GetDataTypeSize();

    for (uint32_t i = 0; i < mCount; i++)
    {
        assert(mType != DatumType::Count);

        switch (mType)
        {
        case DatumType::String:
            if (mData.s[i] != other.mData.s[i])
            {
                return false;
            }
            break;
        // DatumType::Asset doesn't need this, but DatumType::Actor might? Maybe not if its just a pointer like AssetRef
        case DatumType::Table:
            if (mData.t[i] != other.mData.t[i])
            {
                return false;
            }
            break;
        default:
            if (memcmp(reinterpret_cast<char*>(mData.vp) + (i * dataSize), reinterpret_cast<char*>(other.mData.vp) + (i * dataSize), dataSize) != 0)
            {
                return false;
            }
            break;
        }
    }

    return true;
}

bool Datum::operator==(const int32_t& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Integer)
    {
        return false;
    }

    return mData.i[0] == other;
}

bool Datum::operator==(const float& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Float)
    {
        return false;
    }

    return mData.f[0] == other;
}

bool Datum::operator==(const bool& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Bool)
    {
        return false;
    }

    return mData.b[0] == other;
}

bool Datum::operator==(const std::string& other) const
{
    if (mCount == 0 ||
        mType != DatumType::String)
    {
        return false;
    }

    return mData.s[0] == other;
}

bool Datum::operator==(const char* other) const
{
    if (mCount == 0 ||
        mType != DatumType::String)
    {
        return false;
    }

    return mData.s[0] == other;
}


bool Datum::operator==(const glm::vec2& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Vector2D)
    {
        return false;
    }

    return mData.v2[0] == other;
}

bool Datum::operator==(const glm::vec3& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Vector)
    {
        return false;
    }

    return mData.v3[0] == other;
}

bool Datum::operator==(const glm::vec4& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Color)
    {
        return false;
    }

    return mData.v4[0] == other;
}

bool Datum::operator==(const Asset*& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Asset)
    {
        return false;
    }

    return mData.as[0].Get() == other;
}

bool Datum::operator==(const uint32_t& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Enum)
    {
        return false;
    }

    return mData.e[0] == other;
}

bool Datum::operator==(const uint8_t& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Byte)
    {
        return false;
    }

    return mData.by[0] == other;
}

bool Datum::operator!=(const Datum& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const int32_t& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const float& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const bool& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const std::string& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const char* other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const glm::vec2& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const glm::vec3& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const glm::vec4& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const Asset*& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const uint32_t& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const uint8_t& other) const
{
    return !operator==(other);
}

bool Datum::IsProperty() const
{
    return false;
}

bool Datum::IsTableDatum() const
{
    return false;
}

void Datum::Reserve(uint32_t capacity)
{
    if (capacity > mCapacity)
    {
        if (mExternal)
        {
            LogError("Cannot reserve internal memory for an external Datum.");
            assert(0);
        }

        if (mType == DatumType::Count)
        {
            LogError("Cannot reserve internal memory for a typeless Datum.");
            assert(0);
        }

        void* previousBuffer = mData.vp;

        mCapacity = capacity;
        uint32_t typeSize = GetDataTypeSize();
        mData.vp = SYS_AlignedMalloc(mCapacity * typeSize, 4);

        if (previousBuffer != nullptr)
        {
            memcpy(mData.vp, previousBuffer, typeSize * mCount);

            SYS_AlignedFree(previousBuffer);
            previousBuffer = nullptr;
        }
    }
}

void Datum::Destroy()
{
    // Delete internal memory.
    if (!mExternal && 
        mData.vp != nullptr)
    {
        assert(mType != DatumType::Count);
        assert(mCount > 0);

        for (uint32_t i = 0; i < mCount; i++)
        {
            DestructData(mData, i);
        }

        SYS_AlignedFree(mData.vp);
        mData.vp = nullptr;
    }

    Reset();
}

void Datum::DeepCopy(const Datum& src, bool forceInternalStorage)
{
    mType = src.mType;
    mExternal = src.mExternal;
    mOwner = src.mOwner;
    mChangeHandler = src.mChangeHandler;

    if (forceInternalStorage)
    {
        mExternal = false;
    }

    if (mExternal)
    {
        mData.vp = src.mData.vp;
    }
    else
    {
        Reserve(src.mCapacity);

        for (std::uint32_t i = 0; i < src.mCount; i++)
        {
            switch (mType)
            {
            case DatumType::Integer:
                PushBack(*(src.mData.i + i));
                break;
            case DatumType::Float:
                PushBack(*(src.mData.f + i));
                break;
            case DatumType::Bool:
                PushBack(*(src.mData.b + i));
                break;
            case DatumType::String:
                PushBack(*(src.mData.s + i));
                break;
            case DatumType::Vector2D:
                PushBack(*(src.mData.v2 + i));
                break;
            case DatumType::Vector:
                PushBack(*(src.mData.v3 + i));
                break;
            case DatumType::Color:
                PushBack(*(src.mData.v4 + i));
                break;
            case DatumType::Asset:
                PushBack((src.mData.as + i)->Get());
                break;
            case DatumType::Enum:
                PushBack(*(src.mData.e + i));
                break;
            case DatumType::Byte:
                PushBack(*(src.mData.by + i));
                break;
            case DatumType::Table:
                PushBackTableDatum(*(src.mData.t + i));
                break;

            case DatumType::Count:
                break;
            }
        }
    }

    // Copy these after we have copied the individual elements (in the !external branch)
    mCount = src.mCount;
    mCapacity = src.mCapacity;
}

void Datum::PreSet(uint32_t index, DatumType type)
{
    if (index >= mCount)
    {
        LogError("Index out of bounds - Datum::Set().");
        assert(0);
    }

    if (mType != type)
    {
        LogError("Type mismatch - Datum::Set()");
        assert(0);
    }
}

void Datum::PreSetExternal(DatumType type)
{
    if (mType == DatumType::Count)
    {
        mType = type;
    }

    if (mType != type)
    {
        LogError("Datum::SetExternal() - type mismatch");
        assert(0);
    }

    if (!mExternal &&
        (mCapacity > 0 ||
        mCount > 0))
    {
        LogError("Cannot set storage on Datum with internal storage already allocated.");
        assert(0);
    }
}

void Datum::PostSetExternal(DatumType type, uint32_t count)
{
    assert(type != DatumType::Count);
    assert(type == mType);

    mCount = count;
    mCapacity = count;
    mExternal = true;
    mType = type;
}

void Datum::PrePushBack(DatumType type)
{
    if (mExternal)
    {
        LogError("Cannot PushBack on Datum with external storage.");
        assert(0);
    }

    if (mType == DatumType::Count)
    {
        SetType(type);
    }

    if (mType != type)
    {
        LogError("Datum::PushBack() - Type mismatch");
        assert(0);
    }

    if (mCount == mCapacity)
    {
        Reserve(mCapacity + 1);
    }
}

void Datum::PreGet(uint32_t index, DatumType type) const
{
    if (index >= mCount)
    {
        LogError("Index out of bounds - Datum::Get().");
        assert(0);
    }

    if (mType != type)
    {
        LogError("Type mismatch - Propterty::Get()");
        assert(0);
    }
}

void Datum::PreAssign(DatumType type)
{
    if (mType == DatumType::Count)
    {
        mType = type;
    }

    if (mType != type)
    {
        LogError("Invalid type assignment - Datum::PreAssign()");
        assert(0);
    }

    // Make sure we have at least one element, since assign operations only affect index 0.
    if (mCount == 0)
    {
        Reserve(1);
        ConstructData(mData, 0);

        mCount = 1;
    }
}

void Datum::ConstructData(DatumData& dataUnion, uint32_t index)
{
    switch (mType)
    {
    case DatumType::Integer:
        dataUnion.i[index] = 0;
        break;
    case DatumType::Float:
        dataUnion.f[index] = 0.0f;
        break;
    case DatumType::Bool:
        dataUnion.b[index] = false;
        break;
    case DatumType::String:
        new (dataUnion.s + index) std::string();
        break;
    case DatumType::Vector2D:
        dataUnion.v2[index] = {0.0f, 0.0f};
        break;
    case DatumType::Vector:
        dataUnion.v3[index] = {0.0f, 0.0f, 0.0f};
        break;
    case DatumType::Color:
        dataUnion.v4[index] = {0.0f, 0.0f, 0.0f, 0.0f};
        break;
    case DatumType::Asset:
        new (dataUnion.as + index) AssetRef();
        break;
    case DatumType::Enum:
        dataUnion.e[index] = 0;
        break;
    case DatumType::Byte:
        dataUnion.by[index] = 0;
        break;
    case DatumType::Table:
        new (dataUnion.t + index) TableDatum();
        break;

    case DatumType::Count:
        assert(0);
        break;
    }
}

void Datum::DestructData(DatumData& dataUnion, uint32_t index)
{
    switch (mType)
    {
    case DatumType::String:
        dataUnion.s[index].std::string::~string();
        break;
    case DatumType::Asset:
        dataUnion.as[index].AssetRef::~AssetRef();
        break;
    case DatumType::Table:
        dataUnion.t[index].TableDatum::~TableDatum();
        break;

    default: break;
    }
}

void Datum::Reset()
{
    mType = DatumType::Count;
    mExternal = false;
    mOwner = nullptr;
    mData.vp = nullptr;
    mChangeHandler = nullptr;
    mCount = 0;
    mCapacity = 0;
}
