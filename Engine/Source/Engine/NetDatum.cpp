#include "NetDatum.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Log.h"

#include "Components/ScriptComponent.h"

#include <string.h>

NetDatum::NetDatum()
{

}

NetDatum::NetDatum(
    DatumType type,
    void* owner,
    void* data,
    uint32_t count,
    DatumChangeHandlerFP changeHandler,
    bool alwaysReplicate) : 
    Datum(type, owner, data, count, changeHandler)
{
    mAlwaysReplicate = alwaysReplicate;
}

bool NetDatum::ShouldReplicate() const
{
    // Only replicate if the data has changed
    if (mType == DatumType::Count)
        return false;

    // NetDatum count should never change after init.
    OCT_ASSERT(mPrevCount == 0 ||
           mPrevCount == mCount);

    if (mAlwaysReplicate)
        return true;

    if (mCount != mPrevCount)
        return true;

    bool equal = true;
    for (uint32_t i = 0; i < mCount; ++i)
    {
        switch (mType)
        {
            case DatumType::Integer: equal = (mPrevData.i[i] == mData.i[i]); break;
            case DatumType::Float: equal = (mPrevData.f[i] == mData.f[i]); break;
            case DatumType::Bool: equal = (mPrevData.b[i] == mData.b[i]); break;
            case DatumType::String: equal = (mPrevData.s[i] == mData.s[i]); break;
            case DatumType::Vector2D: equal = (mPrevData.v2[i] == mData.v2[i]); break;
            case DatumType::Vector: equal = (mPrevData.v3[i] == mData.v3[i]); break;
            case DatumType::Color: equal = (mPrevData.v4[i] == mData.v4[i]); break;
            case DatumType::Asset: equal = (mPrevData.as[i] == mData.as[i]); break;
            case DatumType::Byte: equal = (mPrevData.by[i] == mData.by[i]); break;
            case DatumType::Table: equal = false; OCT_ASSERT(0); break; // Table not supported for replication
            case DatumType::Pointer: equal = false; OCT_ASSERT(0); break; // Pointers not supported for replication
            case DatumType::Short: equal = (mPrevData.sh[i] == mData.sh[i]); break;

            case DatumType::Count: break;
        }

        if (!equal)
        {
            break;
        }
    }

    return !equal;
}

void NetDatum::PostReplicate()
{
    OCT_ASSERT(mType != DatumType::Count);

    // NetDatums should never change size (except at initialization)
    // We could possible get rid off the mPrevCount member, but I'm using it
    // for sanity checking that the size doesn't change. It could be made Debug only?
    if (mPrevData.vp == nullptr)
    {
        OCT_ASSERT(mPrevCount == 0);
        mPrevData.vp = malloc(mCount * GetDataTypeSize());
        for (uint32_t i = 0; i < mCount; ++i)
        {
            ConstructData(mPrevData, i);
        }

        mPrevCount = mCount;
    }

    // Copy members over
    for (uint32_t i = 0; i < mCount; ++i)
    {
        switch (mType)
        {
            case DatumType::Integer: mPrevData.i[i] = mData.i[i]; break;
            case DatumType::Float: mPrevData.f[i] = mData.f[i]; break;
            case DatumType::Bool: mPrevData.b[i] = mData.b[i]; break;
            case DatumType::String: mPrevData.s[i] = mData.s[i]; break;
            case DatumType::Vector2D: mPrevData.v2[i] = mData.v2[i]; break;
            case DatumType::Vector: mPrevData.v3[i] = mData.v3[i]; break;
            case DatumType::Color: mPrevData.v4[i] = mData.v4[i]; break;
            case DatumType::Asset: mPrevData.as[i] = mData.as[i]; break;
            case DatumType::Byte: mPrevData.by[i] = mData.by[i]; break;
            case DatumType::Table: OCT_ASSERT(0); break; // Table not supported for replication
            case DatumType::Pointer: OCT_ASSERT(0); break; // Pointer not supported for replication
            case DatumType::Short: mPrevData.sh[i] = mData.sh[i]; break;

            case DatumType::Count: break;
        }
    }
}

void NetDatum::Destroy()
{
    if (mPrevData.vp != nullptr)
    {
        for (uint32_t i = 0; i < mPrevCount; ++i)
        {
            DestructData(mPrevData, i);
        }

        free(mPrevData.vp);
        mPrevData.vp = nullptr;
    }

    mPrevCount = 0;
}


ScriptNetDatum::ScriptNetDatum()
{

}

ScriptNetDatum::ScriptNetDatum(
    const char* varName,
    DatumType type,
    void* owner,
    void* data,
    uint32_t count,
    const char* onRepFuncName,
    bool alwaysReplicate) :
    NetDatum(type, owner, data, count, ScriptComponent::OnRepHandler , alwaysReplicate)
{
    mVarName = varName;
}
