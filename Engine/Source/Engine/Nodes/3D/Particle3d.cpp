#include "Nodes/3D/Particle3d.h"

#include "Renderer.h"
#include "AssetManager.h"
#include "Log.h"
#include "Utilities.h"
#include "Maths.h"
#include "Profiler.h"
#include "Assets/ParticleSystemInstance.h"

#include "Graphics/Graphics.h"

#if EDITOR
#include "EditorState.h"
#endif

FORCE_LINK_DEF(Particle3D);
DEFINE_NODE(Particle3D, Primitive3D);

const char* sParticleOrientationStrings[] =
{
    "X",
    "Y",
    "Z",
    "-X",
    "-Y",
    "-Z",
    "Billboard"
};
static_assert(int32_t(ParticleOrientation::Count) == 7, "Need to update string conversion table");

bool Particle3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Particle3D* particleComp = static_cast<Particle3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Emit")
    {
        particleComp->EnableEmission(*(bool*)newValue);
        success = true;
    }

    return success;
}

Particle3D::Particle3D()
{
    mName = "Particle";
}

Particle3D::~Particle3D()
{

}

const char* Particle3D::GetTypeName() const
{
    return "Particle";
}

void Particle3D::GatherProperties(std::vector<Property>& outProps)
{
    Primitive3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Particle");

    outProps.push_back(Property(DatumType::Asset, "Particle System", this, &mParticleSystem, 1, nullptr, int32_t(ParticleSystem::GetStaticType())));
    outProps.push_back(Property(DatumType::Asset, "Material Override", this, &mMaterialOverride, 1, nullptr, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Float, "Time Multiplier", this, &mTimeMultiplier));
    outProps.push_back(Property(DatumType::Bool, "Use Local Space", this, &mUseLocalSpace));
    outProps.push_back(Property(DatumType::Bool, "Emit", this, &mEmit, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Auto Emit", this, &mAutoEmit));
    outProps.push_back(Property(DatumType::Bool, "Always Simulate", this, &mAlwaysSimulate));
    outProps.push_back(Property(DatumType::Byte, "Orientation", this, &mOrientation, 1, nullptr, 0, int32_t(ParticleOrientation::Count), sParticleOrientationStrings));
}

void Particle3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED
    Primitive3D::GatherProxyDraws(inoutDraws);

    glm::vec4 color = glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);

    {
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        debugDraw.mTransform = glm::scale(mTransform, { 0.2f, 0.2f, 0.2f });
        inoutDraws.push_back(debugDraw);
    }

    // Show spawn box when selected
#if EDITOR
    if (GetEditorState()->GetSelectedNode() == this && mParticleSystem != nullptr)
    {
        ParticleSystem* sys = mParticleSystem.Get<ParticleSystem>();
        glm::vec3 minPos = sys->GetParams().mPositionMin;
        glm::vec3 maxPos = sys->GetParams().mPositionMax;

        glm::vec3 pos = { 0.0f, 0.0f, 0.0f };
        pos.x = (maxPos.x + minPos.x) / 2.0f;
        pos.y = (maxPos.y + minPos.y) / 2.0f;
        pos.z = (maxPos.z + minPos.z) / 2.0f;

        glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
        scale.x = (maxPos.x - minPos.x) / 2.0f;
        scale.y = (maxPos.y - minPos.y) / 2.0f;
        scale.z = (maxPos.z - minPos.z) / 2.0f;

        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Cube");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        glm::mat4 transform = glm::translate(mTransform, pos);
        transform = glm::scale(transform, scale);
        debugDraw.mTransform = transform;
        inoutDraws.push_back(debugDraw);
    }
#endif // EDITOR

#endif
}

void Particle3D::Create()
{
    Primitive3D::Create();
    GFX_CreateParticleCompResource(this);
    EnableEmission(true);
}

void Particle3D::Destroy()
{
    Primitive3D::Destroy();
    
    EnableEmission(false);

    GFX_DestroyParticleCompResource(this);

    mParticles.clear();
    mParticles.shrink_to_fit();
}

