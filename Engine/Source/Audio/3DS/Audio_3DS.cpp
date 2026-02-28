#if PLATFORM_3DS

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"

#include "Assets/SoundWave.h"
#include "Log.h"

#include <3ds.h>

// Audio backend: try NDSP first, fall back to CSND.
enum AudioBackend
{
    AUDIO_BACKEND_NONE,
    AUDIO_BACKEND_NDSP,
    AUDIO_BACKEND_CSND
};

static AudioBackend sBackend = AUDIO_BACKEND_NONE;
static float sSampleRates[AUDIO_MAX_VOICES] = {};

// NDSP state
static ndspWaveBuf sWaveBufs[AUDIO_MAX_VOICES] = {};

// CSND state
// CSND channels 8-31 are available for user audio. Map voice 0-7 to channels 8-15.
#define CSND_CHANNEL_BASE 8
// CSND is mono per channel; for stereo sources we mix down to a mono buffer.
static uint8_t* sCsndMonoBuffers[AUDIO_MAX_VOICES] = {};
static uint32_t sCsndMonoSizes[AUDIO_MAX_VOICES] = {};
static bool sCsndPlaying[AUDIO_MAX_VOICES] = {};

void AUD_Initialize()
{
    // Try NDSP first (requires DSP firmware)
    LogDebug("AUD_Initialize: trying ndspInit...");
    Result rc = ndspInit();
    if (R_SUCCEEDED(rc))
    {
        sBackend = AUDIO_BACKEND_NDSP;
        ndspSetOutputMode(NDSP_OUTPUT_STEREO);

        for (uint32_t i = 0; i < AUDIO_MAX_VOICES; ++i)
        {
            sWaveBufs[i].status = NDSP_WBUF_DONE;
        }
        LogDebug("AUD_Initialize: NDSP initialized successfully");
        return;
    }
    LogWarning("AUD_Initialize: ndspInit failed (0x%08lx), trying CSND...", rc);

    // Fall back to CSND
    rc = csndInit();
    if (R_SUCCEEDED(rc))
    {
        sBackend = AUDIO_BACKEND_CSND;
        LogDebug("AUD_Initialize: CSND initialized successfully");
        return;
    }

    LogError("AUD_Initialize: Both NDSP and CSND failed. No audio available.");
}

void AUD_Shutdown()
{
    if (sBackend == AUDIO_BACKEND_NDSP)
    {
        ndspExit();
    }
    else if (sBackend == AUDIO_BACKEND_CSND)
    {
        for (uint32_t i = 0; i < AUDIO_MAX_VOICES; ++i)
        {
            if (sCsndMonoBuffers[i] != nullptr)
            {
                linearFree(sCsndMonoBuffers[i]);
                sCsndMonoBuffers[i] = nullptr;
            }
        }
        csndExit();
    }
    sBackend = AUDIO_BACKEND_NONE;
}

void AUD_Update()
{

}

