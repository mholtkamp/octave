#pragma once

#include "EngineTypes.h"
#include "EmbeddedFile.h"
#include "Factory.h"
#include "RTTI.h"

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Stream;
class Property;
class AssetDir;

#define ASSET_MAGIC_NUMBER 0x4f435421
#define ASSET_CURRENT_VERSION 1

#define DECLARE_ASSET(Base, Parent) DECLARE_FACTORY(Base, Asset); DECLARE_RTTI(Base, Parent);
#define DEFINE_ASSET(Base) DEFINE_FACTORY(Base, Asset); DEFINE_RTTI(Base);

enum class AssetLoadState
{
    Unloaded,
    AwaitBegin,
    AwaitEnd,
    Loaded,

    Count
};

struct AssetHeader
{
    uint32_t mMagic = ASSET_MAGIC_NUMBER;
    uint32_t mVersion = ASSET_CURRENT_VERSION;
    TypeId mType = 0;
    uint8_t mEmbedded = false;
};

struct AssetStub
{
    Asset* mAsset = nullptr;
    const EmbeddedFile* mEmbeddedData = nullptr;
    std::string mPath;
    TypeId mType = INVALID_TYPE_ID;
    bool mEngineAsset = false;

#if EDITOR
    std::string mName;
    AssetDir* mDirectory = nullptr;
#endif
};

class Asset : public RTTI
{
public:

    DECLARE_FACTORY_MANAGER(Asset);
    DECLARE_FACTORY(Asset, Asset);
    DECLARE_RTTI(Asset, RTTI);

    Asset();
    virtual ~Asset();

    virtual void Create();
    virtual void Destroy();

    virtual void Copy(Asset* srcAsset);

    const std::string& GetName() const;
    bool IsLoaded() const;
    bool IsRefCounted() const;
    int32_t GetRefCount() const;

    bool IsEmbedded() const;
    void SetEmbedded(bool embed);

    bool IsEngineAsset() const;
    void SetEngineAsset(bool engineAsset);

    void SetName(const std::string& name);
    void IncrementRefCount();
    void DecrementRefCount();

    void LoadFile(const char* path, AsyncLoadRequest* request = nullptr);
    void LoadEmbedded(const EmbeddedFile* embeddedAsset, AsyncLoadRequest* request = nullptr);
    void SaveFile(const char* path, Platform platform);

    virtual void LoadStream(Stream& stream, Platform platform);
	virtual void SaveStream(Stream& stream, Platform platform);
    virtual void Import(const std::string& path);

    virtual void GatherProperties(std::vector<Property>& outProps);
    virtual glm::vec4 GetTypeColor();
    virtual const char* GetTypeName();
    virtual const char* GetTypeImportExt();
    virtual bool IsTransient() const;

    static AssetHeader ReadHeader(Stream& stream);
    void WriteHeader(Stream& stream);

    static std::string GetNameFromPath(const std::string& path);
    static std::string GetDirectoryFromPath(const std::string& path);

    static const char* GetNameFromTypeId(TypeId id);
    static TypeId GetTypeIdFromName(const char* name);

protected:

    uint32_t mVersion = 0;
    TypeId mType = INVALID_TYPE_ID;
    bool mEmbedded = false;
    bool mLoaded = false;
    bool mEnableRefCount = true;
    bool mEngineAsset = false;

    std::string mName = "Asset";
    int32_t mRefCount = 0;
};