#include "Assets/ParticleSystem.h"

#include "Engine.h"
#include "Log.h"
#include "Property.h"
#include "Stream.h"
#include "Assets/Material.h"

FORCE_LINK_DEF(ParticleSystem);
DEFINE_ASSET(ParticleSystem);

ParticleSystem::ParticleSystem()
{
    mType = ParticleSystem::GetStaticType();

    mBounds.mCenter = { 0.0f, 0.0f, 0.0f };
    mBounds.mRadius = LARGE_BOUNDS;
}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mDuration = stream.ReadFloat();
    mSpawnRate = stream.ReadFloat();
    mBurstCount = stream.ReadUint32();
    mBurstWindow = stream.ReadFloat();
    mMaxParticles = stream.ReadUint32();
    mLoops = stream.ReadUint32();
    mRadialVelocity = stream.ReadBool();
    if (mVersion >= ASSET_VERSION_PARTICLE_RADIAL_SPAWN)
    {
        mRadialSpawn = stream.ReadBool();
    }
    mLockedRatio = stream.ReadBool();

    stream.ReadAsset(mMaterial);

    mBounds.mCenter = stream.ReadVec3();
    mBounds.mRadius = stream.ReadFloat();

    // Particle Params
    mParams.mLifetimeMin = stream.ReadFloat();
    mParams.mLifetimeMax = stream.ReadFloat();

    mParams.mPositionMin = stream.ReadVec3();
    mParams.mPositionMax = stream.ReadVec3();

    mParams.mVelocityMin = stream.ReadVec3();
    mParams.mVelocityMax = stream.ReadVec3();

    mParams.mSizeMin = stream.ReadVec2();
    mParams.mSizeMax = stream.ReadVec2();

    mParams.mRotationMin = stream.ReadFloat();
    mParams.mRotationMax = stream.ReadFloat();

    mParams.mRotationSpeedMin = stream.ReadFloat();
    mParams.mRotationSpeedMax = stream.ReadFloat();

    mParams.mAcceleration = stream.ReadVec3();

    mParams.mAlphaEase = stream.ReadFloat();
    mParams.mScaleEase = stream.ReadFloat();

    mParams.mColorStart = stream.ReadVec4();
    mParams.mColorEnd = stream.ReadVec4();

    mParams.mScaleStart = stream.ReadVec2();
    mParams.mScaleEnd = stream.ReadVec2();
}

void ParticleSystem::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteFloat(mDuration);
    stream.WriteFloat(mSpawnRate);
    stream.WriteUint32(mBurstCount);
    stream.WriteFloat(mBurstWindow);
    stream.WriteUint32(mMaxParticles);
    stream.WriteUint32(mLoops);
    stream.WriteBool(mRadialVelocity);
    stream.WriteBool(mRadialSpawn);
    stream.WriteBool(mLockedRatio);

    stream.WriteAsset(mMaterial);

    stream.WriteVec3(mBounds.mCenter);
    stream.WriteFloat(mBounds.mRadius);

    // Particle Params
    stream.WriteFloat(mParams.mLifetimeMin);
    stream.WriteFloat(mParams.mLifetimeMax);

    stream.WriteVec3(mParams.mPositionMin);
    stream.WriteVec3(mParams.mPositionMax);

    stream.WriteVec3(mParams.mVelocityMin);
    stream.WriteVec3(mParams.mVelocityMax);

    stream.WriteVec2(mParams.mSizeMin);
    stream.WriteVec2(mParams.mSizeMax);

    stream.WriteFloat(mParams.mRotationMin);
    stream.WriteFloat(mParams.mRotationMax);

    stream.WriteFloat(mParams.mRotationSpeedMin);
    stream.WriteFloat(mParams.mRotationSpeedMax);

    stream.WriteVec3(mParams.mAcceleration);

    stream.WriteFloat(mParams.mAlphaEase);
    stream.WriteFloat(mParams.mScaleEase);

    stream.WriteVec4(mParams.mColorStart);
    stream.WriteVec4(mParams.mColorEnd);

    stream.WriteVec2(mParams.mScaleStart);
    stream.WriteVec2(mParams.mScaleEnd);
}

void ParticleSystem::Create()
{
    Asset::Create();
}

void ParticleSystem::Destroy()
{
    Asset::Destroy();
}

void ParticleSystem::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);
}