// Helper: convert interleaved stereo to mono by averaging L+R samples.
// Returns a linearAlloc'd buffer that the caller must track and free.
static uint8_t* MixStereoToMono(const uint8_t* stereoData, uint32_t numSamples, bool bit16, uint32_t& outSize)
{
    uint32_t monoSamples = numSamples / 2;
    uint32_t bytesPerSample = bit16 ? 2 : 1;
    outSize = monoSamples * bytesPerSample;

    uint8_t* mono = (uint8_t*)linearAlloc(outSize);
    if (mono == nullptr)
    {
        return nullptr;
    }

    if (bit16)
    {
        const int16_t* src = (const int16_t*)stereoData;
        int16_t* dst = (int16_t*)mono;
        for (uint32_t i = 0; i < monoSamples; ++i)
        {
            int32_t left = src[i * 2];
            int32_t right = src[i * 2 + 1];
            dst[i] = (int16_t)((left + right) / 2);
        }
    }
    else
    {
        for (uint32_t i = 0; i < monoSamples; ++i)
        {
            int32_t left = (int8_t)stereoData[i * 2];
            int32_t right = (int8_t)stereoData[i * 2 + 1];
            mono[i] = (uint8_t)(int8_t)((left + right) / 2);
        }
    }

    return mono;
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
    if (sBackend == AUDIO_BACKEND_NONE)
    {
        return;
    }

    if (soundWave->GetWaveData() == nullptr || soundWave->GetWaveDataSize() == 0)
    {
        LogError("AUD_Play: No wave data (data=%p size=%u)", soundWave->GetWaveData(), soundWave->GetWaveDataSize());
        return;
    }

    bool stereo = (soundWave->GetNumChannels() == 2);
    bool bit16 = (soundWave->GetBitsPerSample() == 16);
    uint32_t sampleRate = soundWave->GetSampleRate();
    uint32_t nsamples = soundWave->GetNumSamples() / soundWave->GetNumChannels();

    sSampleRates[voiceIndex] = (float)sampleRate;

    float volumeLeft = spatial ? 0.0f : volume;
    float volumeRight = spatial ? 0.0f : volume;

    LogDebug("AUD_Play [%s]: voice=%u ch=%u bits=%u rate=%u samples=%u size=%u vol=%.2f pitch=%.2f loop=%d",
        (sBackend == AUDIO_BACKEND_NDSP) ? "NDSP" : "CSND",
        voiceIndex, soundWave->GetNumChannels(), soundWave->GetBitsPerSample(),
        sampleRate, nsamples, soundWave->GetWaveDataSize(),
        volume, pitch, (int)loop);

    if (sBackend == AUDIO_BACKEND_NDSP)
    {
        uint16_t voiceFormat = 0;
        if (stereo)
        {
            voiceFormat = bit16 ? NDSP_FORMAT_STEREO_PCM16 : NDSP_FORMAT_STEREO_PCM8;
        }
        else
        {
            voiceFormat = bit16 ? NDSP_FORMAT_MONO_PCM16 : NDSP_FORMAT_MONO_PCM8;
        }

        ndspChnReset(voiceIndex);
        ndspChnSetInterp(voiceIndex, NDSP_INTERP_LINEAR);
        ndspChnSetRate(voiceIndex, (float)sampleRate * pitch);
        ndspChnSetFormat(voiceIndex, voiceFormat);

        float mix[12];
        memset(mix, 0, sizeof(mix));
        mix[0] = volumeLeft;
        mix[1] = volumeRight;
        ndspChnSetMix(voiceIndex, mix);

        memset(&sWaveBufs[voiceIndex], 0, sizeof(ndspWaveBuf));
        sWaveBufs[voiceIndex].data_vaddr = soundWave->GetWaveData();
        sWaveBufs[voiceIndex].nsamples = nsamples;
        sWaveBufs[voiceIndex].looping = loop;

        DSP_FlushDataCache(soundWave->GetWaveData(), soundWave->GetWaveDataSize());
        ndspChnWaveBufAdd(voiceIndex, &sWaveBufs[voiceIndex]);
    }
    else if (sBackend == AUDIO_BACKEND_CSND)
    {
        uint32_t chn = CSND_CHANNEL_BASE + voiceIndex;

        // Free previous mono mixdown buffer if any
        if (sCsndMonoBuffers[voiceIndex] != nullptr)
        {
            linearFree(sCsndMonoBuffers[voiceIndex]);
            sCsndMonoBuffers[voiceIndex] = nullptr;
            sCsndMonoSizes[voiceIndex] = 0;
        }

        uint8_t* playData = soundWave->GetWaveData();
        uint32_t playSize = soundWave->GetWaveDataSize();

        // CSND channels are mono. Mix stereo down to mono.
        if (stereo)
        {
            uint32_t monoSize = 0;
            uint8_t* monoData = MixStereoToMono(playData, soundWave->GetNumSamples(), bit16, monoSize);
            if (monoData == nullptr)
            {
                LogError("AUD_Play [CSND]: Failed to allocate mono mixdown buffer");
                return;
            }
            sCsndMonoBuffers[voiceIndex] = monoData;
            sCsndMonoSizes[voiceIndex] = monoSize;
            playData = monoData;
            playSize = monoSize;
        }

        u32 flags = SOUND_LINEAR_INTERP;
        flags |= bit16 ? SOUND_FORMAT_16BIT : SOUND_FORMAT_8BIT;
        flags |= loop ? SOUND_REPEAT : SOUND_ONE_SHOT;

        float vol = spatial ? 0.0f : volume;
        float pan = 0.0f; // center

        uint32_t pitchedRate = (uint32_t)((float)sampleRate * pitch);

        GSPGPU_FlushDataCache(playData, playSize);

        csndPlaySound(chn, flags, pitchedRate, vol, pan,
            playData,
            loop ? playData : NULL,
            playSize);

        sCsndPlaying[voiceIndex] = true;
        LogDebug("AUD_Play [CSND]: playing on channel %u", chn);
    }
}

