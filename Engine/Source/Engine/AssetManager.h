#pragma once

#include "Asset.h"
#include "AssetRef.h"
#include "Log.h"

#include "System/System.h"

#include <string>
#include <deque>
#include <unordered_map>

class Asset;
class AssetDir;
class Material;
class ParticleSystem;

struct AsyncLoadRequest
{
    std::string mName;
    std::string mPath;
    std::vector<AssetRef*> mTargetRefs;
    std::vector<AssetStub*> mDependentAssets;
    const EmbeddedFile* mEmbeddedData = nullptr;
    TypeId mType = INVALID_TYPE_ID;
    Asset* mAsset = nullptr;
    int32_t mRequeueCount = 0;
};

Asset* FetchAsset(const std::string& name);
Asset* LoadAsset(const std::string& name);
void UnloadAsset(const std::string& name);
void AsyncLoadAsset(const std::string& name, AssetRef* targetRef = nullptr);
AssetStub* FetchAssetStub(const std::string& name);

template<typename T>
T* FetchAsset(const std::string& name)
{
    Asset* asset = FetchAsset(name);

    if (asset != nullptr &&
        asset->GetType() != T::GetStaticType())
    {
        LogError("Type mismatch in FetchAsset<T>()");
        asset = nullptr;
    }

    return (T*)asset;
}

template<typename T>
T* LoadAsset(const std::string& name)
{
    Asset* asset = LoadAsset(name);

    if (asset != nullptr &&
        !asset->Is(T::ClassRuntimeId()))
    {
        LogError("Type mismatch in LoadAsset<T>()");
        asset = nullptr;
    }

    return (T*)asset;
}

class AssetManager
{
public:

    ~AssetManager();

    static void Create();
    static void Destroy();
    static AssetManager* Get();

    void Initialize();
    void Update(float deltaTime);
    void DiscoverDirectory(AssetDir* directory, bool engineDir);
    void Discover(const char* directoryName, const char* directoryPath);
    void DiscoverAssetRegistry(const char* registryPath);
    void DiscoverEmbeddedAssets(struct EmbeddedFile* assets, uint32_t numAssets);
    void Purge(bool purgeEngineAssets);
    bool PurgeAsset(const char* name);
    void RefSweep();
    void LoadAll();

    void RegisterTransientAsset(Asset* asset);

    Asset* ImportEngineAsset(TypeId assetType, AssetDir* dir, const std::string& filename, ImportOptions* options = nullptr);
    void ImportEngineAssets();

    AssetStub* GetAssetStub(const std::string& name);
    Asset* GetAsset(const std::string& name);
    Asset* LoadAsset(const std::string& name);
    Asset* LoadAsset(AssetStub& stub);
    void AsyncLoadAsset(const std::string& name, AssetRef* targetRef);
    void SaveAsset(const std::string& name);
    void SaveAsset(AssetStub& stub);
    bool UnloadAsset(const std::string& name);
    bool UnloadAsset(AssetStub& stub);

    bool DoesAssetExist(const std::string& name);
    bool RenameAsset(Asset* asset, const std::string& newName);
    bool RenameDirectory(AssetDir* dir, const std::string& newName);
    AssetDir* FindProjectDirectory();
    AssetDir* FindEngineDirectory();
    AssetDir* GetRootDirectory();
    void UnloadProjectDirectory();
    std::unordered_map<std::string, AssetStub*>& GetAssetMap();

    AssetStub* RegisterAsset(const std::string& filename, TypeId type, AssetDir* directory, EmbeddedFile* embeddedAsset, bool engineAsset);
    AssetStub* CreateAndRegisterAsset(TypeId assetType, AssetDir* directory, const std::string& filename, bool engineAsset);
    AssetDir* GetAssetDirFromPath(const std::string& dirPath);

    bool IsPurging() const;

protected:

    static ThreadFuncRet AsyncLoadThreadFunc(void* in);

    static AssetManager* sInstance;
    AssetManager();

    void UpdateEndLoadQueue();

    std::unordered_map<std::string, AssetStub*> mAssetMap;
    std::vector<Asset*> mTransientAssets;
    AssetDir* mRootDirectory = nullptr;
    bool mPurging = false;
    bool mDestructing = false;
    std::deque<AsyncLoadRequest*> mBeginLoadQueue;
    std::deque<AsyncLoadRequest*> mEndLoadQueue;
    ThreadObject* mAsyncLoadThread = {};
    MutexObject* mMutex = {};

#if EDITOR
public:
    glm::vec4 GetEditorAssetColor(TypeId type);
    void InitAssetColorMap();
protected:
    std::unordered_map<TypeId, glm::vec4> mAssetColorMap;
#endif
};

template<typename T>
T* NewTransientAsset()
{
    T* ret = new T();
    AssetManager::Get()->RegisterTransientAsset(ret);
    return ret;

    // Caller still needs to call Create() when ready!
}
