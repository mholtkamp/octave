#include "Components/ParticleComponent.h"

#include "Renderer.h"
#include "AssetManager.h"
#include "Log.h"
#include "Utilities.h"
#include "Maths.h"
#include "Profiler.h"

#include "Graphics/Graphics.h"

#include <glm/gtx/rotate_vector.hpp>

FORCE_LINK_DEF(ParticleComponent);
DEFINE_COMPONENT(ParticleComponent);

static bool HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    assert(prop != nullptr);
    ParticleComponent* particleComp = static_cast<ParticleComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Emit")
    {
        particleComp->EnableEmission(*(bool*)newValue);
        success = true;
    }

    return success;
}

ParticleComponent::ParticleComponent()
{
    mName = "Particle";
}

ParticleComponent::~ParticleComponent()
{

}

const char* ParticleComponent::GetTypeName() const
{
    return "Particle";
}

void ParticleComponent::GatherProperties(std::vector<Property>& outProps)
{
    PrimitiveComponent::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Asset, "Particle System", this, &mParticleSystem, 1, nullptr, int32_t(ParticleSystem::GetStaticType())));
    outProps.push_back(Property(DatumType::Asset, "Material Override", this, &mMaterialOverride, 1, nullptr, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Float, "Time Multiplier", this, &mTimeMultiplier));
    outProps.push_back(Property(DatumType::Bool, "Use Local Space", this, &mUseLocalSpace));
    outProps.push_back(Property(DatumType::Bool, "Emit", this, &mEmit, 1, HandlePropChange));
}

void ParticleComponent::Create()
{
    PrimitiveComponent::Create();
    GFX_CreateParticleCompResource(this);
    EnableEmission(true);
}

void ParticleComponent::Destroy()
{
    PrimitiveComponent::Destroy();
    
    EnableEmission(false);

    GFX_DestroyParticleCompResource(this);

    mParticles.clear();
    mParticles.shrink_to_fit();
}

ParticleCompResource* ParticleComponent::GetResource()
{
    return &mResource;
}

void ParticleComponent::SaveStream(Stream& stream)
{
    PrimitiveComponent::SaveStream(stream);

    stream.WriteAsset(mParticleSystem);
    stream.WriteAsset(mMaterialOverride);
    stream.WriteFloat(mTimeMultiplier);
    stream.WriteBool(mUseLocalSpace);
    stream.WriteBool(mEmit);
}

void ParticleComponent::LoadStream(Stream& stream)
{
    PrimitiveComponent::LoadStream(stream);

    stream.ReadAsset(mParticleSystem);
    stream.ReadAsset(mMaterialOverride);
    mTimeMultiplier = stream.ReadFloat();
    mUseLocalSpace = stream.ReadBool();
    mEmit = stream.ReadBool();
}

DrawData ParticleComponent::GetDrawData()
{
    DrawData data = {};
    Material* material = GetMaterial();

    data.mComponent = static_cast<PrimitiveComponent*>(this);
    data.mMaterial = material;
    data.mShadingModel = material ? material->GetShadingModel() : ShadingModel::Lit;
    data.mBlendMode = material ? material->GetBlendMode() : BlendMode::Opaque;
    data.mPosition = GetAbsolutePosition();
    data.mBounds = GetBounds();
    data.mSortPriority = material ? material->GetSortPriority() : 0;
    data.mDepthless = material ? material->IsDepthTestDisabled() : false;

    return data;
}

void ParticleComponent::Render()
{
    GFX_DrawParticleComp(this);
}

void ParticleComponent::Tick(float deltaTime)
{
    PrimitiveComponent::Tick(deltaTime);

    float modDeltaTime = deltaTime * mTimeMultiplier;

    // This is the COMPONENT active var... not mEmit
    if (mActive)
    {
        KillExpiredParticles(modDeltaTime);
        UpdateParticles(modDeltaTime);
        SpawnNewParticles(modDeltaTime);
        UpdateVertexBuffer();
    }
}

VertexType ParticleComponent::GetVertexType() const
{
    return VertexType::VertexParticle;
}

void ParticleComponent::Reset()
{
    mParticles.clear();
    mElapsedTime = 0.0f;
    mLoop = 0;
}

void ParticleComponent::EnableEmission(bool enable)
{
    mEmit = enable;

    if (!enable)
    {
        mElapsedTime = 0.0f;
        mLoop = 0;
    }
}

bool ParticleComponent::IsEmissionEnabled() const
{
    return mEmit;
}

float ParticleComponent::GetElapsedTime() const
{
    return mElapsedTime;
}

void ParticleComponent::SetParticleSystem(ParticleSystem* particleSystem)
{
    if (mParticleSystem.Get<ParticleSystem>() != particleSystem)
    {
        mParticleSystem = particleSystem;
    }
}

ParticleSystem* ParticleComponent::GetParticleSystem()
{
    return mParticleSystem.Get<ParticleSystem>();
}

void ParticleComponent::SetMaterialOverride(Material* material)
{
    mMaterialOverride = material;
}

