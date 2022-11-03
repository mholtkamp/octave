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

#if ASSET_REF_VECTOR
std::vector<AssetRef*> AssetRef::sLiveAssetRefs;

void AssetRef::ReplaceReferencesToAsset(Asset* oldAsset, Asset* newAsset)
{
    if (oldAsset != nullptr)
    {
        for (uint32_t i = 0; i < sLiveAssetRefs.size(); ++i)
        {
            if (sLiveAssetRefs[i]->mAsset == oldAsset)
            {
                (*sLiveAssetRefs[i]) = newAsset;
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
    // Ensure that we are not adding this ref a second time to the list.
    for (uint32_t i = 0; i < sLiveAssetRefs.size(); ++i)
    {
        if (sLiveAssetRefs[i] == ref)
        {
            LogError("Duplicate Live Asset Ref");
            OCT_ASSERT(0);
        }
    }

    sLiveAssetRefs.push_back(ref);
}

void AssetRef::RemoveLiveRef(AssetRef* ref)
{
    for (uint32_t i = 0; i < sLiveAssetRefs.size(); ++i)
    {
        if (sLiveAssetRefs[i] == ref)
        {
            sLiveAssetRefs.erase(sLiveAssetRefs.begin() + i);
            break;
        }
    }
}
#endif

AssetRef::AssetRef()
{
#if ASSET_REF_VECTOR
    AddLiveRef(this);
#endif
}

AssetRef::AssetRef(Asset* asset) :
    mAsset(asset)
{
#if ASSET_REF_VECTOR
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

#if ASSET_REF_VECTOR
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

#if ASSET_REF_VECTOR
    RemoveLiveRef(this);
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
