#pragma once

#include "Components/PrimitiveComponent.h"
#include "Assets/ParticleSystem.h"

struct Particle
{
    glm::vec3 mPosition = {};
    float mElapsedTime = 0.0f;

    glm::vec3 mVelocity = {};
    float mLifetime = 1.0f;

    glm::vec2 mSize = {};
    float mRotationSpeed = 0.0f;
    float mRotation = 0.0f;
};

class ParticleComponent : public PrimitiveComponent
{
public:

    DECLARE_COMPONENT(ParticleComponent, PrimitiveComponent);

    ParticleComponent();
    virtual ~ParticleComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    ParticleCompResource* GetResource();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual DrawData GetDrawData() override;
    virtual void Render() override;
    virtual void Tick(float deltaTime) override;

    virtual VertexType GetVertexType() const override;

    void Reset();
    void EnableEmission(bool enable);
    bool IsEmissionEnabled() const;

    float GetElapsedTime() const;

    void SetParticleSystem(ParticleSystem* particleSystem);
    ParticleSystem* GetParticleSystem();

    void SetMaterialOverride(Material* material);
    Material* GetMaterial();
    Material* GetMaterialOverride();

    void SetTimeMultiplier(float timeMultiplier);
    float GetTimeMultiplier() const;

    void SetUseLocalSpace(bool useLocalSpace);
    bool GetUseLocalSpace() const;

    uint32_t GetNumParticles();
    const std::vector<Particle>& GetParticles();
    const std::vector<VertexParticle>& GetVertices();

    virtual Bounds GetLocalBounds() const override;

protected:

    void KillExpiredParticles(float deltaTime);
    void UpdateParticles(float deltaTime);
    void SpawnNewParticles(float deltaTime);
    void UpdateVertexBuffer();

    float mElapsedTime = 0.0f;
    bool mEmit = true;
    std::vector<Particle> mParticles;
    std::vector<VertexParticle> mVertices;
    float mEmissionCounter = 0.0f;
    uint32_t mLoop = 0;

    // Properties
    ParticleSystemRef mParticleSystem;
    MaterialRef mMaterialOverride;
    float mTimeMultiplier = 1.0f;
    bool mUseLocalSpace = false;

    // Graphics Resource
    ParticleCompResource mResource;
};
