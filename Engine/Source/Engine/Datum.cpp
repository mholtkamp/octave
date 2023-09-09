#include "Datum.h"
#include "TableDatum.h"
#include "ScriptFunc.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Log.h"
#include "Stream.h"
#include "World.h"

#include "System/System.h"

Datum::Datum()
{
    mForceScriptArray = false;
    mExternal = false;
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
    mForceScriptArray = false;

    OCT_ASSERT(count <= 255);

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
    Reset();
    PushBack(value);
}

Datum::Datum(uint32_t value)
{
    Reset();
    PushBack(int32_t(value));
}

Datum::Datum(float value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(bool value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(const char* value)
{
    Reset();
    std::string stringVal = value;
    PushBack(stringVal);
}

Datum::Datum(const std::string& value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(glm::vec2 value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(glm::vec3 value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(glm::vec4 value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(Asset* value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(const AssetRef& value)
{
    Reset();
    PushBack(value.Get());
}

Datum::Datum(uint8_t value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(RTTI* value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(int16_t value)
{
    Reset();
    PushBack(value);
}

Datum::Datum(const ScriptFunc& value)
{
    Reset();
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
        OCT_ASSERT(0);
    }

    if (mType == DatumType::Count ||
        mType == type)
    {
        mType = type;
    }
    else
    {
        LogError("Datum type has already been set and cannot be changed with SetType()");
        OCT_ASSERT(0);
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
        OCT_ASSERT(0);
    }

    if (mType == DatumType::Count)
    {
        LogError("Cannot set size on a Datum with no type associated with it.");
        OCT_ASSERT(0);
    }

    if (count > mCapacity)
    {
        Reserve(count);
    }

    if (count < mCount)
    {
        OCT_ASSERT(mData.vp != nullptr);

        for (uint32_t i = count; i < mCount; i++)
        {
            DestructData(mData, i);
        }
    }

    // Default construct new elements, using placement new where needed.
    if (count > mCount)
    {
        OCT_ASSERT(mData.vp != nullptr);

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
        case DatumType::Byte: size = sizeof(uint8_t); break;
        case DatumType::Table: size = sizeof(TableDatum); break;
        case DatumType::Pointer: size = sizeof(RTTI*); break;
        case DatumType::Short: size = sizeof(int16_t); break;
        case DatumType::Function: size = sizeof(ScriptFunc); break;
        
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
    else if (mType == DatumType::Pointer)
    {
        // Pointers are serialized as NetId's, which are 32 bits.
        // So this will be different from sizeof(RTTI*) on 64 bit architectures.
        retSize += (mCount * sizeof(NetId));
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

bool Datum::IsValid() const
{
    return (mType != DatumType::Count && mCount > 0);
}

void Datum::ReadStream(Stream& stream, bool external)
{
    // If the datum was previously in use, destroy it.
    Destroy();

    mType = (DatumType) stream.ReadUint8();
    uint8_t count = stream.ReadUint8();
    mExternal = external;

    OCT_ASSERT(mType != DatumType::Count);

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
                case DatumType::Byte: PushBack(stream.ReadUint8()); break;

                case DatumType::Table:
                {
                    PushBackTableDatum((TableDatum()));
                    mData.t[i].ReadStream(stream, external);
                    break;
                }

                case DatumType::Pointer:
                {
                    // Pointers can only be serialized if it is an actor AND we
                    // are serializing across the network.
                    NetId netId = (NetId)stream.ReadUint32();
                    Actor* localActor = GetWorld()->FindActor(netId);
                    PushBack(localActor);
                    break;
                }
                case DatumType::Short: PushBack(stream.ReadInt16()); break;

                // Functions can't be serialized to file or network.
                case DatumType::Function: OCT_ASSERT(0); break;

                case DatumType::Count: break;
            }
        }
    }
}

void Datum::WriteStream(Stream& stream) const
{
    stream.WriteUint8(uint8_t(mType));
    stream.WriteUint8(mCount);

    OCT_ASSERT(mType != DatumType::Count);

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
            case DatumType::Byte: stream.WriteUint8(mData.by[i]); break;
            case DatumType::Table: mData.t[i].WriteStream(stream); break;
            case DatumType::Pointer:
            {
                // Pointers can only be serialized if it is an actor AND we
                // are serializing across the network.
                NetId netId = INVALID_NET_ID; 
                RTTI* rtti = mData.p[i];
                Actor* actor = rtti ? rtti->As<Actor>() : nullptr;

                if (actor != nullptr)
                {
                    netId = actor->GetNetId();
                }

                stream.WriteUint32((uint32_t)netId);
                break;
            }
            case DatumType::Short: stream.WriteInt16(mData.sh[i]); break;

            // Functions can't be serialized.
            case DatumType::Function: OCT_ASSERT(0); break;

            case DatumType::Count: OCT_ASSERT(0); break;
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
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.i[index] = value;
}

void Datum::SetFloat(float value, uint32_t index)
{
    PreSet(index, DatumType::Float);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.f[index] = value;
}

void Datum::SetBool(bool value, uint32_t index)
{
    PreSet(index, DatumType::Bool);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.b[index] = value;
}

void Datum::SetString(const std::string& value, uint32_t index)
{
    PreSet(index, DatumType::String);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.s[index] = value;
}

void Datum::SetVector2D(const glm::vec2& value, uint32_t index)
{
    PreSet(index, DatumType::Vector2D);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.v2[index] = value;
}

void Datum::SetVector(const glm::vec3& value, uint32_t index)
{
    PreSet(index, DatumType::Vector);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.v3[index] = value;
}

void Datum::SetColor(const glm::vec4& value, uint32_t index)
{
    PreSet(index, DatumType::Color);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.v4[index] = value;
}

void Datum::SetAsset(const Asset* value, uint32_t index)
{
    PreSet(index, DatumType::Asset);
    if (!mChangeHandler || !mChangeHandler(this, index, const_cast<Asset**>(&value)))
        mData.as[index] = value;
}

void Datum::SetByte(uint8_t value, uint32_t index)
{
    PreSet(index, DatumType::Byte);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.by[index] = value;
}

void Datum::SetTableDatum(const TableDatum& value, uint32_t index)
{
    PreSet(index, DatumType::Table);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.t[index] = value;
}

void Datum::SetPointer(RTTI* value, uint32_t index)
{
    PreSet(index, DatumType::Pointer);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.p[index] = value;
}

void Datum::SetShort(int16_t value, uint32_t index)
{
    PreSet(index, DatumType::Short);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.sh[index] = value;
}

void Datum::SetFunction(const ScriptFunc& value, uint32_t index)
{
    PreSet(index, DatumType::Function);
    if (!mChangeHandler || !mChangeHandler(this, index, &value))
        mData.fn[index] = value;
}

void Datum::SetValue(const void* value, uint32_t index, uint32_t count)
{
    OCT_ASSERT(mType != DatumType::Count);
    OCT_ASSERT(index + count <= mCount);

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
            case DatumType::Asset: SetAsset((reinterpret_cast<const AssetRef*>(value) + i)->Get(),    index + i); break;
            case DatumType::Byte: SetByte(*(reinterpret_cast<const uint8_t*>(value) + i),             index + i); break;
            case DatumType::Table: SetTableDatum(*(reinterpret_cast<const TableDatum*>(value) + i),   index + i); break;
            case DatumType::Pointer: SetPointer(*(((RTTI**)value) + i),                               index + i); break;
            case DatumType::Short: SetShort(*(reinterpret_cast<const int16_t*>(value) + i),           index + i); break;
            case DatumType::Function: SetFunction(*(reinterpret_cast<const ScriptFunc*>(value) + i),  index + i); break;
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
    case DatumType::Byte: mData.by[index] = *reinterpret_cast<const uint8_t*>(value); break;
    case DatumType::Table: mData.t[index] = *reinterpret_cast<const TableDatum*>(value); break;
    case DatumType::Pointer: mData.p[index] = *((RTTI**)value); break;
    case DatumType::Short: mData.sh[index] = *reinterpret_cast<const int16_t*>(value); break;
    case DatumType::Function: mData.fn[index] = *reinterpret_cast<const ScriptFunc*>(value); break;

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

void Datum::SetExternal(RTTI** data, uint32_t count)
{
    PreSetExternal(DatumType::Pointer);
    mData.p = data;
    PostSetExternal(DatumType::Pointer, count);
}

void Datum::SetExternal(int16_t* data, uint32_t count)
{
    PreSetExternal(DatumType::Short);
    mData.sh = data;
    PostSetExternal(DatumType::Short, count);
}

void Datum::SetExternal(ScriptFunc* data, uint32_t count)
{
    PreSetExternal(DatumType::Function);
    mData.fn = data;
    PostSetExternal(DatumType::Function, count);
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

RTTI* Datum::GetPointer(uint32_t index) const
{
    PreGet(index, DatumType::Pointer);
    return mData.p[index];
}

int16_t Datum::GetShort(uint32_t index) const
{
    PreGet(index, DatumType::Short);
    return mData.sh[index];
}

const ScriptFunc& Datum::GetFunction(uint32_t index) const
{
    PreGet(index, DatumType::Function);
    return mData.fn[index];
}

TableDatum* Datum::FindTableDatum(const char* key)
{
    TableDatum* ret = nullptr;
    if (mType == DatumType::Table)
    {
        for (uint32_t i = 0; i < GetCount(); ++i)
        {
            TableDatum& td = GetTableDatum(i);

            if (td.IsStringKey() &&
                strcmp(td.GetStringKey(), key) == 0)
            {
                ret = &td;
                break;
            }
        }
    }

    return ret;
}

TableDatum* Datum::FindTableDatum(int32_t key)
{
    TableDatum* ret = nullptr;
    if (mType == DatumType::Table)
    {
        for (uint32_t i = 0; i < GetCount(); ++i)
        {
            TableDatum& td = GetTableDatum(i);

            if (td.IsIntegerKey() &&
                td.GetIntegerKey() == key)
            {
                ret = &td;
                break;
            }
        }
    }

    return ret;
}

TableDatum* Datum::GetField(const char* key)
{
    return FindTableDatum(key);
}

TableDatum* Datum::GetField(int32_t key)
{
    return FindTableDatum(key);
}

TableDatum* Datum::AddTableField(int32_t key)
{
    TableDatum* td = PushBackTableDatum(TableDatum());
    td->SetIntegerKey(key);
    return td;
}

TableDatum* Datum::AddTableField(const char* key)
{
    TableDatum* td = PushBackTableDatum(TableDatum());
    td->SetStringKey(key);
    return td;
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

void Datum::PushBack(uint8_t value)
{
    PrePushBack(DatumType::Byte);
    new (mData.by + mCount) uint8_t(value);
    mCount++;
}

TableDatum* Datum::PushBackTableDatum(const TableDatum& value)
{
    PrePushBack(DatumType::Table);
    new (mData.t + mCount) TableDatum(value);
    mCount++;

    return (mData.t + (mCount - 1));
}

void Datum::PushBack(RTTI* value)
{
    PrePushBack(DatumType::Pointer);
    new (mData.p + mCount) RTTI*(value);
    mCount++;
}

void Datum::PushBack(int16_t value)
{
    PrePushBack(DatumType::Short);
    new (mData.sh + mCount) int16_t(value);
    mCount++;
}

void Datum::PushBack(const ScriptFunc& value)
{
    PrePushBack(DatumType::Function);
    new (mData.fn + mCount) ScriptFunc(value);
    mCount++;
}

void Datum::Erase(uint32_t index)
{
    if (index >= 0 && index < mCount)
    {
        uint32_t numElementsToShift = (mCount - 1) - index;

        for (uint32_t i = 0; i < numElementsToShift; ++i)
        {
            CopyData(mData, index + i, mData, index + i + 1);
        }

        SetCount(mCount - 1);
    }
}

#define DEFINE_GET_FIELD(KeyType, DatType, Type, Default)           \
Type Datum::Get##DatType##Field(KeyType key)                        \
{                                                                   \
    Type ret = Default;                                             \
    if (mType == DatumType::Table)                                  \
    {                                                               \
        TableDatum* td = FindTableDatum(key);                       \
        if (td != nullptr && td->GetType() == DatumType::DatType)   \
        {                                                           \
            ret = td->Get##DatType();                               \
        }                                                           \
    }                                                               \
    return ret;                                                     \
}

//DEFINE_GET_FIELD(const char*, Integer, int32_t, 0)
//DEFINE_GET_FIELD(const char*, Float, float, 0.0f)
int32_t Datum::GetIntegerField(const char* key)
{
    int32_t ret = 0;
    if (mType == DatumType::Table)
    {
        TableDatum* td = FindTableDatum(key);
        if (td != nullptr)
        {
            if (td->GetType() == DatumType::Integer)
            {
                ret = td->GetInteger();
            }
            else if (td->GetType() == DatumType::Float)
            {
                ret = int32_t(td->GetFloat() + 0.5f);
            }
        }
    }
    return ret;
}

float Datum::GetFloatField(const char* key)
{
    float ret = 0;
    if (mType == DatumType::Table)
    {
        TableDatum* td = FindTableDatum(key);
        if (td != nullptr)
        {
            if (td->GetType() == DatumType::Float)
            {
                ret = td->GetFloat();
            }
            else if (td->GetType() == DatumType::Integer)
            {
                ret = (float)td->GetInteger();
            }
        }
    }
    return ret;
}

DEFINE_GET_FIELD(const char*, Bool, bool, false)
DEFINE_GET_FIELD(const char*, String, std::string, "")
DEFINE_GET_FIELD(const char*, Vector2D, glm::vec2, {})
DEFINE_GET_FIELD(const char*, Vector, glm::vec3, {})
DEFINE_GET_FIELD(const char*, Color, glm::vec4, {})
DEFINE_GET_FIELD(const char*, Asset, Asset*, nullptr)
DEFINE_GET_FIELD(const char*, Pointer, RTTI*, nullptr)
DEFINE_GET_FIELD(const char*, Function, ScriptFunc, {})

//DEFINE_GET_FIELD(int32_t, Integer, int32_t, 0)
//DEFINE_GET_FIELD(int32_t, Float, float, 0.0f)

int32_t Datum::GetIntegerField(int32_t key)
{
    int32_t ret = 0;
    if (mType == DatumType::Table)
    {
        TableDatum* td = FindTableDatum(key);
        if (td != nullptr)
        {
            if (td->GetType() == DatumType::Integer)
            {
                ret = td->GetInteger();
            }
            else if (td->GetType() == DatumType::Float)
            {
                ret = int32_t(td->GetFloat() + 0.5f);
            }
        }
    }
    return ret;
}

float Datum::GetFloatField(int32_t key)
{
    float ret = 0;
    if (mType == DatumType::Table)
    {
        TableDatum* td = FindTableDatum(key);
        if (td != nullptr)
        {
            if (td->GetType() == DatumType::Float)
            {
                ret = td->GetFloat();
            }
            else if (td->GetType() == DatumType::Integer)
            {
                ret = (float)td->GetInteger();
            }
        }
    }
    return ret;
}

DEFINE_GET_FIELD(int32_t, Bool, bool, false)
DEFINE_GET_FIELD(int32_t, String, std::string, "")
DEFINE_GET_FIELD(int32_t, Vector2D, glm::vec2, {})
DEFINE_GET_FIELD(int32_t, Vector, glm::vec3, {})
DEFINE_GET_FIELD(int32_t, Color, glm::vec4, {})
DEFINE_GET_FIELD(int32_t, Asset, Asset*, nullptr)
DEFINE_GET_FIELD(int32_t, Pointer, RTTI*, nullptr)
DEFINE_GET_FIELD(int32_t, Function, ScriptFunc, {})

TableDatum& Datum::GetTableField(const char* key)
{
    static TableDatum nullTable;
    TableDatum* ret = &nullTable;

    if (mType == DatumType::Table)
    {
        TableDatum* td = FindTableDatum(key);
        if (td != nullptr && td->GetType() == DatumType::Table)
        {
            ret = td;
        }
    }

    return *ret;
}

TableDatum& Datum::GetTableField(int32_t key)
{
    static TableDatum nullTable;
    TableDatum* ret = &nullTable;

    if (mType == DatumType::Table)
    {
        TableDatum* td = FindTableDatum(key);
        if (td != nullptr && td->GetType() == DatumType::Table)
        {
            ret = td;
        }
    }

    return *ret;
}

#define DEFINE_SET_FIELD(KeyType, DatType, Type)                                            \
void Datum::Set##DatType##Field(KeyType key, Type value)                                    \
{                                                                                           \
    if (mType == DatumType::Table || mType == DatumType::Count)                             \
    {                                                                                       \
        TableDatum* td = FindTableDatum(key);                                               \
        if (td == nullptr)                                                                  \
        {                                                                                   \
            PushBackTableDatum(TableDatum(key, value));                                     \
        }                                                                                   \
        else if (td->GetType() == DatumType::Count || td->GetType() == DatumType::DatType)  \
        {                                                                                   \
            td->Set##DatType(value);                                                        \
        }                                                                                   \
        else                                                                                \
        {                                                                                   \
            LogWarning("SetField() failed because of mismatching types");                   \
        }                                                                                   \
    }                                                                                       \
}

DEFINE_SET_FIELD(const char*, Integer, int32_t)
DEFINE_SET_FIELD(const char*, Float, float)
DEFINE_SET_FIELD(const char*, Bool, bool)
DEFINE_SET_FIELD(const char*, String, const std::string&)
DEFINE_SET_FIELD(const char*, Vector2D, glm::vec2)
DEFINE_SET_FIELD(const char*, Vector, glm::vec3)
DEFINE_SET_FIELD(const char*, Color, glm::vec4)
DEFINE_SET_FIELD(const char*, Asset, Asset*)
DEFINE_SET_FIELD(const char*, Pointer, RTTI*)
DEFINE_SET_FIELD(const char*, Function, const ScriptFunc&)

DEFINE_SET_FIELD(int32_t, Integer, int32_t)
DEFINE_SET_FIELD(int32_t, Float, float)
DEFINE_SET_FIELD(int32_t, Bool, bool)
DEFINE_SET_FIELD(int32_t, String, const std::string&)
DEFINE_SET_FIELD(int32_t, Vector2D, glm::vec2)
DEFINE_SET_FIELD(int32_t, Vector, glm::vec3)
DEFINE_SET_FIELD(int32_t, Color, glm::vec4)
DEFINE_SET_FIELD(int32_t, Asset, Asset*)
DEFINE_SET_FIELD(int32_t, Pointer, RTTI*)
DEFINE_SET_FIELD(int32_t, Function, const ScriptFunc&)

void Datum::SetTableField(const char* key, const TableDatum& value)
{
    if (mType == DatumType::Table || mType == DatumType::Count)
    {
        TableDatum* td = FindTableDatum(key);
        if (td == nullptr)
        {
            td = PushBackTableDatum(TableDatum());
            td->SetStringKey(key);
            td->SetTableDatum(value);
        }
        else if (td->GetType() == DatumType::Count || td->GetType() == DatumType::Table)
        {
            td->SetTableDatum(value);
        }
        else
        {
            LogWarning("Failed to SetTableField() because of mismatching type");
        }
    }
}

void Datum::SetTableField(int32_t key, const TableDatum& value)
{
    if (mType == DatumType::Table || mType == DatumType::Count)
    {
        TableDatum* td = FindTableDatum(key);
        if (td == nullptr)
        {
            td = PushBackTableDatum(TableDatum());
            td->SetIntegerKey(key);
            td->SetTableDatum(value);
        }
        else if (td->GetType() == DatumType::Count || td->GetType() == DatumType::Table)
        {
            td->SetTableDatum(value);
        }
        else
        {
            LogWarning("Failed to SetTableField() because of mismatching type");
        }
    }
}

bool Datum::HasField(const char* key)
{
    return (mType == DatumType::Table && FindTableDatum(key) != nullptr);
}

bool Datum::HasField(int32_t key)
{
    return (mType == DatumType::Table && FindTableDatum(key) != nullptr);
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

Datum& Datum::operator=(RTTI* src)
{
    PreAssign(DatumType::Pointer);
    mData.p[0] = src;
    return *this;
}

Datum& Datum::operator=(int16_t src)
{
    PreAssign(DatumType::Short);
    mData.sh[0] = src;
    return *this;
}

Datum& Datum::operator=(const ScriptFunc& src)
{
    PreAssign(DatumType::Function);
    mData.fn[0] = src;
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
        OCT_ASSERT(mType != DatumType::Count);

        switch (mType)
        {
        case DatumType::String:
            if (mData.s[i] != other.mData.s[i])
            {
                return false;
            }
            break;
        case DatumType::Asset:
            if (mData.as[i] != other.mData.as[i])
            {
                return false;
            }
            break;
        case DatumType::Table:
            if (mData.t[i] != other.mData.t[i])
            {
                return false;
            }
            break;
        case DatumType::Function:
            if (mData.fn[i] != other.mData.fn[i])
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

bool Datum::operator==(const uint32_t& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Integer)
    {
        return false;
    }

    return mData.i[0] == (int32_t)other;
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

bool Datum::operator==(const uint8_t& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Byte)
    {
        return false;
    }

    return mData.by[0] == other;
}

bool Datum::operator==(const RTTI*& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Pointer)
    {
        return false;
    }

    return mData.p[0] == other;
}

bool Datum::operator==(const int16_t& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Short)
    {
        return false;
    }

    return mData.sh[0] == other;
}

bool Datum::operator==(const ScriptFunc& other) const
{
    if (mCount == 0 ||
        mType != DatumType::Function)
    {
        return false;
    }

    return mData.fn[0] == other;
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

bool Datum::operator!=(const RTTI*& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const int16_t& other) const
{
    return !operator==(other);
}

bool Datum::operator!=(const ScriptFunc& other) const
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
            OCT_ASSERT(0);
        }

        if (mType == DatumType::Count)
        {
            LogError("Cannot reserve internal memory for a typeless Datum.");
            OCT_ASSERT(0);
        }

        DatumData prevData = mData;

        mCapacity = capacity;
        uint32_t typeSize = GetDataTypeSize();
        mData.vp = SYS_AlignedMalloc(mCapacity * typeSize, 4);

        if (prevData.vp != nullptr)
        {
            if (mType == DatumType::String ||
                mType == DatumType::Asset ||
                mType == DatumType::Table ||
                mType == DatumType::Function)
            {
                for (uint32_t i = 0; i < mCount; ++i)
                {
                    ConstructData(mData, i);
                    CopyData(mData, i, prevData, i);
                    DestructData(prevData, i);
                }
            }
            else
            {
                // For primitive types, a dumb memcpy will be fine.
                memcpy(mData.vp, prevData.vp, typeSize * mCount);
            }

            SYS_AlignedFree(prevData.vp);
            prevData.vp = nullptr;
        }
    }
}

void Datum::Destroy()
{
    // Delete internal memory.
    if (!mExternal && 
        mData.vp != nullptr)
    {
        OCT_ASSERT(mType != DatumType::Count);
        OCT_ASSERT(mCount > 0);

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
    mForceScriptArray = src.mForceScriptArray;
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
            case DatumType::Byte:
                PushBack(*(src.mData.by + i));
                break;
            case DatumType::Table:
                PushBackTableDatum(*(src.mData.t + i));
                break;
            case DatumType::Pointer:
                PushBack(*(src.mData.p + i));
                break;
            case DatumType::Short:
                PushBack(*(src.mData.sh + i));
                break;
            case DatumType::Function:
                PushBack(*(src.mData.fn + i));
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

void* Datum::GetValue(uint32_t index)
{
    char* charData = (char*)mData.vp;
    void* retData = charData + index * GetDataTypeSize();
    return retData;
}

void Datum::PreSet(uint32_t index, DatumType type)
{
    if (index >= mCount)
    {
        LogError("Index out of bounds - Datum::Set().");
        OCT_ASSERT(0);
    }

    if (mType != type)
    {
        LogError("Type mismatch - Datum::Set()");
        OCT_ASSERT(0);
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
        OCT_ASSERT(0);
    }

    if (!mExternal &&
        (mCapacity > 0 ||
        mCount > 0))
    {
        LogError("Cannot set storage on Datum with internal storage already allocated.");
        OCT_ASSERT(0);
    }
}

void Datum::PostSetExternal(DatumType type, uint32_t count)
{
    OCT_ASSERT(type != DatumType::Count);
    OCT_ASSERT(type == mType);

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
        OCT_ASSERT(0);
    }

    if (mType == DatumType::Count)
    {
        SetType(type);
    }

    if (mType != type)
    {
        LogError("Datum::PushBack() - Type mismatch");
        OCT_ASSERT(0);
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
        OCT_ASSERT(0);
    }

    if (mType != type)
    {
        LogError("Type mismatch - Propterty::Get()");
        OCT_ASSERT(0);
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
        OCT_ASSERT(0);
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
    case DatumType::Byte:
        dataUnion.by[index] = 0;
        break;
    case DatumType::Table:
        new (dataUnion.t + index) TableDatum();
        break;
    case DatumType::Pointer:
        dataUnion.p[index] = nullptr;
        break;
    case DatumType::Short:
        dataUnion.sh[index] = 0;
        break;
    case DatumType::Function:
        new (dataUnion.fn + index) ScriptFunc();
        break;

    case DatumType::Count:
        OCT_ASSERT(0);
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
        // WARNING! I don't understand why this is happening, but invoking the TableDatum destructor
        // is causing a crash on GCN / 3DS. If I invoke the ~Datum() destructor then it doesn't crash.
        // Might have to see if this happens on Linux. Maybe valgrind will provide helpful insight.
        // ~TableDatum() is still called when invoking ~Datum(), so I'm just leaving it this way for now.

        //dataUnion.t[index].TableDatum::~TableDatum();
        dataUnion.t[index].Datum::~Datum();

        break;
    case DatumType::Function:
        dataUnion.fn[index].ScriptFunc::~ScriptFunc();
        break;

    default: break;
    }
}

void Datum::CopyData(DatumData& dst, uint32_t dstIndex, DatumData& src, uint32_t srcIndex)
{
    switch (mType)
    {
    case DatumType::String:
        dst.s[dstIndex] = src.s[srcIndex];
        break;
    case DatumType::Asset:
        dst.as[dstIndex] = src.as[srcIndex];
        break;
    case DatumType::Table:
        dst.t[dstIndex] = src.t[srcIndex];
        break;
    case DatumType::Function:
        dst.fn[dstIndex] = src.fn[srcIndex];
        break;

    default: 
        memcpy(dst.by + (dstIndex * GetDataTypeSize()), src.by + (srcIndex * GetDataTypeSize()), GetDataTypeSize());
        break;
    }
}

void Datum::Reset()
{
    mType = DatumType::Count;
    mExternal = false;
    mForceScriptArray = false;
    mOwner = nullptr;
    mData.vp = nullptr;
    mChangeHandler = nullptr;
    mCount = 0;
    mCapacity = 0;
}