void AUD_Stop(uint32_t voiceIndex)
{
    if (sBackend == AUDIO_BACKEND_NDSP)
    {
        ndspChnWaveBufClear(voiceIndex);
        sWaveBufs[voiceIndex].status = NDSP_WBUF_DONE;
    }
    else if (sBackend == AUDIO_BACKEND_CSND)
    {
        uint32_t chn = CSND_CHANNEL_BASE + voiceIndex;
        CSND_SetPlayState(chn, 0);
        csndExecCmds(true);
        sCsndPlaying[voiceIndex] = false;

        if (sCsndMonoBuffers[voiceIndex] != nullptr)
        {
            linearFree(sCsndMonoBuffers[voiceIndex]);
            sCsndMonoBuffers[voiceIndex] = nullptr;
            sCsndMonoSizes[voiceIndex] = 0;
        }
    }
}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    if (sBackend == AUDIO_BACKEND_NDSP)
    {
        return sWaveBufs[voiceIndex].status != NDSP_WBUF_DONE;
    }
    else if (sBackend == AUDIO_BACKEND_CSND)
    {
        if (!sCsndPlaying[voiceIndex])
        {
            return false;
        }

        uint32_t chn = CSND_CHANNEL_BASE + voiceIndex;
        u8 playing = 0;
        csndIsPlaying(chn, &playing);
        if (playing == 0)
        {
            sCsndPlaying[voiceIndex] = false;

            if (sCsndMonoBuffers[voiceIndex] != nullptr)
            {
                linearFree(sCsndMonoBuffers[voiceIndex]);
                sCsndMonoBuffers[voiceIndex] = nullptr;
                sCsndMonoSizes[voiceIndex] = 0;
            }
        }
        return playing != 0;
    }

    return false;
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{
    if (sBackend == AUDIO_BACKEND_NDSP)
    {
        float mix[12];
        memset(mix, 0, sizeof(mix));
        mix[0] = leftVolume;
        mix[1] = rightVolume;
        ndspChnSetMix(voiceIndex, mix);
    }
    else if (sBackend == AUDIO_BACKEND_CSND)
    {
        uint32_t chn = CSND_CHANNEL_BASE + voiceIndex;
        // Pack L/R volumes into the format CSND expects
        u32 lvol = (u32)(glm::clamp(leftVolume, 0.0f, 1.0f) * 0x8000);
        u32 rvol = (u32)(glm::clamp(rightVolume, 0.0f, 1.0f) * 0x8000);
        u32 volumes = lvol | (rvol << 16);
        CSND_SetVol(chn, volumes, volumes);
        csndExecCmds(true);
    }
}

void AUD_SetPitch(uint32_t voiceIndex, float pitch)
{
    if (sBackend == AUDIO_BACKEND_NDSP)
    {
        float pitchHz = pitch * sSampleRates[voiceIndex];
        ndspChnSetRate(voiceIndex, pitchHz);
    }
    else if (sBackend == AUDIO_BACKEND_CSND)
    {
        uint32_t chn = CSND_CHANNEL_BASE + voiceIndex;
        u32 sampleRate = (u32)(pitch * sSampleRates[voiceIndex]);
        u32 timer = CSND_TIMER(sampleRate);
        if (timer < 0x0042) timer = 0x0042;
        else if (timer > 0xFFFF) timer = 0xFFFF;
        CSND_SetTimer(chn, timer);
        csndExecCmds(true);
    }
}

uint8_t* AUD_AllocWaveBuffer(uint32_t size)
{
    uint8_t* buffer = (uint8_t*)linearAlloc(size);
    if (buffer == nullptr)
    {
        LogError("AUD_AllocWaveBuffer: linearAlloc failed for size %u", size);
    }
    return buffer;
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
