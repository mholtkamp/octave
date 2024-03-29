#if PLATFORM_ANDROID

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"
#include "System/System.h"

#include "Assets/SoundWave.h"
#include "Log.h"
#include "Maths.h"
#include "Engine.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

static bool sInitialized = false;
static bool sPlaying = false;
static SLObjectItf sEngineObj = 0;
static SLEngineItf sEngine = 0;
static SLObjectItf sOutputMixObj = 0;

static SLObjectItf sPlayerObjs[AUDIO_MAX_VOICES] = { 0 };
static SLPlayItf sPlayers[AUDIO_MAX_VOICES] = { 0 };
static SLBufferQueueItf sBufferQueues[AUDIO_MAX_VOICES] = { 0 };
static SLVolumeItf sVolumes[AUDIO_MAX_VOICES] = { 0 };
static SLPitchItf sPitches[AUDIO_MAX_VOICES] = { 0 };

static bool sLoop[AUDIO_MAX_VOICES] = { };
static void* sSoundData[AUDIO_MAX_VOICES] = { };
static uint32_t sSoundSizes[AUDIO_MAX_VOICES] = { };


static void QueueCallback(SLBufferQueueItf caller, void *pContext)
{
    int32_t index = (int32_t)((size_t)pContext);

    if (index >= 0 && index < AUDIO_MAX_VOICES)
    {
        if (sLoop[index])
        {
            // Resubmit the same sound buffer
            (*(sBufferQueues[index]))->Clear(sBufferQueues[index]);

            (*(sBufferQueues[index]))->Enqueue(sBufferQueues[index],
                sSoundData[index],
                sSoundSizes[index]);
        }
    }
    else
    {
        LogError("Unknown buffer queue callback received");
    }
}

void AUD_Initialize()
{
    if (!sInitialized)
    {
        // Set initialization flag immediately.
        // If there is an error initializing, then initializing multiple
        // times won't result in a memory leak.
        sInitialized = true;

        LogDebug("Initializing Android Audio.");

        SLresult result;
        const SLuint32      engineMixIIDCount = 1;
        const SLInterfaceID engineMixIIDs[] = { SL_IID_ENGINE };
        const SLboolean     engineMixReqs[] = { SL_BOOLEAN_TRUE };
        const SLuint32      outputMixIIDCount = 0;
        const SLInterfaceID outputMixIIDs[] = {};
        const SLboolean     outputMixReqs[] = {};

        // Create the engine object
        result = slCreateEngine(&sEngineObj,
            0,
            0,
            engineMixIIDCount,
            engineMixIIDs,
            engineMixReqs);


        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error creating SL ES engine.");
            return;
        }

        // Realize the engine object to allocate memory
        result = (*sEngineObj)->Realize(sEngineObj, SL_BOOLEAN_FALSE);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error realizing the SL ES engine object.");
            return;
        }

        // Get engine object interface
        result = (*sEngineObj)->GetInterface(sEngineObj, SL_IID_ENGINE, &sEngine);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error retrieving SL engine interface.");
            return;
        }

        // Create audio output object
        result = (*sEngine)->CreateOutputMix(sEngine,
            &sOutputMixObj,
            outputMixIIDCount,
            outputMixIIDs,
            outputMixReqs);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error creating SL output mix object.");
            return;
        }

        // Realize the output mix object
        result = (*sOutputMixObj)->Realize(sOutputMixObj, SL_BOOLEAN_FALSE);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error realizing SL output mix object.");
            return;
        }


        // Now to create buffer queues for loading sound to be played.
        SLDataLocator_AndroidSimpleBufferQueue dataLocatorIn;
        dataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
        dataLocatorIn.numBuffers = 1;

        SLDataFormat_PCM dataFormat;
        dataFormat.formatType = SL_DATAFORMAT_PCM;
        dataFormat.numChannels = 2;
        dataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1;
        dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
        dataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
        dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
        dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;

        SLDataSource dataSource;
        dataSource.pLocator = &dataLocatorIn;
        dataSource.pFormat = &dataFormat;

        SLDataLocator_OutputMix dataLocatorOut;
        dataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
        dataLocatorOut.outputMix = sOutputMixObj;

        SLDataSink dataSink;
        dataSink.pLocator = &dataLocatorOut;
        dataSink.pFormat = 0;


        // Note: Apparently SL_IID_PITCH and SL_IID_RATEPITCH are not mandatory?
        // My Pixel 3A does not support these. So for now, pitch adjustment isn't possible.
        // I suppose a solution to this (and also for exceeding 0db volume limit) is to 
        // perform processing on the wave buffer? Need to research how other engines do this.
        const SLuint32 soundPlayerIIDCount = 3;
        const SLInterfaceID soundPlayerIIDs[] = { SL_IID_PLAY, SL_IID_BUFFERQUEUE, SL_IID_VOLUME };
        const SLboolean soundPlayerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

        for (int i = 0; i < AUDIO_MAX_VOICES; i++)
        {
            // Create
            result = (*sEngine)->CreateAudioPlayer(sEngine,
                &sPlayerObjs[i],
                &dataSource,
                &dataSink,
                soundPlayerIIDCount,
                soundPlayerIIDs,
                soundPlayerReqs);

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating audio player.");
                OCT_ASSERT(0);
                return;
            }

            // Realize
            result = (*(sPlayerObjs[i]))->Realize(sPlayerObjs[i], SL_BOOLEAN_FALSE);

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error realizing audio player.");
                OCT_ASSERT(0);
                return;
            }

            // Player Interface
            result = (*(sPlayerObjs[i]))->GetInterface(sPlayerObjs[i],
                SL_IID_PLAY,
                &(sPlayers[i]));

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating player interface.");
                OCT_ASSERT(0);
                return;
            }

            // Buffer Queue Interface
            result = (*(sPlayerObjs[i]))->GetInterface(sPlayerObjs[i],
                SL_IID_BUFFERQUEUE,
                &(sBufferQueues[i]));

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating buffer queue interface.");
                OCT_ASSERT(0);
                return;
            }

            (*(sBufferQueues[i]))->RegisterCallback(sBufferQueues[i], QueueCallback, (void*)i);

            // Volume Interface
            result = (*(sPlayerObjs[i]))->GetInterface(sPlayerObjs[i],
                SL_IID_VOLUME,
                &(sVolumes[i]));

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating volume interface.");
                OCT_ASSERT(0);
                return;
            }

            result = (*(sPlayers[i]))->SetPlayState(sPlayers[i], SL_PLAYSTATE_PLAYING);

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error setting audio player to play state.");
                OCT_ASSERT(0);
                return;
            }

            sPlaying = true;
        }
    }
}