void Particle3D::Start()
{
    Primitive3D::Start();

    // Have auto emit determine starting emission.
    EnableEmission(mAutoEmit);
}

ParticleCompResource* Particle3D::GetResource()
{
    return &mResource;
}

DrawData Particle3D::GetDrawData()
{
    DrawData data = {};
    Material* material = GetMaterial();

    data.mNode = this;
    data.mMaterial = material;
    data.mBlendMode = material ? material->GetBlendMode() : BlendMode::Opaque;
    data.mPosition = GetWorldPosition();
    data.mBounds = GetBounds();
    data.mSortPriority = material ? material->GetSortPriority() : 0;
    data.mDepthless = material ? material->IsDepthTestDisabled() : false;

    return data;
}

void Particle3D::Render()
{
    GFX_DrawParticleComp(this);
}

void Particle3D::Tick(float deltaTime)
{
    Primitive3D::Tick(deltaTime);
    TickCommon(deltaTime);
}

void Particle3D::EditorTick(float deltaTime)
{
    Primitive3D::EditorTick(deltaTime);
    TickCommon(deltaTime);
}

void Particle3D::TickCommon(float deltaTime)
{
    mHasSimulatedThisFrame = false;
    mHasUpdatedVerticesThisFrame = false;

    if (mAutoDestroy)
    {
        mElapsedTime += deltaTime;

        if (mElapsedTime > 0.3f &&
            !IsEmissionEnabled() &&
            GetNumParticles() == 0)
        {
            SetPendingDestroy(true);
        }
    }
}

VertexType Particle3D::GetVertexType() const
{
    return VertexType::VertexParticle;
}

void Particle3D::Simulate(float deltaTime)
{
    if (mHasSimulatedThisFrame)
        return;

    float modDeltaTime = deltaTime * mTimeMultiplier;

    // This is the COMPONENT active var... not mEmit
    if (mActive && mEnableSimulation)
    {
        KillExpiredParticles(modDeltaTime);
        UpdateParticles(modDeltaTime);
        SpawnNewParticles(modDeltaTime);
    }

    mHasSimulatedThisFrame = true;
}

void Particle3D::Reset()
{
    mParticles.clear();
    mElapsedTime = 0.0f;
    mLoop = 0;
}

void Particle3D::EnableEmission(bool enable)
{
    mEmit = enable;

    if (!enable)
    {
        mElapsedTime = 0.0f;
        mLoop = 0;
    }
}

bool Particle3D::IsEmissionEnabled() const
{
    return mEmit;
}

void Particle3D::EnableAutoEmit(bool enable)
{
    mAutoEmit = enable;
}

bool Particle3D::IsAutoEmitEnabled() const
{
    return mAutoEmit;
}

void Particle3D::EnableAutoDestroy(bool enable)
{
    mAutoDestroy = enable;
}

bool Particle3D::IsAutoDestroyEnabled() const
{
    return mAutoDestroy;
}

float Particle3D::GetElapsedTime() const
{
    return mElapsedTime;
}

void Particle3D::SetParticleSystem(ParticleSystem* particleSystem)
{
    if (mParticleSystem.Get<ParticleSystem>() != particleSystem)
    {
        mParticleSystem = particleSystem;
    }
}

ParticleSystem* Particle3D::GetParticleSystem()
{
    return mParticleSystem.Get<ParticleSystem>();
}

void Particle3D::SetMaterialOverride(Material* material)
{
    mMaterialOverride = material;
}

Material* Particle3D::GetMaterial()
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

Material* Particle3D::GetMaterialOverride()
{
    return mMaterialOverride.Get<Material>();
}

void Particle3D::SetTimeMultiplier(float timeMultiplier)
{
    mTimeMultiplier = timeMultiplier;
}

float Particle3D::GetTimeMultiplier() const
{
    return mTimeMultiplier;
}

void Particle3D::SetUseLocalSpace(bool useLocalSpace)
{
    mUseLocalSpace = useLocalSpace;
}