void ParticleSystem::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Float, "Duration", this, &mDuration));
    outProps.push_back(Property(DatumType::Float, "Spawn Rate", this, &mSpawnRate));
    outProps.push_back(Property(DatumType::Integer, "Burst Count", this, &mBurstCount));
    outProps.push_back(Property(DatumType::Float, "Burst Window", this, &mBurstWindow));
    outProps.push_back(Property(DatumType::Integer, "Max Particles", this, &mMaxParticles));
    outProps.push_back(Property(DatumType::Integer, "Loops", this, &mLoops));
    outProps.push_back(Property(DatumType::Bool, "Radial Velocity", this, &mRadialVelocity));
    outProps.push_back(Property(DatumType::Bool, "Radial Spawn", this, &mRadialSpawn));
    outProps.push_back(Property(DatumType::Bool, "Locked Ratio", this, &mLockedRatio));

    outProps.push_back(Property(DatumType::Vector, "Bounds Center", this, &mBounds.mCenter));
    outProps.push_back(Property(DatumType::Float, "Bounds Radius", this, &mBounds.mRadius));

    outProps.push_back(Property(DatumType::Asset, "Material", this, &mMaterial, 1, nullptr, int32_t(Material::GetStaticType())));

    outProps.push_back(Property(DatumType::Float, "Lifetime Min", this, &mParams.mLifetimeMin));
    outProps.push_back(Property(DatumType::Float, "Lifetime Max", this, &mParams.mLifetimeMax));

    outProps.push_back(Property(DatumType::Vector, "Position Min", this, &mParams.mPositionMin));
    outProps.push_back(Property(DatumType::Vector, "Position Max", this, &mParams.mPositionMax));

    outProps.push_back(Property(DatumType::Vector, "Velocity Min", this, &mParams.mVelocityMin));
    outProps.push_back(Property(DatumType::Vector, "Velocity Max", this, &mParams.mVelocityMax));

    outProps.push_back(Property(DatumType::Vector2D, "Size Min", this, &mParams.mSizeMin));
    outProps.push_back(Property(DatumType::Vector2D, "Size Max", this, &mParams.mSizeMax));

    outProps.push_back(Property(DatumType::Float, "Rotation Min", this, &mParams.mRotationMin));
    outProps.push_back(Property(DatumType::Float, "Rotation Max", this, &mParams.mRotationMax));

    outProps.push_back(Property(DatumType::Float, "Rotation Speed Min", this, &mParams.mRotationSpeedMin));
    outProps.push_back(Property(DatumType::Float, "Rotation Speed Max", this, &mParams.mRotationSpeedMax));

    outProps.push_back(Property(DatumType::Vector, "Acceleration", this, &mParams.mAcceleration));

    outProps.push_back(Property(DatumType::Float, "Alpha Ease", this, &mParams.mAlphaEase));
    outProps.push_back(Property(DatumType::Float, "Scale Ease", this, &mParams.mScaleEase));

    outProps.push_back(Property(DatumType::Color, "Color Start", this, &mParams.mColorStart));
    outProps.push_back(Property(DatumType::Color, "Color End", this, &mParams.mColorEnd));

    outProps.push_back(Property(DatumType::Vector2D, "Scale Start", this, &mParams.mScaleStart));
    outProps.push_back(Property(DatumType::Vector2D, "Scale End", this, &mParams.mScaleEnd));
}

glm::vec4 ParticleSystem::GetTypeColor()
{
    return glm::vec4(0.2f, 0.7f, 1.0f, 1.0f);
}

const char* ParticleSystem::GetTypeName()
{
    return "ParticleSystem";
}

void ParticleSystem::SetMaterial(Material* material)
{
    mMaterial = material;
}

Material* ParticleSystem::GetMaterial() const
{
    return mMaterial.Get<Material>();
}

const ParticleParams& ParticleSystem::GetParams() const
{
    return mParams;
}

ParticleParams& ParticleSystem::GetParams()
{
    return mParams;
}

void ParticleSystem::SetParams(const ParticleParams& params)
{
    mParams = params;
}

void ParticleSystem::SetDuration(float duration)
{
    mDuration = duration;
}

float ParticleSystem::GetDuration() const
{
    return mDuration;
}

void ParticleSystem::SetSpawnRate(float spawnRate)
{
    mSpawnRate = spawnRate;
}

float ParticleSystem::GetSpawnRate() const
{
    return mSpawnRate;
}

void ParticleSystem::SetBurstCount(uint32_t burstCount)
{
    mBurstCount = burstCount;
}

uint32_t ParticleSystem::GetBurstCount() const
{
    return mBurstCount;
}

void ParticleSystem::SetBurstWindow(float window)
{
    mBurstWindow = window;
}

float ParticleSystem::GetBurstWindow() const
{
    return mBurstWindow;
}

void ParticleSystem::SetMaxParticles(uint32_t maxParticles)
{
    mMaxParticles = maxParticles;
}

uint32_t ParticleSystem::GetMaxParticles() const
{
    return mMaxParticles;
}

void ParticleSystem::SetLoops(uint32_t loops)
{
    mLoops = loops;
}

uint32_t ParticleSystem::GetLoops() const
{
    return mLoops;
}

void ParticleSystem::SetRadialVelocity(bool radial)
{
    mRadialVelocity = radial;
}

bool ParticleSystem::IsRadialVelocity() const
{
    return mRadialVelocity;
}

void ParticleSystem::SetRadialSpawn(bool radial)
{
    mRadialSpawn = radial;
}

bool ParticleSystem::IsRadialSpawn() const
{
    return mRadialSpawn;
}

void ParticleSystem::SetLockedRatio(bool lockedRatio)
{
    mLockedRatio = lockedRatio;
}

bool ParticleSystem::IsRatioLocked() const
{
    return mLockedRatio;
}

Bounds ParticleSystem::GetBounds() const
{
    return mBounds;
}