void AUD_Shutdown()
{
    if (sInitialized != 0)
    {
        LogDebug("Shutting down Sound class.");

        // Destroy the audio players first
        for (int i = 0; i < AUDIO_MAX_VOICES; i++)
        {
            if (sPlayerObjs[i] != 0)
            {
                (*(sPlayerObjs[i]))->Destroy(sPlayerObjs[i]);
                sPlayerObjs[i] = 0;
                sPlayers[i] = 0;
                sBufferQueues[i] = 0;
            }
        }

        if (sOutputMixObj != 0)
        {
            (*sOutputMixObj)->Destroy(sOutputMixObj);
            sOutputMixObj = 0;
        }

        if (sEngineObj != 0)
        {
            (*sEngineObj)->Destroy(sEngineObj);
            sEngineObj = 0;
            sEngine = 0;
        }

        sInitialized = 0;
    }
}

void AUD_Update()
{   
    bool minimized = GetEngineState()->mWindowMinimized;

    if (minimized == sPlaying)
    {
        // If we are minimized we shouldn't be played.
        // If we are not minimized, we should be playing.
        sPlaying = !minimized;

        for (uint32_t i = 0; i < AUDIO_MAX_VOICES; ++i)
        {
            (*sPlayers[i])->SetPlayState(sPlayers[i], sPlaying ? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
        }
    }
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
    SLresult result;
    SLuint32 playerState;

    if (soundWave->GetBitsPerSample() != 16 ||
        soundWave->GetSampleRate() != 44100 ||
        soundWave->GetNumChannels() != 2)
    {
        LogDebug("Skipping sound %s", soundWave->GetName().c_str());
    }

    (*(sPlayerObjs[voiceIndex]))->GetState(sPlayerObjs[voiceIndex], &playerState);

    if (playerState == SL_OBJECT_STATE_REALIZED)
    {
        // Clear any sound that was there before
        result = (*(sBufferQueues[voiceIndex]))->Clear(sBufferQueues[voiceIndex]);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error clearing player queue.");
            return;
        }

        sLoop[voiceIndex] = loop;
        sSoundData[voiceIndex] = soundWave->GetWaveData();
        sSoundSizes[voiceIndex] = soundWave->GetWaveDataSize();

        // Add the new sound buffer to queue
        result = (*(sBufferQueues[voiceIndex]))->Enqueue(sBufferQueues[voiceIndex],
            soundWave->GetWaveData(),
            soundWave->GetWaveDataSize());

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Could not enqueue sound buffer to queue.");
            return;
        }

        AUD_SetVolume(voiceIndex, volume, volume);
        AUD_SetPitch(voiceIndex, pitch);
    }
}

