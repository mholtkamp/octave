#include "AudioManager.h"
#include "Assets/SoundWave.h"
#include "Asset.h"
#include "Log.h"
#include "Maths.h"
#include "Engine.h"
#include "World.h"
#include "Profiler.h"

#include "Components/TransformComponent.h"
#include "Components/AudioComponent.h"

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"

// TODO: define max audio sources as AUDIO_MAX_VOICES
#define MAX_AUDIO_SOURCES AUDIO_MAX_VOICES

struct AudioSource
{
    SoundWaveRef mSoundWave;
    AudioComponent* mComponent;
    float mVolumeMult;
    float mPitchMult;
    int32_t mPriority;
    glm::vec3 mPosition;
    float mInnerRadius;
    float mOuterRadius;
    AttenuationFunc mAttenuationFunc;

    AudioSource()
    {
        Reset();
    }

    void Set(
        SoundWave* soundWave,
        AudioComponent* component,
        float volumeMult,
        float pitchMult,
        int32_t priority,
        glm::vec3 position,
        float innerRadius,
        float outerRadius,
        AttenuationFunc attenFunc)
    {
        mSoundWave = soundWave;
        mComponent = component;
        mVolumeMult = volumeMult;
        mPitchMult = pitchMult;
        mPriority = priority;
        mPosition = position;
        mInnerRadius = innerRadius;
        mOuterRadius = outerRadius;
        mAttenuationFunc = attenFunc;
    }

    void Reset()
    {
        mSoundWave = nullptr;
        mComponent = nullptr;
        mVolumeMult = 1.0f;
        mPitchMult = 1.0f;
        mPriority = 0;
        mPosition = { 0.0f, 0.0f, 0.0f };
        mInnerRadius = -1.0f;
        mOuterRadius = -1.0f;
        mAttenuationFunc = AttenuationFunc::Count;
    }
};

static AudioSource sAudioSources[MAX_AUDIO_SOURCES];

float CalcVolumeAttenuation(AttenuationFunc func, float innerRadius, float outerRadius, float distance)
{
    float ret = 1.0f;

    float x = glm::max(0.0f, distance - innerRadius) / (outerRadius - innerRadius);
    x = glm::clamp(x, 0.0f, 1.0f);
    x = (1.0f - x);

    switch (func)
    {
    case AttenuationFunc::Linear:
        ret = x;
        break;

    default:
        break;
    }

    return ret;
}

void CalcVolumeAttenuationLR(
    AttenuationFunc func,
    float innerRadius,
    float outerRadius,
    glm::vec3 srcPos,
    glm::vec3 listenerPos,
    glm::vec3 listenerRight,
    float distance,
    float& outLeft,
    float& outRight)
{
    float volume = 1.0f;

    float x = glm::max(0.0f, distance - innerRadius) / (outerRadius - innerRadius);
    x = glm::clamp(x, 0.0f, 1.0f);
    x = (1.0f - x);

    switch (func)
    {
    case AttenuationFunc::Linear:
        volume = x;
        break;

    default:
        break;
    }

    glm::vec3 toSrc = Maths::SafeNormalize(srcPos - listenerPos);
    float dot = glm::dot(listenerRight, toSrc);

    float minAttenAlpha = glm::clamp((distance - 1.0f) / 5.0f, 0.0f, 1.0f);
    float MinDirAtten = glm::mix(1.0f, 0.2f, minAttenAlpha);
    if (dot > 0)
    {
        outRight = 1.0f;
        outLeft = glm::mix(1.0f, MinDirAtten, dot);
    }
    else
    {
        outLeft = 1.0f;
        outRight = glm::mix(1.0f, MinDirAtten, -dot);
    }

    outLeft *= volume;
    outRight *= volume;
}


void PlayAudio(
    uint32_t sourceIndex,
    SoundWave* soundWave,
    AudioComponent* component,
    float volumeMult,
    float pitchMult,
    int32_t priority,
    glm::vec3 position,
    float innerRadius,
    float outerRadius,
    AttenuationFunc attenFunc,
    bool loop,
    float startTime)
{
    OCT_ASSERT(sourceIndex < MAX_AUDIO_SOURCES);
    OCT_ASSERT(soundWave != nullptr);

    sAudioSources[sourceIndex].Set(
        soundWave,
        component,
        volumeMult,
        pitchMult,
        priority,
        position,
        innerRadius,
        outerRadius,
        attenFunc);

    float volume = volumeMult * soundWave->GetVolumeMultiplier();
    float pitch = pitchMult * soundWave->GetPitchMultiplier();

    if (component != nullptr)
    {
        component->NotifyAudible(true);
    }

    bool spatial = (innerRadius >= 0.0f &&
        outerRadius > 0.0f &&
        attenFunc != AttenuationFunc::Count);

    AUD_Play(
        sourceIndex,
        soundWave,
        volume,
        pitch,
        loop,
        startTime,
        spatial);
}

void StopAudio(uint32_t sourceIndex)
{
    if (sAudioSources[sourceIndex].mComponent != nullptr)
    {
        sAudioSources[sourceIndex].mComponent->NotifyAudible(false);
    }

    AUD_Stop(sourceIndex);

    sAudioSources[sourceIndex].Reset();
}

