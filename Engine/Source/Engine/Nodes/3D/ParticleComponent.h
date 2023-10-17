#pragma once

#include "Components/PrimitiveComponent.h"
#include "Assets/ParticleSystem.h"
#include "Assets/ParticleSystemInstance.h"

enum class ParticleOrientation : uint8_t
{
    X,
    Y,
    Z,
    NX,
    NY,
    NZ,
    Billboard,

    Count
};

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
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void BeginPlay() override;
    ParticleCompResource* GetResource();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual DrawData GetDrawData() override;
    virtual void Render() override;
    virtual void Tick(float deltaTime) override;

    virtual VertexType GetVertexType() const override;

    void Simulate(float deltaTime);
    void UpdateVertexBuffer();

    void Reset();
    void EnableEmission(bool enable);
    bool IsEmissionEnabled() const;

    void EnableAutoEmit(bool enable);
    bool IsAutoEmitEnabled() const;

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

    void SetAlwaysSimulate(bool alwaysSimulate);
    bool ShouldAlwaysSimulate() const;

    void EnableSimulation(bool simulate);
    bool IsSimulationEnabled() const;

    uint32_t GetNumParticles();
    uint32_t GetNumVertices();
    const std::vector<Particle>& GetParticles();
    const std::vector<VertexParticle>& GetVertices();

    void SetParticleVelocity(int32_t index, glm::vec3 velocity);
    glm::vec3 GetParticleVelocity(int32_t index);

    void SetParticlePosition(int32_t index, glm::vec3 position);
    glm::vec3 GetParticlePosition(int32_t index);

    void SetParticleSpeed(int32_t index, float speed);

    void SetParticleOrientation(ParticleOrientation orientation);
    ParticleOrientation GetParticleOrientation();

    ParticleSystemInstance* InstantiateParticleSystem();

    virtual Bounds GetLocalBounds() const override;

protected:

    void KillExpiredParticles(float deltaTime);
    void UpdateParticles(float deltaTime);
    void SpawnNewParticles(float deltaTime);

    float mElapsedTime = 0.0f;
    bool mEmit = true;
    bool mAutoEmit = true;
    std::vector<Particle> mParticles;
    std::vector<VertexParticle> mVertices;
    float mEmissionCounter = 0.0f;
    uint32_t mLoop = 0;
    bool mHasSimulatedThisFrame = false;
    bool mHasUpdatedVerticesThisFrame = false;

    // Properties
    ParticleSystemRef mParticleSystem;
    MaterialRef mMaterialOverride;
    float mTimeMultiplier = 1.0f;
    bool mUseLocalSpace = false;
    bool mAlwaysSimulate = true;
    bool mEnableSimulation = true;
    ParticleOrientation mOrientation = ParticleOrientation::Billboard;

    // Graphics Resource
    ParticleCompResource mResource;
};