bool Particle3D::GetUseLocalSpace() const
{
    return mUseLocalSpace;
}

void Particle3D::SetAlwaysSimulate(bool alwaysSimulate)
{
    mAlwaysSimulate = alwaysSimulate;
}

bool Particle3D::ShouldAlwaysSimulate() const
{
    return mAlwaysSimulate;
}

void Particle3D::EnableSimulation(bool simulate)
{
    mEnableSimulation = simulate;
}

bool Particle3D::IsSimulationEnabled() const
{
    return mEnableSimulation;
}

uint32_t Particle3D::GetNumParticles()
{
    return (uint32_t)mParticles.size();
}

uint32_t Particle3D::GetNumVertices()
{
    return (uint32_t)mVertices.size();
}

std::vector<Particle>& Particle3D::GetParticles()
{
    return mParticles;
}

const std::vector<VertexParticle>& Particle3D::GetVertices()
{
    return mVertices;
}

void Particle3D::SetParticleVelocity(int32_t index, glm::vec3 velocity)
{
    if (index == -1)
    {
        for (uint32_t i = 0; i < mParticles.size(); ++i)
        {
            mParticles[i].mVelocity = velocity;
        }
    }
    else if (index >= 0 && index < (int32_t)mParticles.size())
    {
        mParticles[index].mVelocity = velocity;
    }
}

glm::vec3 Particle3D::GetParticleVelocity(int32_t index)
{
    glm::vec3 ret = { 0.0f, 0.0f, 0.0f };
    if (index >= 0 && index < (int32_t)mParticles.size())
    {
        ret = mParticles[index].mVelocity;
    }
    return ret;
}


void Particle3D::SetParticlePosition(int32_t index, glm::vec3 position)
{
    if (index == -1)
    {
        for (uint32_t i = 0; i < mParticles.size(); ++i)
        {
            mParticles[i].mPosition = position;
        }
    }
    else if (index >= 0 && index < mParticles.size())
    {
        mParticles[index].mPosition = position;
    }
}

glm::vec3 Particle3D::GetParticlePosition(int32_t index)
{
    glm::vec3 ret = { 0.0f, 0.0f, 0.0f };
    if (index >= 0 && index < mParticles.size())
    {
        ret = mParticles[index].mPosition;
    }
    return ret;
}

void Particle3D::SetParticleSpeed(int32_t index, float speed)
{
    if (index == -1)
    {
        for (uint32_t i = 0; i < mParticles.size(); ++i)
        {
            mParticles[i].mVelocity = Maths::SafeNormalize(mParticles[i].mVelocity) * speed;
        }
    }
    else if (index >= 0 && index < mParticles.size())
    {
        mParticles[index].mVelocity = Maths::SafeNormalize(mParticles[index].mVelocity) * speed;
    }
}

void Particle3D::SetParticleOrientation(ParticleOrientation orientation)
{
    mOrientation = orientation;
}

ParticleOrientation Particle3D::GetParticleOrientation()
{
    return mOrientation;
}

ParticleSystemInstance* Particle3D::InstantiateParticleSystem()
{
    ParticleSystem* sys = GetParticleSystem();
    ParticleSystemInstance* sysInst = ParticleSystemInstance::New(sys);
    SetParticleSystem(sysInst);
    return sysInst;
}

Bounds Particle3D::GetLocalBounds() const
{
    if (mParticleSystem != nullptr)
    {
        return mParticleSystem.Get<ParticleSystem>()->GetBounds();
    }
    else
    {
        return Primitive3D::GetLocalBounds();
    }
}

void Particle3D::KillExpiredParticles(float deltaTime)
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

void Particle3D::UpdateParticles(float deltaTime)
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

