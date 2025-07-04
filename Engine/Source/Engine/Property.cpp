#include "Property.h"
#include "TableDatum.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Log.h"
#include "Script.h"
#include "NodePath.h"

Property::Property()
{
    
}

Property::Property(
    DatumType type,
    const std::string& name,
    Object* owner,
    void* data,
    uint32_t count,
    DatumChangeHandlerFP changeHandler,
    Datum extra,
    int32_t enumCount,
    const char** enumStrings) :
    Datum(type, owner, data, count, changeHandler)
{
    mName = name;
    mEnumCount = enumCount;
    mEnumStrings = enumStrings;

    if (extra.IsValid())
    {
        CreateExtraData();
        *mExtra = extra;
    }

#if EDITOR
    mCategory = sCategory;
#endif
}

Property::~Property()
{
    DestroyExtraData();
}

Property::Property(const Property& src) :
    Datum(src)
{
    mName = src.mName;
    mEnumCount = src.mEnumCount;
    mEnumStrings = src.mEnumStrings;

    mVector = src.mVector;
    mMinCount = src.mMinCount;
    mMaxCount = src.mMaxCount;
    mIsVector = src.mIsVector;

    if (src.mExtra)
    {
        CreateExtraData();
        *mExtra = *src.mExtra;
    }

#if EDITOR
    mCategory = src.mCategory;
#endif
}

Property& Property::operator=(const Property& src)
{
    if (this != &src)
    {
        Datum::operator=(src);
    }

    return *this;
}

void Property::ReadStream(Stream& stream, uint32_t version, bool net, bool external)
{
    Datum::ReadStream(stream, version, net, external);
    stream.ReadString(mName);

    if (version >= ASSET_VERSION_PROPERTY_EXTRA)
    {
        bool hasExtra = stream.ReadBool();

        if (hasExtra)
        {
            CreateExtraData();
            mExtra->ReadStream(stream, version, net, external);
        }
    }
    else
    {
        CreateExtraData();
        mExtra->PushBack(stream.ReadInt32());
    }

    // We don't really need to write mIsVector since the values are copied
    // over to the script property which already has mIsVector set.
    //mIsVector = stream.ReadBool();
}

void Property::WriteStream(Stream& stream, bool net) const
{
    Datum::WriteStream(stream, net);

    if (!net)
    {
        if (mType == DatumType::Node)
        {
            CreateExtraData();

            for (uint32_t i = 0; i < mCount; ++i)
            {
                Node* srcNode = mOwner->As<Node>();
                if (srcNode == nullptr && mOwner->As<Script>())
                {
                    srcNode = mOwner->As<Script>()->GetOwner();
                }

                const WeakPtr<Node>& dstNode = mData.n[i];
                std::string nodePath = FindRelativeNodePath(srcNode, dstNode.Get());

                mExtra->PushBack(nodePath);
            }
        }
    }

    stream.WriteString(mName);

    stream.WriteBool(mExtra != nullptr);

    if (mExtra != nullptr)
    {
        mExtra->WriteStream(stream, net);
    }

    if (mType == DatumType::Node)
    {
        DestroyExtraData();
    }

    // We don't really need to write mIsVector since the values are copied
    // over to the script property which already has mIsVector set.
    //stream.WriteBool(mIsVector);
}

uint32_t Property::GetSerializationSize(bool net) const
{
    return Datum::GetSerializationSize(net) +
        GetStringSerializationSize(mName) +
        sizeof(uint8_t) + // Has extra?
        mExtra ? mExtra->GetSerializationSize(net) : 0;
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

        if (srcProp.mExtra)
        {
            CreateExtraData();
            *mExtra = *srcProp.mExtra;
        }
        else
        {
            DestroyExtraData();
        }

        mEnumCount = srcProp.mEnumCount;
        mEnumStrings = srcProp.mEnumStrings;

        mVector = srcProp.mVector;
        mMinCount = srcProp.mMinCount;
        mMaxCount = srcProp.mMaxCount;
        mIsVector = srcProp.mIsVector;
    }
}

