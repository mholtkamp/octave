#include "AssetRef.h"
#include "Asset.h"
#include "Engine.h"

#include "Assets/StaticMesh.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/Texture.h"
#include "Assets/ParticleSystem.h"
#include "Assets/SoundWave.h"

#include "AssetManager.h"

#include "Assertion.h"

#if ASSET_LIVE_REF_TRACKING
// Need a mutex so that newly added AssetRefs during async load won't mess up operations.
static MutexHandle GetLiveRefMutex()
{
    static MutexHandle sLiveRefMutex = SYS_CreateMutex();
    return sLiveRefMutex;
}

static std::unordered_set<AssetRef*>& GetLiveRefMap()
{
    static std::unordered_set<AssetRef*> sLiveAssetRefs;
    return sLiveAssetRefs;
}

void AssetRef::ReplaceReferencesToAsset(Asset* oldAsset, Asset* newAsset)
{
    SCOPED_LOCK(GetLiveRefMutex());

    if (oldAsset != nullptr)
    {
        for (AssetRef* ref : GetLiveRefMap())
        {
            if (ref->Get() == oldAsset)
            {
                *ref = newAsset;
            }
        }
    }

    // If a texture asset was replaced, it could mean that a material needs to update
    // its descriptor tables (for vulkan). So for now, dirty all materials. Other options could be
    // only dirtying materials that are using the texture, or using a global dirty flag or something.
#if API_VULKAN
    // It's possible that this DirtyAllMaterials() func is called during AssetManager construction
    // so the instance may not be available yet, so check first.
    if (AssetManager::Get() != nullptr &&
        !AssetManager::Get()->IsPurging())
    {
        AssetManager::Get()->DirtyAllMaterials();
    }
#endif
}

void AssetRef::EraseReferencesToAsset(Asset* asset)
{
    ReplaceReferencesToAsset(asset, nullptr);
}

void AssetRef::AddLiveRef(AssetRef* ref)
{
    SCOPED_LOCK(GetLiveRefMutex());

    // Ensure that we are not adding this ref a second time to the list.
    OCT_ASSERT(GetLiveRefMap().find(ref) == GetLiveRefMap().end());

    GetLiveRefMap().insert(ref);
}

void AssetRef::RemoveLiveRef(AssetRef* ref)
{
    SCOPED_LOCK(GetLiveRefMutex());

    GetLiveRefMap().erase(ref);
}
#endif

AssetRef::AssetRef()
{
#if ASSET_LIVE_REF_TRACKING
    AddLiveRef(this);
#endif
}

AssetRef::AssetRef(Asset* asset) :
    mAsset(asset)
{
#if ASSET_LIVE_REF_TRACKING
    AddLiveRef(this);
#endif

    if (mAsset != nullptr)
    {
        mAsset->IncrementRefCount();
    }
}

AssetRef::AssetRef(const AssetRef& src)
{
    mAsset = src.mAsset;

#if ASSET_LIVE_REF_TRACKING
    AddLiveRef(this);
#endif

    if (mAsset != nullptr)
    {
        mAsset->IncrementRefCount();
    }

    if (mLoadRequest != nullptr)
    {
        AssetManager::Get()->EraseAsyncLoadRef(*this);
        mLoadRequest = nullptr;
    }
}

AssetRef::~AssetRef()
{
    // Make sure an async load request isn't referencing deleted memory
    if (mLoadRequest != nullptr)
    {
        AssetManager::Get()->EraseAsyncLoadRef(*this);
        mLoadRequest = nullptr;
    }

    if (!IsShuttingDown() && mAsset != nullptr)
    {
        mAsset->DecrementRefCount();
    }

#if ASSET_LIVE_REF_TRACKING
    if (!IsShuttingDown())
    {
        RemoveLiveRef(this);
    }
#endif
}

AssetRef& AssetRef::operator=(const AssetRef& src)
{
    return operator=(src.mAsset);
}

AssetRef& AssetRef::operator=(const Asset* srcAsset)
{
    if (mAsset != nullptr)
    {
        mAsset->DecrementRefCount();
    }

    mAsset = const_cast<Asset*>(srcAsset);

    // Increment the new asset if it's valid.
    if (mAsset != nullptr)
    {
        mAsset->IncrementRefCount();
    }

    return *this;
}

bool AssetRef::operator==(const AssetRef& other) const
{
    return mAsset == other.mAsset;
}

bool AssetRef::operator!=(const AssetRef& other) const
{
    return !operator==(other);
}

bool AssetRef::operator==(const Asset* other) const
{
    return (mAsset == other);
}

bool AssetRef::operator!=(const Asset* other) const
{
    return (mAsset != other);
}

Asset* AssetRef::Get() const
{
    // Only return non-nullptr if asset is loaded.
    //return mAsset->IsLoaded() ? mAsset : nullptr;
    return mAsset;
}