void Particle3D::SpawnNewParticles(float deltaTime)
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
            if (system->IsRadialSpawn())
            {
                // Doing the powf(x,1/3) seems to be important for getting a uniform distribution in sphere.
                float distUnit = Maths::RandRange(params.mPositionMin.x, params.mPositionMax.x);
                distUnit = Maths::Map(distUnit, params.mPositionMin.x, params.mPositionMax.x, 0.0f, 1.0f);
                distUnit = powf(distUnit, 1 / 3.0f);
                distUnit = Maths::Map(distUnit, 0.0f, 1.0f, params.mPositionMin.x, params.mPositionMax.x);

                float yaw = Maths::RandRange(0.0f, PI * 2.0f);
                float pitch = Maths::RandRange(-PI/2.0f, PI/2.0f);
                glm::vec3 newPos = glm::vec3(0.0f, 0.0f, distUnit);
                newPos = glm::rotate(newPos, pitch, glm::vec3(1.0f, 0.0f, 0.0f));
                newPos = glm::rotate(newPos, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
                newParticle.mPosition = newPos;

                float dist = powf(distUnit, 1 / 3.0f);

            }
            else
            {
                newParticle.mPosition = Maths::RandRange(params.mPositionMin, params.mPositionMax);
            }
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

            if (system->IsRadialVelocity())
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

void Particle3D::UpdateVertexBuffer()
{
    ParticleSystem* system = mParticleSystem.Get<ParticleSystem>();

    if (system == nullptr || mHasUpdatedVerticesThisFrame)
        return;

    uint32_t numParticles = (uint32_t)mParticles.size();
    mVertices.resize(numParticles * 4);

    const ParticleParams& params = system->GetParams();

    const float alphaEase = params.mAlphaEase;
    const float scaleEase = params.mScaleEase;

    const float invAlphaEase2 = (alphaEase != 0.0f) ? (0.5f / alphaEase) : 1.0f;
    const float invScaleEase2 = (scaleEase != 0.0f) ? (0.5f / scaleEase) : 1.0f;

    glm::vec3 right = { 1.0f, 0.0f, 0.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 forward = { 0.0f, 0.0f, -1.0f };

    switch (mOrientation)
    {
    case ParticleOrientation::X:
        right = { 0.0f, 0.0f, -1.0f };
        up = { 0.0f, 1.0f, 0.0f };
        forward = { -1.0f, 0.0f, 0.0f };
        break;

    case ParticleOrientation::Y:
        right = { 1.0f, 0.0f, 0.0f };
        up = { 0.0f, 0.0f, -1.0f };
        forward = { 0.0f, -1.0f, 0.0f };
        break;

    case ParticleOrientation::Z:
        right = { 1.0f, 0.0f, 0.0f };
        up = { 0.0f, 1.0f, 0.0f };
        forward = { 0.0f, 0.0f, -1.0f };
        break;

    case ParticleOrientation::NX:
        right = { 0.0f, 0.0f, 1.0f };
        up = { 0.0f, 1.0f, 0.0f };
        forward = { 1.0f, 0.0f, 0.0f };
        break;

    case ParticleOrientation::NY:
        right = { -1.0f, 0.0f, 0.0f };
        up = { 0.0f, 0.0f, -1.0f };
        forward = { 0.0f, 1.0f, 0.0f };
        break;

    case ParticleOrientation::NZ:
        right = { -1.0f, 0.0f, 0.0f };
        up = { 0.0f, 1.0f, 0.0f };
        forward = { 0.0f, 0.0f, 1.0f };
        break;

    case ParticleOrientation::Billboard:
    {
        const glm::mat4& view = GetWorld()->GetActiveCamera()->GetViewMatrix();
        right = { view[0][0], view[1][0], view[2][0] };
        up = { view[0][1], view[1][1], view[2][1] };
        forward = { view[0][2], view[1][2], view[2][2] };
        break;
    }

    default:
        break;
    }

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

        glm::vec3 rightAxis = glm::rotate(right, mParticles[i].mRotation, forward);
        glm::vec3 upAxis = glm::rotate(up, mParticles[i].mRotation, forward);

        if (mUseLocalSpace && mOrientation == ParticleOrientation::Billboard)
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

    mHasUpdatedVerticesThisFrame = true;
}

