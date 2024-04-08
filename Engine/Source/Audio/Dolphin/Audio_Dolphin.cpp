#if PLATFORM_DOLPHIN

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"

#include "Assets/SoundWave.h"
#include "Log.h"

#include "System/System.h"

#include <gccore.h>
#include <asndlib.h>

static int32_t sSampleRates[AUDIO_MAX_VOICES] = {};

void AUD_Initialize()
{
    ASND_Init();
    ASND_Pause(0);
}

void AUD_Shutdown()
{
    ASND_End();
}

void AUD_Update()
{
    
}

void AUD_Play(
    uint32_t voiceIndex,
    SoundWave* soundWave,
    float volume,
    float pitch,
    bool loop,
    float startTime,
    bool spatial)
{
    //float startPercent = startTime / soundWave->GetDuration();
    //startPercent = glm::clamp(startPercent, 0.0f, 1.0f);
    //uint32_t startingSample = uint32_t(startPercent * soundWave->GetNumSamples());

    int32_t volumeInt = int32_t(volume * MID_VOLUME);
    int32_t pitchHz = int32_t(soundWave->GetSampleRate() * pitch);
    int32_t voiceFormat = 0;

    sSampleRates[voiceIndex] = soundWave->GetSampleRate();

    bool stereo = (soundWave->GetNumChannels() == 2);
    bool bit16 = (soundWave->GetBitsPerSample() == 16);

    // Wave data is stored in little endian format
    if (stereo)
    {
        voiceFormat = bit16 ? VOICE_STEREO_16BIT_LE : VOICE_STEREO_8BIT_U;
    }
    else
    {
        voiceFormat = bit16 ? VOICE_MONO_16BIT_LE : VOICE_MONO_8BIT_U;
    }

    if (loop)
    {
        ASND_SetInfiniteVoice(
            voiceIndex,
            voiceFormat,
            pitchHz,
            0,
            soundWave->GetWaveData(),
            soundWave->GetWaveDataSize(),
            spatial ? 0 : volumeInt,
            spatial ? 0 : volumeInt);
    }
    else
    {
        ASND_SetVoice(
            voiceIndex,
            voiceFormat,
            pitchHz,
            0,
            soundWave->GetWaveData(),
            soundWave->GetWaveDataSize(),
            spatial ? 0 : volumeInt,
            spatial ? 0 : volumeInt,
            nullptr);
    }
}

void AUD_Stop(uint32_t voiceIndex)
{
    ASND_StopVoice(voiceIndex);

}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    int32_t status = ASND_StatusVoice(voiceIndex);
    return status == SND_WORKING;
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{
    int32_t leftVolInt = int32_t(MID_VOLUME * leftVolume);
    int32_t rightVolInt = int32_t(MID_VOLUME * rightVolume);
    ASND_ChangeVolumeVoice(voiceIndex, leftVolInt, rightVolInt);
}

void AUD_SetPitch(uint32_t voiceIndex, float pitch)
{
    int32_t pitchHz = int32_t(pitch * sSampleRates[voiceIndex]);
    ASND_ChangePitchVoice(voiceIndex, pitchHz);
}

uint8_t* AUD_AllocWaveBuffer(uint32_t size)
{
    return (uint8_t*)SYS_AlignedMalloc(size, 32);
}

void AUD_FreeWaveBuffer(void* buffer)
{
    SYS_AlignedFree(buffer);
}

void AUD_ProcessWaveBuffer(SoundWave* soundWave)
{

}

#endif