void AUD_Stop(uint32_t voiceIndex)
{
    SLresult result = (*(sBufferQueues[voiceIndex]))->Clear(sBufferQueues[voiceIndex]);

    sLoop[voiceIndex] = false;
    sSoundData[voiceIndex] = nullptr;
    sSoundSizes[voiceIndex] = 0;

    if (result != SL_RESULT_SUCCESS)
    {
        LogError("Error stopping audio.");
        return;
    }
}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    bool playing = false;
    SLBufferQueueState queueState;
    SLresult result = (*(sBufferQueues[voiceIndex]))->GetState(sBufferQueues[voiceIndex], &queueState);

    if (result != SL_RESULT_SUCCESS)
    {
        LogError("Error retrieving audio play state.");
    }
    else
    {
        playing = (queueState.count > 0);
    }

    return playing;
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{
    float volume = (leftVolume + rightVolume) / 2.0f;

    // Convert volume from 0.0 to 1.0 range to millibels
    SLmillibel volMillibels = SL_MILLIBEL_MIN;
    if (volume < 0.00001)
    {
        volMillibels = SL_MILLIBEL_MIN;
    }
    else
    {
        volMillibels = (SLmillibel)(2000.f * log10f(volume) + 0.5f);
    }

    volMillibels = glm::clamp<SLmillibel>(volMillibels, SL_MILLIBEL_MIN, 0);

    SLresult result = (*(sVolumes[voiceIndex]))->SetVolumeLevel(sVolumes[voiceIndex], volMillibels);
    if (result != SL_RESULT_SUCCESS)
    {
        LogError("Error setting volume.");
    }
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
    if (soundWave->GetBitsPerSample() != 16 ||
        soundWave->GetSampleRate() != 44100 ||
        soundWave->GetNumChannels() != 2)
    {
        // Our OpenSL audio players are set to use 2-channel, 16-bit samples at 44100 Hz.
        // So if our source sound wave is different, we need to convert it to that format.

        uint32_t numSamples = soundWave->GetNumSamples();
        uint32_t numChannels = soundWave->GetNumChannels();
        uint32_t sampleRate = soundWave->GetSampleRate();
        uint32_t bytesPerSample = soundWave->GetBitsPerSample() / 8;

        uint32_t oldSize = numSamples * bytesPerSample;
        uint32_t newSize = numSamples * 2; // 2 bytes per sample

        if (numChannels == 1)
        {
            // Need to add samples for second channel
            newSize *= 2;
        }

        if (sampleRate == 22050)
        {
            // Need to interpolate extra samples so it sounds normal at 44100 Hz.
            newSize *= 2;
        }

        uint8_t* newWave = AUD_AllocWaveBuffer(newSize);
        uint8_t* oldWave = soundWave->GetWaveData();

        // (1) Convert to 16 bit
        if (bytesPerSample == 1)
        {
            for (int32_t i = int32_t(numSamples) - 1; i >= 0; --i)
            {
                int16_t sample = (int16_t)oldWave[i];
                sample = sample * 256 - 32767;
                *((int16_t*)(newWave + i * 2)) = sample;
            }
        }
        else
        {
            // Copy the samples to newWave since the next two loops work in-place on newWave buffer.
            memcpy(newWave, oldWave, oldSize);
        }

        //  Interpolate to 44100
        if (sampleRate == 22050)
        {
            int16_t* samples = (int16_t*)newWave;

            // Work backwards so we don't overwrite samples
            for (int32_t i = int32_t(numSamples) - 1; i >= 0; --i)
            {
                if (i == (numSamples - 1))
                {
                    samples[i * 2] = samples[i];
                    // Interpolate with 0? Essentially multiply 0.5f.
                    samples[i * 2 + 1] = int16_t((samples[i] * 0.5f) + 0.5f);
                }
                else
                {
                    samples[i * 2] = samples[i];
                    samples[i * 2 + 1] = int16_t((samples[i] + samples[i + 1]) * 0.5f + 0.5f);
                }
            }

            // We just doubled our sample count.
            numSamples *= 2;
        }

        // Duplicate to stereo
        if (numChannels == 1)
        {
            int16_t* samples = (int16_t*)newWave;

            for (int32_t i = int32_t(numSamples) - 1; i >= 0; --i)
            {
                samples[i * 2] = samples[i];
                samples[i * 2 + 1] = samples[i];
            }

            // We just doubled our sample count.
            numSamples *= 2;
        }

        soundWave->SetPcmData(newWave, newSize, numSamples, 16, 2, 44100);

        AUD_FreeWaveBuffer(oldWave);
        oldWave = nullptr;
    }
}

#endif
