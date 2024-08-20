#pragma once

#include "Assets/ParticleSystem.h"

class ParticleSystemInstance : public ParticleSystem
{
public:

    DECLARE_ASSET(ParticleSystemInstance, ParticleSystem);

    static ParticleSystemInstance* New(const ParticleSystem* src = nullptr);

    ParticleSystemInstance();
    ~ParticleSystemInstance();

    void CopyParticleSystem(const ParticleSystem* src);

    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void LoadStream(Stream& stream, Platform platform) override;
};

