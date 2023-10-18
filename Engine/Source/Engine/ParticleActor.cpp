#include "ParticleActor.h"
#include "Log.h"
#include "World.h"

DEFINE_ACTOR(ParticleActor, Actor);

void ParticleActor::Create()
{
    Actor::Create();

    mParticleComp = CreateComponent<Particle3D>();
    SetRootComponent(mParticleComp);
    mParticleComp->EnableEmission(true);
}

void ParticleActor::Tick(float deltaTime)
{
    Actor::Tick(deltaTime);

    mTimeAlive += deltaTime;

    if (mTimeAlive > 0.3f &&
        !mParticleComp->IsEmissionEnabled() &&
        mParticleComp->GetNumParticles() == 0)
    {
        SetPendingDestroy(true);
    }
}

void ParticleActor::SetParticleSystem(ParticleSystem* system)
{
    mParticleComp->SetParticleSystem(system);
}

ParticleActor* ParticleActor::SpawnParticleActor(World* world, glm::vec3 position, ParticleSystem* system)
{
    // This might be replaced with a pooling system later. For now it's a simple world->SpawnActor().
    ParticleActor* particleActor = world->SpawnActor<ParticleActor>();
    particleActor->SetPosition(position);
    particleActor->SetParticleSystem(system);
    particleActor->UpdateComponentTransforms();
    return particleActor;
}