uint32_t FindAvailableAudioSourceIndex(int32_t inPriority)
{
    uint32_t availableIndex = MAX_AUDIO_SOURCES;
    int32_t lowestPriority = 0x7fffffff;
    uint32_t lowestPriorityIndex = 0;

    for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
    {
        if (sAudioSources[i].mSoundWave.Get() == nullptr)
        {
            availableIndex = i;
            break;
        }

        if (sAudioSources[i].mPriority < lowestPriority)
        {
            lowestPriority = sAudioSources[i].mPriority;
            lowestPriorityIndex = i;
        }
    }

    // All sources are being used. But see if we can evict one with lower priority
    if (availableIndex == MAX_AUDIO_SOURCES &&
        lowestPriority < inPriority)
    {
        LogWarning("Evicting lower priority sound");
        StopAudio(lowestPriorityIndex);
        availableIndex = lowestPriorityIndex;
    }

    return availableIndex;
}

void AudioManager::Initialize()
{

}

void AudioManager::Shutdown()
{

}

void AudioManager::Update(float deltaTime)
{
    SCOPED_FRAME_STAT("Audio");

    // TODO:
    // (1) -- Update Active Sources --
    //     Iterate through audio sources and update volume for any 3D sounds (including components)
    //     If an source has finished playing, Reset the source (and notify the component if applicable).
    //     Do not evict 3D sounds that are out of range. We want to hear them when we return.
    //     Evict components if out of hearing range.
    // (2) -- Play New Sounds --
    //     Iterate over AudioComponent list. If any component is playing and in range, but has no audio source active,
    //     then find an available audio source and Play(). Use component's mPlayTime var to start at correct time.


    // (1) Update Active Sources
    TransformComponent* listener = GetWorld()->GetAudioReceiver();
    glm::vec3 listenerPos = listener ? listener->GetAbsolutePosition() : glm::vec3(0,0,0);
    glm::vec3 listenerRight = listener ? listener->GetRightVector() : glm::vec3(1.0f, 0.0f, 0.0f);

    for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
    {
        if (sAudioSources[i].mSoundWave.Get() != nullptr)
        {
            SoundWave* soundWave = sAudioSources[i].mSoundWave.Get<SoundWave>();
            bool stopped = false;

            if (sAudioSources[i].mComponent != nullptr &&
                !sAudioSources[i].mComponent->IsPlaying())
            {
                // If the component has been stopped, then stop the source!
                StopAudio(i);
                stopped = true;
            }
            else if (!AUD_IsPlaying(i))
            {
                // If the audio engine has finished the sound wave, then stop it.
                if (sAudioSources[i].mComponent != nullptr &&
                    !sAudioSources[i].mComponent->GetLoop())
                {
                    sAudioSources[i].mComponent->Stop();
                }

                StopAudio(i);
                stopped = true;
            }
            else if (sAudioSources[i].mInnerRadius >= 0.0f &&
                sAudioSources[i].mOuterRadius > 0.0f &&
                sAudioSources[i].mAttenuationFunc != AttenuationFunc::Count)
            {
                // Update attenuation of the 3D sound
                if (sAudioSources[i].mComponent != nullptr)
                {
                    AudioComponent* comp = sAudioSources[i].mComponent;
                    sAudioSources[i].mPosition = comp->GetAbsolutePosition();
                    sAudioSources[i].mVolumeMult = comp->GetVolume();
                }

                float dist = glm::distance(listenerPos, sAudioSources[i].mPosition);

                if (sAudioSources[i].mComponent != nullptr && 
                    dist > sAudioSources[i].mOuterRadius)
                {
                    // Sound is no longer in hearing range.
                    // If this belongs to a component, Stop() the sound.
                    StopAudio(i);
                    stopped = true;
                }
                else
                {

#if 0
                    // Otherwise update the new volume
                    float volume = CalcVolumeAttenuation(
                        sAudioSources[i].mAttenuationFunc,
                        sAudioSources[i].mInnerRadius,
                        sAudioSources[i].mOuterRadius,
                        dist);

                    volume = volume * sAudioSources[i].mVolumeMult * soundWave->GetVolumeMultiplier();
                    AUD_SetVolume(i, volume, volume);
#else
                    float volLeft = 1.0f;
                    float volRight = 1.0f;

                    CalcVolumeAttenuationLR(sAudioSources[i].mAttenuationFunc,
                        sAudioSources[i].mInnerRadius,
                        sAudioSources[i].mOuterRadius,
                        sAudioSources[i].mPosition,
                        listenerPos,
                        listenerRight,
                        dist,
                        volLeft,
                        volRight);

                    volLeft = volLeft * sAudioSources[i].mVolumeMult * soundWave->GetVolumeMultiplier();
                    volRight = volRight * sAudioSources[i].mVolumeMult * soundWave->GetVolumeMultiplier();
                    AUD_SetVolume(i, volLeft, volRight);
#endif
                }

                if (!stopped &&
                    sAudioSources[i].mComponent != nullptr)
                {
                    if (sAudioSources[i].mPitchMult != sAudioSources[i].mComponent->GetPitch())
                    {
                        sAudioSources[i].mPitchMult = sAudioSources[i].mComponent->GetPitch();
                        AUD_SetPitch(i, sAudioSources[i].mPitchMult);
                    }
                }
            }
        }
    }

    // (2) Play New Sounds
    World* world = GetWorld();
    if (world != nullptr)
    {
        const std::vector<AudioComponent*>& components = GetWorld()->GetAudioComponents();

        for (uint32_t i = 0; i < components.size(); ++i)
        {
            AudioComponent* comp = components[i];

            // In the case that the component is playing, but it is inaudible (not a current sound source)
            // Then we need to check if it should be audible
            if (comp->IsPlaying() &&
                !comp->IsAudible() &&
                comp->GetVolume() > 0.0f &&
                comp->GetSoundWave() != nullptr)
            {
                // We need to check the distance to the listener. Should it be audible?
                glm::vec3 compPosition = comp->GetAbsolutePosition();
                float dist = glm::distance(listenerPos, compPosition);
                float outerRadius = glm::max(0.0f, comp->GetOuterRadius());

                if (dist < outerRadius)
                {
                    // It should be audible, so attempt to add it as a sound source.
                    uint32_t sourceIndex = FindAvailableAudioSourceIndex(comp->GetPriority());

                    float soundDuration = comp->GetSoundWave()->GetDuration();
                    float startTime = glm::mod(comp->GetStartOffset() + comp->GetPlayTime(), soundDuration);
                    if (startTime >= soundDuration)
                    {
                        startTime = 0.0f;
                    }

                    if (sourceIndex < MAX_AUDIO_SOURCES)
                    {
                        PlayAudio(
                            sourceIndex,
                            comp->GetSoundWave(),
                            comp,
                            comp->GetVolume(),
                            comp->GetPitch(),
                            comp->GetPriority(),
                            compPosition,
                            comp->GetInnerRadius(),
                            comp->GetOuterRadius(),
                            comp->GetAttenuationFunc(),
                            comp->GetLoop(),
                            startTime);
                    }
                }
            }
        }
    }
}

