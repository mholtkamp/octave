#pragma once

#include "EngineTypes.h"

class SoundWave;
class AudioComponent;

class AudioManager
{
public:

    static void Initialize();
    static void Shutdown();
    static void Update(float deltaTime);

    static void PlaySound2D(
        SoundWave* soundWave,
        float volumeMult = 1.0f,
        float pitchMult = 1.0f,
        float startTime = 0.0f,
        bool loop = false,
        int32_t priority = 0);

    static void PlaySound3D(
        SoundWave* soundWave,
        glm::vec3 worldPosition,
        float innerRadius,
        float outerRadius,
        AttenuationFunc attenFunc = AttenuationFunc::Linear,
        float volumeMult = 1.0f,
        float pitchMult = 1.0f,
        float startTime = 0.0f,
        bool loop = false,
        int32_t priority = 0);

    static void UpdateSound(
        SoundWave* soundWave,
        float volume,
        float pitch,
        bool loop = false,
        int32_t priority = 0);

    static void StopComponent(AudioComponent* comp);
    static void StopSounds(SoundWave* soundWave);
    static void StopSound(const std::string& name);
    static void StopAllSounds();

    static void SetAudioClassVolume(int8_t audioClass, float volume);
    static void SetAudioClassPitch(int8_t audioClass, float pitch);
    static float GetAudioClassVolume(int8_t audioClass);
    static float GetAudioClassPitch(int8_t audioClass);

    static void SetMasterVolume(float volume);
    static void SetMasterPitch(float pitch);
    static float GetMasterVolume();
    static float GetMasterPitch();
};
