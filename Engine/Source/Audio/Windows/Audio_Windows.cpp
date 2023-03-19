#if PLATFORM_WINDOWS

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"
#include "System/System.h"

#include "Assets/SoundWave.h"
#include "Log.h"

#include <xaudio2.h>

static IXAudio2* sXAudio2 = nullptr;
static IXAudio2MasteringVoice* sMasterVoice = nullptr;
static IXAudio2SourceVoice* sSourceVoices[AUDIO_MAX_VOICES] = { };
static XAUDIO2_BUFFER sSourceBuffers[AUDIO_MAX_VOICES] = { };
static uint8_t* sStereoConvertedBuffers[AUDIO_MAX_VOICES] = { };

// An attempt to reuse source voices?
//struct WaveFormat
//{
//    uint32_t mSampleRate = 44100;
//    uint8_t mNumChannels = 2;
//    uint8_t mBitsPerSample = 16;
//};
//static WaveFormat sLastWaveFormats[AUDIO_MAX_VOICES] = {};

void AUD_Initialize()
{
    if (XAudio2Create(&sXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR) < 0)
    {
        LogError("Failed to create XAudio2 engine");
    }

    if (sXAudio2->CreateMasteringVoice(&sMasterVoice) < 0)
    {
        LogError("Failed to create mastering voice");
    }

    // TODO: Prime the XAudio2 internal memory pool for source voices by allocating many.
    // And then immediately destroying them I suppose?
}

void AUD_Shutdown()
{
    for (uint32_t i = 0; i < AUDIO_MAX_VOICES; ++i)
    {
        if (sSourceVoices[i] != nullptr)
        {
            sSourceVoices[i]->DestroyVoice();
            // TODO: Do we need to call delete??
            sSourceVoices[i] = nullptr;
        }
    }
    sMasterVoice->DestroyVoice();
    sXAudio2->Release();
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
    OCT_ASSERT(sSourceVoices[voiceIndex] == nullptr);

    bool monoInput = (soundWave->GetNumChannels() == 1);
    if (monoInput)
    {
        OCT_ASSERT(sStereoConvertedBuffers[voiceIndex] == nullptr);
        sStereoConvertedBuffers[voiceIndex] = new uint8_t[soundWave->GetWaveDataSize() * 2];

        uint32_t numSamples = soundWave->GetNumSamples();
        uint32_t sampleSize = (soundWave->GetBitsPerSample() == 8) ? 1 : 2;

        uint8_t* srcData = soundWave->GetWaveData();
        uint8_t* dstData = sStereoConvertedBuffers[voiceIndex];

        for (uint32_t i = 0; i < numSamples; ++i)
        {
            memcpy(dstData, srcData, sampleSize);
            memcpy(dstData + sampleSize, srcData, sampleSize);

            srcData += sampleSize;
            dstData += (sampleSize * 2);
        }
    }

    float startPercent = startTime / soundWave->GetDuration();
    startPercent = glm::clamp(startPercent, 0.0f, 1.0f);
    uint32_t startingSample = uint32_t(startPercent * soundWave->GetNumSamples());

    sSourceBuffers[voiceIndex].Flags = XAUDIO2_END_OF_STREAM;
    sSourceBuffers[voiceIndex].AudioBytes = soundWave->GetWaveDataSize();
    sSourceBuffers[voiceIndex].pAudioData = soundWave->GetWaveData();
    sSourceBuffers[voiceIndex].PlayBegin = startingSample;
    sSourceBuffers[voiceIndex].PlayLength = 0;
    sSourceBuffers[voiceIndex].LoopBegin = 0;
    sSourceBuffers[voiceIndex].LoopLength = 0;
    sSourceBuffers[voiceIndex].LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    WAVEFORMATEX waveFormat = {};
    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    waveFormat.nChannels = soundWave->GetNumChannels();
    waveFormat.nSamplesPerSec = soundWave->GetSampleRate();
    waveFormat.wBitsPerSample = (uint16_t)soundWave->GetBitsPerSample();
    waveFormat.nBlockAlign = soundWave->GetBlockAlign();
    waveFormat.nAvgBytesPerSec = soundWave->GetByteRate();
    waveFormat.cbSize = 0;

    if (monoInput)
    {
        // Use ephemeral stereo buffer
        sSourceBuffers[voiceIndex].AudioBytes *= 2;
        sSourceBuffers[voiceIndex].pAudioData = sStereoConvertedBuffers[voiceIndex];
        waveFormat.nAvgBytesPerSec *= 2;
        waveFormat.nBlockAlign *= 2;
        waveFormat.nChannels = 2;
    }

    if (sXAudio2->CreateSourceVoice(&sSourceVoices[voiceIndex], &waveFormat) >= 0)
    {
        sSourceVoices[voiceIndex]->SubmitSourceBuffer(&sSourceBuffers[voiceIndex]);

        // Spatial sounds will update their volume every frame.
        sSourceVoices[voiceIndex]->SetVolume(spatial ? 0.0f : volume);

        sSourceVoices[voiceIndex]->SetFrequencyRatio(pitch);
        sSourceVoices[voiceIndex]->Start();
    }
    else
    {
        LogError("Error creating XAUDIO2 source voice");
        OCT_ASSERT(0);
    }
}

void AUD_Stop(uint32_t voiceIndex)
{
    OCT_ASSERT(sSourceVoices[voiceIndex] != nullptr);
    sSourceVoices[voiceIndex]->Stop();
    sSourceVoices[voiceIndex]->DestroyVoice();
    sSourceVoices[voiceIndex] = nullptr;

    if (sStereoConvertedBuffers[voiceIndex] != nullptr)
    {
        delete sStereoConvertedBuffers[voiceIndex];
        sStereoConvertedBuffers[voiceIndex] = nullptr;
    }
}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    OCT_ASSERT(sSourceVoices[voiceIndex] != nullptr);
    XAUDIO2_VOICE_STATE state;
    sSourceVoices[voiceIndex]->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    return (state.BuffersQueued != 0);
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{
    OCT_ASSERT(sSourceVoices[voiceIndex] != nullptr);

    // Use this version to set volume of all channels
    // sSourceVoices[voiceIndex]->SetVolume((leftVolume + rightVolume) / 2.0f);

    // Use this version to set volume of left/right ear
    sSourceVoices[voiceIndex]->SetVolume(1.0f);
    float volumes[2] = { leftVolume, rightVolume };
    sSourceVoices[voiceIndex]->SetChannelVolumes(2, volumes);
}

void AUD_SetPitch(uint32_t voiceIndex, float pitch)
{
    OCT_ASSERT(sSourceVoices[voiceIndex] != nullptr);
    sSourceVoices[voiceIndex]->SetFrequencyRatio(pitch);
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
