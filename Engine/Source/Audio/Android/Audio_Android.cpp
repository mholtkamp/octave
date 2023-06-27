#if PLATFORM_ANDROID

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"
#include "System/System.h"

#include "Assets/SoundWave.h"
#include "Log.h"
#include "Maths.h"


void AUD_Initialize()
{

}

void AUD_Shutdown()
{

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

}

void AUD_Stop(uint32_t voiceIndex)
{
}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    return false;
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{

}

void AUD_SetPitch(uint32_t voiceIndex, float pitch)
{
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
