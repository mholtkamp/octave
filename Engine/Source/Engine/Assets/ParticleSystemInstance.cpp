
#include "Assets/ParticleSystemInstance.h"

#include "AssetManager.h"
#include "Asset.h"
#include "Assets/Material.h"

FORCE_LINK_DEF(ParticleSystemInstance);
DEFINE_ASSET(ParticleSystemInstance);

ParticleSystemInstance* ParticleSystemInstance::New(const ParticleSystem* src)
{
    ParticleSystemInstance* ret = NewTransientAsset<ParticleSystemInstance>();
    if (src != nullptr)
    {
        ret->CopyParticleSystem(src);
    }
    ret->Create();

    return ret;
}


ParticleSystemInstance::ParticleSystemInstance()
{

}

ParticleSystemInstance::~ParticleSystemInstance()
{

}

void ParticleSystemInstance::CopyParticleSystem(const ParticleSystem* src)
{
    mDuration = src->GetDuration();
    mSpawnRate = src->GetSpawnRate();
    mBurstCount = src->GetBurstCount();
    mBurstWindow = src->GetBurstWindow();
    mMaxParticles = src->GetMaxParticles();
    mLoops = src->GetLoops();
    mRadialVelocity = src->IsRadialVelocity();
    mRadialSpawn = src->IsRadialSpawn();
    mLockedRatio = src->IsRatioLocked();

    mMaterial = src->GetMaterial();

    SetParams(src->GetParams());
}

void ParticleSystemInstance::SaveStream(Stream& stream, Platform platform)
{
    // Do nothing
}

void ParticleSystemInstance::LoadStream(Stream& stream, Platform platform)
{
    // Do nothing
}