Material* ParticleComponent::GetMaterial()
{
    Material* retMaterial = nullptr;

    if (mMaterialOverride.Get<Material>() != nullptr)
    {
        retMaterial = mMaterialOverride.Get<Material>();
    }
    else if (mParticleSystem.Get<ParticleSystem>() != nullptr)
    {
        retMaterial = mParticleSystem.Get<ParticleSystem>()->GetMaterial();
    }

    return retMaterial;
}

Material* ParticleComponent::GetMaterialOverride()
{
    return mMaterialOverride.Get<Material>();
}

void ParticleComponent::SetTimeMultiplier(float timeMultiplier)
{
    mTimeMultiplier = timeMultiplier;
}

float ParticleComponent::GetTimeMultiplier() const
{
    return mTimeMultiplier;
}

void ParticleComponent::SetUseLocalSpace(bool useLocalSpace)
{
    mUseLocalSpace = useLocalSpace;
}

bool ParticleComponent::GetUseLocalSpace() const
{
    return mUseLocalSpace;
}

uint32_t ParticleComponent::GetNumParticles()
{
    return (uint32_t)mParticles.size();
}

const std::vector<Particle>& ParticleComponent::GetParticles()
{
    return mParticles;
}

const std::vector<VertexParticle>& ParticleComponent::GetVertices()
{
    return mVertices;
}

Bounds ParticleComponent::GetLocalBounds() const
{
    if (mParticleSystem != nullptr)
    {
        return mParticleSystem.Get<ParticleSystem>()->GetBounds();
    }
    else
    {
        return PrimitiveComponent::GetLocalBounds();
    }
}

void ParticleComponent::KillExpiredParticles(float deltaTime)
{
    for (int32_t i = int32_t(mParticles.size()) - 1; i >= 0; --i)
    {
        Particle& particle = mParticles[i];

        if (particle.mElapsedTime >= particle.mLifetime)
        {
            // Might be more efficient if we swap instead of erase?
            mParticles.erase(mParticles.begin() + i);
        }
    }
}

void ParticleComponent::UpdateParticles(float deltaTime)
{
    ParticleSystem* system = mParticleSystem.Get<ParticleSystem>();

    if (system != nullptr)
    {
        ParticleParams& params = system->GetParams();

        for (uint32_t i = 0; i < mParticles.size(); ++i)
        {
            mParticles[i].mElapsedTime += deltaTime;
            mParticles[i].mVelocity += (params.mAcceleration * deltaTime);
            mParticles[i].mPosition += (mParticles[i].mVelocity * deltaTime);
            mParticles[i].mRotation += (mParticles[i].mRotationSpeed * deltaTime);
        }
    }
}

void ParticleComponent::SpawnNewParticles(float deltaTime)
{
    ParticleSystem* system = mParticleSystem.Get<ParticleSystem>();

    if (mEmit && system)
    {
        const ParticleParams& params = system->GetParams();

        int32_t burstCount = 0;

        if (system->GetBurstCount() > 0 &&
            mElapsedTime < system->GetBurstWindow())
        {
            if (system->GetBurstWindow() > 0.0f)
            {
                burstCount = int32_t(system->GetBurstCount() * (deltaTime / system->GetBurstWindow()));
                burstCount = glm::min(burstCount, (int32_t)system->GetBurstCount());
            }
            else
            {
                burstCount = system->GetBurstCount();
            }
        }

        mElapsedTime += deltaTime;

        if (system->GetDuration() > 0.0f &&
            mElapsedTime >= system->GetDuration())
        {
            ++mLoop;
            mElapsedTime = 0.0f;

            if (system->GetLoops() > 0 &&
                mLoop >= system->GetLoops())
            {
                EnableEmission(false);
            }
        }

        mEmissionCounter += (deltaTime * system->GetSpawnRate());

        int32_t spawnCount = int32_t(mEmissionCounter);
        mEmissionCounter -= float(spawnCount);

        spawnCount += burstCount;

        int32_t maxParticles = (int32_t)system->GetMaxParticles();

        if (maxParticles > 0)
        {
            int32_t numParticles = (int32_t)mParticles.size();
            spawnCount = glm::min(maxParticles - numParticles, spawnCount);
        }

        for (int32_t i = 0; i < spawnCount; ++i)
        {
            Particle newParticle;

            newParticle.mLifetime = Maths::RandRange(params.mLifetimeMin, params.mLifetimeMax);
            newParticle.mPosition = Maths::RandRange(params.mPositionMin, params.mPositionMax);
            newParticle.mVelocity = Maths::RandRange(params.mVelocityMin, params.mVelocityMax);
            newParticle.mSize = Maths::RandRange(params.mSizeMin, params.mSizeMax);
            newParticle.mRotation = Maths::RandRange(params.mRotationMin, params.mRotationMax);
            newParticle.mRotationSpeed = Maths::RandRange(params.mRotationSpeedMin, params.mRotationSpeedMax);

            if (system->IsRatioLocked())
            {
                float ratioYX = params.mSizeMax.x != 0.0f ? (params.mSizeMax.y / params.mSizeMax.x) : 1.0f;
                newParticle.mSize.x = Maths::RandRange(params.mSizeMin.x, params.mSizeMax.x);
                newParticle.mSize.y = ratioYX * newParticle.mSize.x;
            }

            if (system->IsRadial())
            {
                // Velocity will be oriented away from origin.
                float speed = newParticle.mVelocity.x;
                glm::vec3 direction = glm::normalize(newParticle.mPosition);
                newParticle.mVelocity = direction * speed;
            }

            if (!mUseLocalSpace)
            {
                // Make sure to do this step AFTER RADIAL VELOCITY
                newParticle.mPosition = mTransform * glm::vec4(newParticle.mPosition, 1.0f);
                newParticle.mVelocity = mTransform * glm::vec4(newParticle.mVelocity, 0.0f);
            }

            mParticles.push_back(newParticle);
        }
    }
}

