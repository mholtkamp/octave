#pragma once

#include "Asset.h"
#include "Constants.h"
#include <vector>
#include "Assertion.h"

#include <unordered_set>

//class Asset;
class AssetManager;
struct AsyncLoadRequest;

class AssetRef
{
public:

    friend AssetManager;

    AssetRef();
    AssetRef(Asset* asset);
    AssetRef(const AssetRef& src);
    ~AssetRef();
    AssetRef& operator=(const AssetRef& src);
    AssetRef& operator=(const Asset* srcAsset);

    bool operator==(const AssetRef& other) const;
    bool operator!=(const AssetRef& other) const;
	bool operator==(const Asset* other) const;
	bool operator!=(const Asset* other) const;

    Asset* Get() const;

    template<typename T>
    T* Get() const
    {
        OCT_ASSERT(!mAsset || mAsset->GetType() == T::GetStaticType() || mAsset->Is(T::ClassRuntimeId()));
        return static_cast<T*>(Get());
    }

private:

    Asset* mAsset = nullptr;
    AsyncLoadRequest* mLoadRequest = nullptr;

#if ASSET_LIVE_REF_TRACKING
public:
    static void ReplaceReferencesToAsset(Asset* oldAsset, Asset* newAsset);
    static void EraseReferencesToAsset(Asset* asset);
private:
    static void AddLiveRef(AssetRef* ref);
    static void RemoveLiveRef(AssetRef* ref);
#endif

};

typedef AssetRef TextureRef;
typedef AssetRef StaticMeshRef;
typedef AssetRef MaterialRef;
typedef AssetRef SkeletalMeshRef;
typedef AssetRef ParticleSystemRef;
typedef AssetRef SoundWaveRef;
typedef AssetRef FontRef;
typedef AssetRef SceneRef;
