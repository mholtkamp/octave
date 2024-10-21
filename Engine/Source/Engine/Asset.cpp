#include "Asset.h"
#include "Stream.h"
#include "Property.h"
#include "AssetDir.h"
#include "Engine.h"
#include "AssetManager.h"
#include "Log.h"
#include "Utilities.h"
#include "Assets/Scene.h"
#include "Assets/MaterialLite.h"

#include "Assertion.h"

DEFINE_FACTORY_MANAGER(Asset);
DEFINE_FACTORY(Asset, Asset);
DEFINE_RTTI(Asset);

bool HandleAssetNamePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    // Handle Name property changes manually.
    OCT_ASSERT(prop->mType == DatumType::String);
    OCT_ASSERT(prop->mName == "Name");
    std::string newName = *reinterpret_cast<const std::string*>(newValue);
    Asset* asset = reinterpret_cast<Asset*>(prop->mOwner);
    AssetManager::Get()->RenameAsset(asset, newName);
    return true;
}

bool ImportOptions::HasOption(const std::string& key)
{
    auto it = mOptions.find(key);
    bool hasOption = (it != mOptions.end());
    return hasOption;
}

Datum ImportOptions::GetOptionValue(const std::string& key)
{
    Datum ret;
    if (HasOption(key))
    {
        ret = mOptions[key];
    }

    return ret;
}

void ImportOptions::SetOptionValue(const std::string& key, const Datum& value)
{
    mOptions[key] = value;
}

Asset::Asset()
{

}

Asset::~Asset()
{

}

void Asset::Create()
{
    OCT_ASSERT(!mLoaded);
    mLoaded = true;
}

void Asset::Destroy()
{
#if ASSET_LIVE_REF_TRACKING
    if (!IsShuttingDown())
    {
        AssetRef::EraseReferencesToAsset(this);
    }
#endif
}

void Asset::Copy(Asset* srcAsset)
{
    if (srcAsset == nullptr ||
        srcAsset->GetType() != GetType())
    {
        LogError("Failed to copy actor");
        return;
    }

    Stream stream;
    srcAsset->SaveStream(stream, Platform::Count);
    stream.SetPos(0);
    LoadStream(stream, Platform::Count);
}

const std::string& Asset::GetName() const
{
    return mName;
}

bool Asset::IsLoaded() const
{
    return mLoaded;
}

bool Asset::IsRefCounted() const
{
    return mEnableRefCount;
}

int32_t Asset::GetRefCount() const
{
    return mRefCount;
}

bool Asset::IsEmbedded() const
{
    return mEmbedded;
}

void Asset::SetEmbedded(bool embed)
{
    mEmbedded = embed;
}

bool Asset::IsEngineAsset() const
{
    return mEngineAsset;
}

void Asset::SetEngineAsset(bool engineAsset)
{
    mEngineAsset = engineAsset;
}

void Asset::SetName(const std::string& name)
{
    mName = name;
}

void Asset::IncrementRefCount()
{
    //if (mRefCount == 0 &&
    //    !mLoaded &&
    //    mEnableRefCount)
    //{
    //    Load();
    //}

    mRefCount++;
}

void Asset::DecrementRefCount()
{
    mRefCount--;
    OCT_ASSERT(mRefCount >= 0 || AssetManager::Get()->IsPurging());
}

void Asset::LoadFile(const char* path, AsyncLoadRequest* request)
{
    if (IsLoaded())
        return;

    Stream stream;
    stream.SetAsyncRequest(request);
    stream.ReadFile(path, true);
    LoadStream(stream, GetPlatform());

    // Only "finish" the load if not async.
    if (request == nullptr)
    {
        Create();
    }

    LogDebug("Asset loaded: %s", mName.c_str());
}

void Asset::SaveFile(const char* path, Platform platform)
{
#if EDITOR
    Stream stream;
    SaveStream(stream, platform);
    stream.WriteFile(path);
    LogDebug("Asset saved: %s", mName.c_str());
#endif
}

void Asset::LoadEmbedded(const EmbeddedFile* embeddedAsset, AsyncLoadRequest* request)
{
    Stream stream(embeddedAsset->mData, embeddedAsset->mSize);
    LoadStream(stream, GetPlatform());
    SetEmbedded(true);

    // Only "finish" the load if not async.
    if (request == nullptr)
    {
        Create();
    }

    LogDebug("Asset loaded: %s", mName.c_str());
}

void Asset::LoadStream(Stream& stream, Platform platform)
{
    AssetHeader header = ReadHeader(stream);
    mVersion = header.mVersion;
    mType = header.mType;
    mEmbedded = header.mEmbedded;

    stream.ReadString(mName);
}

void Asset::SaveStream(Stream& stream, Platform platform)
{
#if EDITOR
    WriteHeader(stream);
    stream.WriteString(mName);
#endif
}

void Asset::Import(const std::string& path, ImportOptions* options)
{

}

void Asset::GatherProperties(std::vector<Property>& outProps)
{
    outProps.push_back(Property(DatumType::String, "Name", this, &mName, 1, HandleAssetNamePropChange));
}

glm::vec4 Asset::GetTypeColor()
{
    return glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
}

const char* Asset::GetTypeName()
{
    return "Asset";
}

const char* Asset::GetTypeImportExt()
{
    return "";
}

bool Asset::IsTransient() const
{
    return mTransient;
}

void Asset::SetTransient(bool transient)
{
    mTransient = transient;
}

AssetHeader Asset::ReadHeader(Stream& stream)
{
    AssetHeader header;
    header.mMagic = stream.ReadUint32();
    header.mVersion = stream.ReadUint32();
    header.mType = TypeId(stream.ReadUint32());
    header.mEmbedded = stream.ReadUint8();

    return header;
}

void Asset::WriteHeader(Stream& stream)
{
    stream.WriteUint32(ASSET_MAGIC_NUMBER);
    stream.WriteUint32(ASSET_VERSION_CURRENT);
    stream.WriteUint32(uint32_t(mType));
    stream.WriteUint8(mEmbedded);
}


std::string Asset::GetNameFromPath(const std::string& path)
{
    std::string retName = path;
    size_t slashLoc = path.find_last_of('/');
    if (slashLoc != std::string::npos)
    {
        retName = retName.substr(slashLoc + 1, std::string::npos);
    }

    retName = retName.substr(0, retName.find_last_of('.'));

    return retName;
}

std::string Asset::GetDirectoryFromPath(const std::string& path)
{
    std::string retDir = path;
    size_t slashLoc = retDir.find_last_of('/');
    if (slashLoc != std::string::npos)
    {
        retDir = retDir.substr(0, slashLoc + 1);
    }
    else
    {
        retDir = "./";
    }

    return retDir;
}

const char* Asset::GetNameFromTypeId(TypeId id)
{
    // TODO: Possibly move this into Factory.h
    const char* retName = "Asset";
    std::vector<Factory*>& factoryList = GetFactoryList();

    for (uint32_t i = 0; i < factoryList.size(); ++i)
    {
        if (id == factoryList[i]->GetType())
        {
            retName = factoryList[i]->GetClassName();
            break;
        }
    }

    return retName;
}

TypeId Asset::GetTypeIdFromName(const char* name)
{
    // TODO: Possibly move this into Factory.h
    TypeId retId = INVALID_TYPE_ID;
    std::vector<Factory*>& factoryList = GetFactoryList();

    for (uint32_t i = 0; i < factoryList.size(); ++i)
    {
        if (strcmp(name, factoryList[i]->GetClassName()) == 0)
        {
            retId = factoryList[i]->GetType();
            break;
        }
    }

    return retId;
}

