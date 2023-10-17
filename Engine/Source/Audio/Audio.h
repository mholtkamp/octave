#pragma once

#include "EngineTypes.h"

class Stream;
class SoundWave;
class AudioComponent;

struct PcmFormat
{
    uint32_t mNumChannels = 2;
    uint32_t mSampleRate = 44100;
    uint32_t mBytesPerSample = 2;
};

void AUD_Initialize();
void AUD_Shutdown();
void AUD_Update();

void AUD_Play(
    uint32_t voiceIndex,
    SoundWave* soundWave,
    float volume,
    float pitch,
    bool loop,
    float startTime,
    bool spatial);

void AUD_Stop(uint32_t voiceIndex);
bool AUD_IsPlaying(uint32_t voiceIndex);
void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume);
void AUD_SetPitch(uint32_t voiceIndex, float pitch);

uint8_t* AUD_AllocWaveBuffer(uint32_t size);
void AUD_FreeWaveBuffer(void* buffer);
void AUD_ProcessWaveBuffer(SoundWave* soundWave);

// Platform Independent
void AUD_EncodeVorbis(Stream& inStream, Stream& outStream, PcmFormat format);
void AUD_DecodeVorbis(Stream& inStream, Stream& outStream, PcmFormat format);