void ParticleComponent::UpdateVertexBuffer()
{
    ParticleSystem* system = mParticleSystem.Get<ParticleSystem>();

    if (system == nullptr)
        return;

    uint32_t numParticles = (uint32_t)mParticles.size();
    mVertices.resize(numParticles * 4);

    const ParticleParams& params = system->GetParams();

    const float alphaEase = params.mAlphaEase;
    const float scaleEase = params.mScaleEase;

    const float invAlphaEase2 = (alphaEase != 0.0f) ? (0.5f / alphaEase) : 1.0f;
    const float invScaleEase2 = (scaleEase != 0.0f) ? (0.5f / scaleEase) : 1.0f;

    const glm::mat4& view = GetWorld()->GetActiveCamera()->GetViewMatrix();
    const glm::vec3 cameraRight = { view[0][0], view[1][0], view[2][0] };
    const glm::vec3 cameraUp = { view[0][1], view[1][1], view[2][1] };
    const glm::vec3 cameraForward = { view[0][2], view[1][2], view[2][2] };

    for (uint32_t i = 0; i < numParticles; ++i)
    {
        VertexParticle* verts = &mVertices[i * 4];

        float life = mParticles[i].mElapsedTime / mParticles[i].mLifetime;

        glm::vec2 scale = glm::mix(params.mScaleStart, params.mScaleEnd, life);
        glm::vec4 color = glm::mix(params.mColorStart, params.mColorEnd, life);

        float easeX = 2 * fabs(life - 0.5f);

        if (scaleEase > 0.0f)
        {
            float scalePower = glm::clamp(invScaleEase2 * (1.0f - easeX), 0.0f, 1.0f);
            scale = glm::mix(glm::vec2(0.0f, 0.0f), scale, scalePower);
        }

        if (alphaEase > 0.0f)
        {
            float alphaPower = glm::clamp(invAlphaEase2 * (1.0f - easeX), 0.0f, 1.0f);
            color.a = glm::mix(0.0f, color.a, alphaPower);
        }

        glm::vec3 pos = mParticles[i].mPosition;
        glm::vec2 halfSize = mParticles[i].mSize * scale * 0.5f;
        uint8_t colors[4] = 
        {
            uint8_t(glm::clamp(color.r * 255.0f, 0.0f, 255.0f)),
            uint8_t(glm::clamp(color.g * 255.0f, 0.0f, 255.0f)),
            uint8_t(glm::clamp(color.b * 255.0f, 0.0f, 255.0f)),
            uint8_t(glm::clamp(color.a * 255.0f, 0.0f, 255.0f))
        };
        uint32_t color32 = 
            (colors[0]) |
            (colors[1] << 8) |
            (colors[2] << 16) |
            (colors[3] << 24);

        glm::vec3 rightAxis = glm::rotate(cameraRight, mParticles[i].mRotation, cameraForward);
        glm::vec3 upAxis = glm::rotate(cameraUp, mParticles[i].mRotation, cameraForward);

        if (mUseLocalSpace)
        {
            rightAxis = glm::vec4(rightAxis, 0.0f) * mTransform;
            upAxis = glm::vec4(upAxis, 0.0f) * mTransform;
        }

        //   0----2
        //   |  / |
        //   | /  |
        //   1----3
        verts[0].mPosition = pos - (rightAxis * halfSize.x) + (upAxis * halfSize.y);
        verts[0].mTexcoord = glm::vec2(0.0f, 0.0f);
        verts[0].mColor = color32;

        verts[1].mPosition = pos - (rightAxis * halfSize.x) - (upAxis * halfSize.y);
        verts[1].mTexcoord = glm::vec2(0.0f, 1.0f);
        verts[1].mColor = color32;

        verts[2].mPosition = pos + (rightAxis * halfSize.x) + (upAxis * halfSize.y);
        verts[2].mTexcoord = glm::vec2(1.0f, 0.0f);
        verts[2].mColor = color32;

        verts[3].mPosition = pos + (rightAxis * halfSize.x) - (upAxis * halfSize.y);
        verts[3].mTexcoord = glm::vec2(1.0f, 1.0f);
        verts[3].mColor = color32;
    }

    GFX_UpdateParticleCompVertexBuffer(this, mVertices);
}

