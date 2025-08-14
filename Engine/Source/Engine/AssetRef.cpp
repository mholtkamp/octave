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

class AssetRefLock
{
public :

    MutexObject* mMutex = nullptr;

    AssetRefLock()
    {
        mMutex = SYS_CreateMutex();
    }

    ~AssetRefLock()
    {
        SYS_DestroyMutex(mMutex);
        mMutex = nullptr;
    }
};

MutexObject* GetAssetRefMutex()
{
    static AssetRefLock sLock;
    return sLock.mMutex;
}

#if ASSET_LIVE_REF_TRACKING
// Need a mutex so that newly added AssetRefs during async load won't mess up operations.
static MutexObject* GetLiveRefMutex()
{
    static MutexObject* sLiveRefMutex = SYS_CreateMutex();
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
}

void AssetRef::EraseReferencesToAsset(Asset* asset)
{
    ReplaceReferencesToAsset(asset, nullptr);
}

void AssetRef::AddLiveRef(AssetRef* ref)
{
    if (!IsShuttingDown())
    {
        SCOPED_LOCK(GetLiveRefMutex());

        // Ensure that we are not adding this ref a second time to the list.
        OCT_ASSERT(GetLiveRefMap().find(ref) == GetLiveRefMap().end());

        GetLiveRefMap().insert(ref);
    }
}

void AssetRef::RemoveLiveRef(AssetRef* ref)
{
    if (!IsShuttingDown())
    {
        SCOPED_LOCK(GetLiveRefMutex());

        GetLiveRefMap().erase(ref);
    }
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

    if (src.mLoadRequest)
    {
        SetLoadRequest(src.mLoadRequest);
    }

    if (mAsset != nullptr)
    {
        mAsset->IncrementRefCount();
    }
}

AssetRef::~AssetRef()
{
    // Make sure an async load request isn't referencing deleted memory
    if (mLoadRequest != nullptr)
    {
        SetLoadRequest(nullptr);
    }

    AssetManager* am = AssetManager::Get();
    bool purging = am && am->IsPurging();
    bool decrement = !(purging || IsShuttingDown());

    if (decrement && mAsset != nullptr)
    {
        mAsset->DecrementRefCount();
    }

#if ASSET_LIVE_REF_TRACKING
    RemoveLiveRef(this);
#endif
}

AssetRef& AssetRef::operator=(const AssetRef& src)
{
    operator=(src.mAsset);

    if (src.mLoadRequest)
    {
        SetLoadRequest(src.mLoadRequest);
    }

    return *this;
}

AssetRef& AssetRef::operator=(const Asset* srcAsset)
{
    AssetManager* am = AssetManager::Get();
    bool purging = am && am->IsPurging();
    bool decrement = !(purging || IsShuttingDown());

    if (decrement && mAsset != nullptr)
    {
        mAsset->DecrementRefCount();
    }

    SetLoadRequest(nullptr);

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

void AssetRef::SetLoadRequest(AsyncLoadRequest* loadRequest)
{
    SCOPED_LOCK(GetAssetRefMutex());
    if (mLoadRequest != loadRequest)
    {
        if (mLoadRequest != nullptr)
        {
            EraseAsyncLoadRef();
        }

        mLoadRequest = loadRequest;

        if (mLoadRequest)
        {
            mLoadRequest->mTargetRefs.push_back(this);
        }
    }
}

AsyncLoadRequest* AssetRef::GetLoadRequest()
{
    AsyncLoadRequest* request = mLoadRequest;
    return request;
}

void AssetRef::EraseAsyncLoadRef()
{
    // Mutex should be locked priority to calling
    if (mLoadRequest != nullptr)
    {
        std::vector<AssetRef*>& refs = mLoadRequest->mTargetRefs;
        for (int32_t r = int32_t(refs.size()) - 1; r >= 0; --r)
        {
            if (refs[r] == this)
            {
                refs.erase(refs.begin() + r);
            }
        }

        mLoadRequest = nullptr;
    }
}
