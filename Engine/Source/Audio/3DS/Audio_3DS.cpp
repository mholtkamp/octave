#if PLATFORM_3DS

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"

#include "Assets/SoundWave.h"
#include "Log.h"

#include <3ds.h>

#define USE_DSP 1

static float sSampleRates[AUDIO_MAX_VOICES] = {};
static ndspWaveBuf sWaveBufs[AUDIO_MAX_VOICES] = {};

void AUD_Initialize()
{
#if USE_DSP
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
#else
    csndInit();
#endif
}

void AUD_Shutdown()
{
#if USE_DSP
    ndspExit();
#else
    csndExit();
#endif
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
    bool stereo = (soundWave->GetNumChannels() == 2);
    bool bit16 = (soundWave->GetBitsPerSample() == 16);
    uint32_t sampleRate = soundWave->GetSampleRate();

    uint16_t voiceFormat = 0;

    if (stereo)
    {
        voiceFormat = bit16 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_STEREO_PCM8;
    }
    else
    {
        voiceFormat = bit16 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_MONO_PCM8;
    }

    sSampleRates[voiceIndex] = soundWave->GetSampleRate();

    float volumeLeft = spatial ? 0.0f : 1.0f;
    float volumeRight = spatial ? 0.0f : 1.0f;

#if USE_DSP
    ndspChnReset(voiceIndex);

    ndspChnSetInterp(voiceIndex, NDSP_INTERP_LINEAR);
    ndspChnSetRate(voiceIndex, (float)sampleRate);
    ndspChnSetFormat(voiceIndex, voiceFormat);

    // Might not be needed really...
    DSP_FlushDataCache(soundWave->GetWaveData(), soundWave->GetWaveDataSize());

    // Output at 100% on the first pair of left and right channels.
    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = volumeLeft;
    mix[1] = volumeRight;
    ndspChnSetMix(voiceIndex, mix);

    sWaveBufs[voiceIndex] = {};
    sWaveBufs[voiceIndex].data_vaddr = soundWave->GetWaveData();
    sWaveBufs[voiceIndex].nsamples = soundWave->GetNumSamples() / soundWave->GetNumChannels();
    sWaveBufs[voiceIndex].looping = loop;

    ndspChnWaveBufAdd(voiceIndex, &sWaveBufs[voiceIndex]);
#else

#endif
}

void AUD_Stop(uint32_t voiceIndex)
{
    //ndspChnSetPaused(voiceIndex, true);
    ndspChnWaveBufClear(voiceIndex);
    
    //ndspChnReset(voiceIndex);
}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    // Using ndspChnIsPlaying() may return false if you just added the sound.
    // Probably due to the multithreaded nature of the ndsp code.
    //return ndspChnIsPlaying(voiceIndex);

    return sWaveBufs[voiceIndex].status != NDSP_WBUF_DONE;
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{
    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = leftVolume;
    mix[1] = rightVolume;
    ndspChnSetMix(voiceIndex, mix);
}

void AUD_SetPitch(uint32_t voiceIndex, float pitch)
{
    float pitchHz = pitch * sSampleRates[voiceIndex];
    ndspChnSetRate(voiceIndex, pitchHz);
}

uint8_t* AUD_AllocWaveBuffer(uint32_t size)
{
    return (uint8_t*)linearAlloc(size);
}

void AUD_FreeWaveBuffer(void* buffer)
{
    linearFree(buffer);
}

void AUD_ProcessWaveBuffer(SoundWave* soundWave)
{
    if (soundWave->GetBitsPerSample() == 8)
    {
        uint8_t* waveData = soundWave->GetWaveData();

        // Convert from unsigned to signed PCM
        for (uint32_t i = 0; i < soundWave->GetNumSamples(); ++i)
        {
            int32_t signedValue = (int32_t) waveData[i];
            signedValue -= 128;
            signedValue = glm::clamp(signedValue, int32_t(-128), int32_t(127));
            int8_t signedChar = (int8_t)signedValue;
            memcpy(&waveData[i], &signedChar, sizeof(int8_t));
        }
    }
}

#endif