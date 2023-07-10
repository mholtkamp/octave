#if PLATFORM_ANDROID

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"
#include "System/System.h"

#include "Assets/SoundWave.h"
#include "Log.h"
#include "Maths.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

static int s_nInitialized = 0;
static android_app* s_pApp = 0;
static SLObjectItf s_slEngineObj = 0;
static SLEngineItf s_slEngine = 0;
static SLObjectItf s_slOutputMixObj = 0;

static SLObjectItf      s_arPlayerObjs[AUDIO_MAX_VOICES] = { 0 };
static SLPlayItf        s_arPlayers[AUDIO_MAX_VOICES] = { 0 };
static SLBufferQueueItf s_arPlayerQueues[AUDIO_MAX_VOICES];

void AUD_Initialize()
{
    if (s_nInitialized == 0)
    {
        // Set initialization flag immediately.
        // If there is an error initializing, then initializing multiple
        // times won't result in a memory leak.
        s_nInitialized = 1;

        LogDebug("Initializing Sound class.");

        SLresult result;
        const SLuint32      engineMixIIDCount = 1;
        const SLInterfaceID engineMixIIDs[] = { SL_IID_ENGINE };
        const SLboolean     engineMixReqs[] = { SL_BOOLEAN_TRUE };
        const SLuint32      outputMixIIDCount = 0;
        const SLInterfaceID outputMixIIDs[] = {};
        const SLboolean     outputMixReqs[] = {};

        // Create the engine object
        result = slCreateEngine(&s_slEngineObj,
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
        result = (*s_slEngineObj)->Realize(s_slEngineObj, SL_BOOLEAN_FALSE);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error realizing the SL ES engine object.");
            return;
        }

        // Get engine object interface
        result = (*s_slEngineObj)->GetInterface(s_slEngineObj, SL_IID_ENGINE, &s_slEngine);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error retrieving SL engine interface.");
            return;
        }

        // Create audio output object
        result = (*s_slEngine)->CreateOutputMix(s_slEngine,
            &s_slOutputMixObj,
            outputMixIIDCount,
            outputMixIIDs,
            outputMixReqs);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error creating SL output mix object.");
            return;
        }

        // Realize the output mix object
        result = (*s_slOutputMixObj)->Realize(s_slOutputMixObj, SL_BOOLEAN_FALSE);

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
        dataLocatorOut.outputMix = s_slOutputMixObj;

        SLDataSink dataSink;
        dataSink.pLocator = &dataLocatorOut;
        dataSink.pFormat = 0;

        const SLuint32 soundPlayerIIDCount = 2;
        const SLInterfaceID soundPlayerIIDs[] = { SL_IID_PLAY, SL_IID_BUFFERQUEUE };
        const SLboolean soundPlayerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

        for (int i = 0; i < AUDIO_MAX_VOICES; i++)
        {
            // Create
            result = (*s_slEngine)->CreateAudioPlayer(s_slEngine,
                &s_arPlayerObjs[i],
                &dataSource,
                &dataSink,
                soundPlayerIIDCount,
                soundPlayerIIDs,
                soundPlayerReqs);

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating audio player.");
                return;
            }

            // Realize
            result = (*(s_arPlayerObjs[i]))->Realize(s_arPlayerObjs[i], SL_BOOLEAN_FALSE);

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error realizing audio player.");
                return;
            }

            // Player Interface
            result = (*(s_arPlayerObjs[i]))->GetInterface(s_arPlayerObjs[i],
                SL_IID_PLAY,
                &(s_arPlayers[i]));

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating player interface.");
                return;
            }

            // Buffer Queue Interface
            result = (*(s_arPlayerObjs[i]))->GetInterface(s_arPlayerObjs[i],
                SL_IID_BUFFERQUEUE,
                &(s_arPlayerQueues[i]));

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error creating buffer queue interface.");
                return;
            }

            result = (*(s_arPlayers[i]))->SetPlayState(s_arPlayers[i], SL_PLAYSTATE_PLAYING);

            if (result != SL_RESULT_SUCCESS)
            {
                LogError("Error setting audio player to play state.");
                return;
            }
        }
    }
}

void AUD_Shutdown()
{
    if (s_nInitialized != 0)
    {
        LogDebug("Shutting down Sound class.");

        // Destroy the audio players first
        for (int i = 0; i < AUDIO_MAX_VOICES; i++)
        {
            if (s_arPlayerObjs[i] != 0)
            {
                (*(s_arPlayerObjs[i]))->Destroy(s_arPlayerObjs[i]);
                s_arPlayerObjs[i] = 0;
                s_arPlayers[i] = 0;
                s_arPlayerQueues[i] = 0;
            }
        }

        if (s_slOutputMixObj != 0)
        {
            (*s_slOutputMixObj)->Destroy(s_slOutputMixObj);
            s_slOutputMixObj = 0;
        }

        if (s_slEngineObj != 0)
        {
            (*s_slEngineObj)->Destroy(s_slEngineObj);
            s_slEngineObj = 0;
            s_slEngine = 0;
        }

        s_nInitialized = 0;
    }
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
    SLresult result;
    SLuint32 playerState;

    if (soundWave->GetBitsPerSample() != 16 ||
        soundWave->GetSampleRate() != 44100 ||
        soundWave->GetNumChannels() != 2)
    {
        LogDebug("Skipping sound %s", soundWave->GetName().c_str());
    }

    LogDebug("Play Sound %s", soundWave->GetName().c_str());

    (*(s_arPlayerObjs[voiceIndex]))->GetState(s_arPlayerObjs[voiceIndex], &playerState);

    if (playerState == SL_OBJECT_STATE_REALIZED)
    {
        // Clear any sound that was there before
        result = (*(s_arPlayerQueues[voiceIndex]))->Clear(s_arPlayerQueues[voiceIndex]);

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Error clearing player queue.");
            return;
        }

        // Add the new sound buffer to queue
        result = (*(s_arPlayerQueues[voiceIndex]))->Enqueue(s_arPlayerQueues[voiceIndex],
            soundWave->GetWaveData(),
            soundWave->GetWaveDataSize());

        if (result != SL_RESULT_SUCCESS)
        {
            LogError("Could not enqueue sound buffer to queue.");
            return;
        }
    }
}

void AUD_Stop(uint32_t voiceIndex)
{
    SLresult result = (*(s_arPlayerQueues[voiceIndex]))->Clear(s_arPlayerQueues[voiceIndex]);

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
    SLresult result = (*(s_arPlayerQueues[voiceIndex]))->GetState(s_arPlayerQueues[voiceIndex], &queueState);

    if (result != SL_RESULT_SUCCESS)
    {
        LogError("Error retrieving audio play state.");
    }
    else
    {
        LogDebug("QueueState: index %d, count %d", queueState.playIndex, queueState.count);
        playing = (queueState.count > 0);
    }

    return playing;
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