void AudioManager::PlaySound2D(
    SoundWave* soundWave,
    float volumeMult,
    float pitchMult,
    float startTime,
    bool loop,
    int32_t priority)
{
    uint32_t sourceIndex = FindAvailableAudioSourceIndex(priority);

    if (soundWave != nullptr && 
        sourceIndex < MAX_AUDIO_SOURCES)
    {
        PlayAudio(
            sourceIndex,
            soundWave,
            nullptr,
            volumeMult,
            pitchMult,
            priority,
            glm::vec3(0, 0, 0),
            -1.0f,
            -1.0f,
            AttenuationFunc::Count,
            loop,
            startTime);
    }
}

void AudioManager::PlaySound3D(
    SoundWave* soundWave,
    glm::vec3 worldPosition,
    float innerRadius,
    float outerRadius,
    AttenuationFunc attenFunc,
    float volumeMult,
    float pitchMult,
    float startTime,
    bool loop,
    int32_t priority)
{
    uint32_t sourceIndex = FindAvailableAudioSourceIndex(priority);

    if (soundWave != nullptr && 
        sourceIndex != MAX_AUDIO_SOURCES)
    {
        PlayAudio(
            sourceIndex,
            soundWave,
            nullptr,
            volumeMult,
            pitchMult,
            priority,
            worldPosition,
            innerRadius,
            outerRadius,
            attenFunc,
            loop,
            startTime);
    }
}

void AudioManager::UpdateSound(
    SoundWave* soundWave,
    float volume,
    float pitch,
    bool loop,
    int32_t priority)
{
    if (soundWave != nullptr)
    {
        for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
        {
            if (sAudioSources[i].mSoundWave == soundWave)
            {
                sAudioSources[i].mVolumeMult = volume;
                sAudioSources[i].mPitchMult = pitch;
                //sAudioSources[i].mLoop = loop;
                sAudioSources[i].mPriority = priority;
                
                AUD_SetVolume(i, volume, volume);
                AUD_SetPitch(i, pitch);

                break;
            }
        }
    }
}

void AudioManager::StopComponent(AudioComponent* comp)
{
    for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
    {
        if (sAudioSources[i].mComponent == comp)
        {
            StopAudio(i);
            break;
        }
    }
}

void AudioManager::StopSounds(SoundWave* soundWave)
{
    if (soundWave == nullptr)
        return;

    for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
    {
        if (sAudioSources[i].mSoundWave.Get() == soundWave)
        {
            StopAudio(i);
        }
    }
}

void AudioManager::StopSound(const std::string& name)
{
    for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
    {
        SoundWave* soundWave = sAudioSources[i].mSoundWave.Get<SoundWave>();

        if (soundWave && soundWave->GetName() == name)
        {
            StopAudio(i);
        }
    }
}

void AudioManager::StopAllSounds()
{
    for (uint32_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
    {
        if (sAudioSources[i].mSoundWave.Get() != nullptr)
        {
            StopAudio(i);
        }
    }
}
