
#include "Assets/ParticleSystemInstance.h"

#include "AssetManager.h"
#include "Asset.h"
#include "Assets/Material.h"

DEFINE_ASSET(ParticleSystemInstance);

ParticleSystemInstance* ParticleSystemInstance::New(const ParticleSystem* src)
{
    ParticleSystemInstance* ret = new ParticleSystemInstance();
    if (src != nullptr)
    {
        ret->CopyParticleSystem(src);
    }
    ret->Create();

    AssetManager::Get()->RegisterTransientAsset(ret);

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

bool ParticleSystemInstance::IsTransient() const
{
    return true;
}

void ParticleSystemInstance::SaveStream(Stream& stream)
{
    // Do nothing
}

void ParticleSystemInstance::LoadStream(Stream& stream)
{
    // Do nothing
}

