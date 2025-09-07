#pragma once

#include "Maths.h"
#include <string>
#include <vector>
#include <map>

#include "Enums.h"
#include "Asset.h"
#include "AssetRef.h"
#include "Constants.h"
#include "EngineTypes.h"
#include "Vertex.h"
#include "Maths.h"

#include "Graphics/GraphicsConstants.h"
#include "Graphics/GraphicsTypes.h"

class Material;

struct ParticleParams
{
    // Spawn Randomized Params
    float mLifetimeMin = 1.0f;
    float mLifetimeMax = 2.0f;

    glm::vec3 mPositionMin = { -1.0f, -1.0f, -1.0f };
    glm::vec3 mPositionMax = { 1.0f, 1.0f, 1.0f };

    glm::vec3 mVelocityMin = { 0.0f, 1.0f, 0.0f };
    glm::vec3 mVelocityMax = { 0.0f, 3.0f, 0.0f };

    glm::vec2 mSizeMin = { 0.1f, 0.1f };
    glm::vec2 mSizeMax = { 0.2f, 0.2f };

    float mRotationMin = 0.0f;
    float mRotationMax = 2.0f * PI;

    float mRotationSpeedMin = 0.0f;
    float mRotationSpeedMax = 0.0f;

    // Constant Params
    glm::vec3 mAcceleration = { 0.0f, 0.0f, 0.0f };

    float mAlphaEase = 0.1f;
    float mScaleEase = 0.0f;

    // Over Lifetime Params
    glm::vec4 mColorStart = { 1.0f, 1.0f, 0.0f, 1.0f };
    glm::vec4 mColorEnd = { 1.0f, 0.0f, 0.0f, 0.0f };

    glm::vec2 mScaleStart = { 1.0f, 1.0f };
    glm::vec2 mScaleEnd = { 0.5f, 0.5f };
};

class ParticleSystem : public Asset
{
public:

    DECLARE_ASSET(ParticleSystem, Asset);

    ParticleSystem();
    ~ParticleSystem();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual bool Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    void SetMaterial(Material* material);
    Material* GetMaterial() const;

    const ParticleParams& GetParams() const;
    ParticleParams& GetParams();
    void SetParams(const ParticleParams& params);

    void SetDuration(float duration);
    float GetDuration() const;

    void SetSpawnRate(float spawnRate);
    float GetSpawnRate() const;

    void SetBurstCount(uint32_t burstCount);
    uint32_t GetBurstCount() const;

    void SetBurstWindow(float window);
    float GetBurstWindow() const;

    void SetMaxParticles(uint32_t maxParticles);
    uint32_t GetMaxParticles() const;

    void SetLoops(uint32_t loops);
    uint32_t GetLoops() const;

    void SetRadialVelocity(bool radial);
    bool IsRadialVelocity() const;

    void SetRadialSpawn(bool radial);
    bool IsRadialSpawn() const;

    void SetLockedRatio(bool lockedRatio);
    bool IsRatioLocked() const;

    Bounds GetBounds() const;

protected:

    float mDuration = 1.0f;
    float mSpawnRate = 10.0f;
    uint32_t mBurstCount = 0;
    float mBurstWindow = 0.1f;
    uint32_t mMaxParticles = 0;
    uint32_t mLoops = 0;
    bool mRadialVelocity = false;
    bool mRadialSpawn = false;
    bool mLockedRatio = true;

    MaterialRef mMaterial = nullptr;
    Bounds mBounds;

    ParticleParams mParams;

    // Make sure you copy over any new params in ParticleSystemInstance::CopyParticleSystem()
};