void Property::Destroy()
{
    Datum::Destroy();
    DestroyExtraData();
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
        case DatumType::Node:
        {
            std::vector<NodePtrWeak>& vect = *((std::vector<NodePtrWeak>*) mVector);
            vect.push_back(value ? *((NodePtrWeak*)value) : NodePtrWeak());
            mData.n = vect.data();
            break;
        }
        case DatumType::Short:
        {
            std::vector<int16_t>& vect = *((std::vector<int16_t>*) mVector);
            vect.push_back(value ? *((int16_t*)value) : (int16_t)0);
            mData.sh = vect.data();
            break;
        }
        case DatumType::Function:
        {
            // Function not supported as vector.
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
    OCT_ASSERT((mExternal == (mVector != nullptr)));
    OCT_ASSERT(mIsVector);

    if (mExternal)
    {
        OCT_ASSERT(mVector != nullptr);
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
        case DatumType::Node:
        {
            std::vector<NodePtrWeak>& vect = *((std::vector<NodePtrWeak>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Short:
        {
            std::vector<int16_t>& vect = *((std::vector<int16_t>*) mVector);
            vect.erase(vect.begin() + index);
            break;
        }
        case DatumType::Function:
        {
            // Function not supported as Vector
            OCT_ASSERT(0);
            break;
        }

        default: break;
        }

        mCount--;
    }
    else
    {
        Erase(index);
    }
}

void Property::ResizeVector(uint32_t count)
{
    OCT_ASSERT((mExternal == (mVector != nullptr)));
    OCT_ASSERT(mIsVector);

    count = glm::clamp<uint32_t>(count, 0u, 255u);

    if (mExternal)
    {
        OCT_ASSERT(mVector != nullptr);

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
        case DatumType::Node:
        {
            std::vector<NodePtrWeak>& vect = *((std::vector<NodePtrWeak>*) mVector);
            vect.resize(count);
            mData.n = vect.data();
            break;
        }
        case DatumType::Short:
        {
            std::vector<int16_t>& vect = *((std::vector<int16_t>*) mVector);
            vect.resize(count);
            mData.sh = vect.data();
            break;
        }
        case DatumType::Function:
        {
            // Function not supported as Vector
            OCT_ASSERT(0);
            break;
        }

        default: break;
        }

        mCount = count;
    }
    else
    {
        SetCount(count);
    }
}

Property& Property::MakeVector(uint8_t minCount, uint8_t maxCount)
{
    mIsVector = true;
    mMinCount = minCount;
    mMaxCount = maxCount;

    if (mExternal)
    {
        OCT_ASSERT(mData.vp);
        OCT_ASSERT(!mVector);

        mVector = mData.vp;

        switch (mType)
        {
        case DatumType::Integer:
        {
            std::vector<int32_t>& vect = *((std::vector<int32_t>*) mVector);
            mData.i = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Float:
        {
            std::vector<float>& vect = *((std::vector<float>*) mVector);
            mData.f = vect.data();
            mCount = (uint8_t)vect.size();
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
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Vector2D:
        {
            std::vector<glm::vec2>& vect = *((std::vector<glm::vec2>*) mVector);
            mData.v2 = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Vector:
        {
            std::vector<glm::vec3>& vect = *((std::vector<glm::vec3>*) mVector);
            mData.v3 = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Color:
        {
            std::vector<glm::vec4>& vect = *((std::vector<glm::vec4>*) mVector);
            mData.v4 = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Asset:
        {
            std::vector<AssetRef>& vect = *((std::vector<AssetRef>*) mVector);
            mData.as = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Byte:
        {
            std::vector<uint8_t>& vect = *((std::vector<uint8_t>*) mVector);
            mData.by = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Table:
        {
            // Table not supported as vector
            OCT_ASSERT(0);
            break;
        }
        case DatumType::Node:
        {
            std::vector<NodePtrWeak>& vect = *((std::vector<NodePtrWeak>*) mVector);
            mData.n = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Short:
        {
            std::vector<int16_t>& vect = *((std::vector<int16_t>*) mVector);
            mData.sh = vect.data();
            mCount = (uint8_t)vect.size();
            break;
        }
        case DatumType::Function:
        {
            // Function not supported as vector
            OCT_ASSERT(0);
            break;
        }

        default: break;
        }
    }

    return *this;
}

bool Property::IsVector() const
{
    // External props should have mVector allocated if mIsVector is true.
    // Internal props should NOT have mVector allocated.
    OCT_ASSERT(!mIsVector || (mExternal == (mVector != nullptr)));
    return mIsVector;
}

bool Property::IsArray() const
{
    return (IsVector() || mCount > 1);
}

void Property::CreateExtraData() const
{
    if (mExtra == nullptr)
    {
        mExtra = new Datum();
    }
}

void Property::DestroyExtraData() const
{
    if (mExtra != nullptr)
    {
        delete mExtra;
        mExtra = nullptr;
    }
}

void Property::ResolveNodePaths()
{
    if (GetType() == DatumType::Node &&
        mExtra != nullptr &&
        mOwner != nullptr &&
        mOwner->As<Node>())
    {
        Node* ownerNode = mOwner->As<Node>();
        
        for (uint32_t i = 0; i < mCount; ++i)
        {
            const std::string& path = mExtra->GetString();
            Node* dstNode = ::ResolveNodePath(ownerNode, path);
            LogDebug("Resolve [%s] -> %p", path.c_str(), dstNode);
            SetNode(ResolveWeakPtr<Node>(dstNode), i);
        }

        // Now that paths have been resolved, free up the string data.
        DestroyExtraData();
    }
}

void Property::Reset()
{
    Datum::Reset();
    mName = "";
    mEnumCount = 0;
    mEnumStrings = nullptr;
    mVector = nullptr;
    mMinCount = 0;
    mMaxCount = 255;
    mIsVector = false;

    DestroyExtraData();
}


#if EDITOR

const char* Property::sCategory = "";

void Property::SetCategory(const char* category)
{
    sCategory = category;
}

void Property::ClearCategory()
{
    sCategory = "";
}

#endif