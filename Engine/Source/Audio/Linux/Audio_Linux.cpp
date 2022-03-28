#if PLATFORM_LINUX

#include "Audio/Audio.h"
#include "Audio/AudioConstants.h"
#include "System/System.h"

#include "Assets/SoundWave.h"
#include "Log.h"

#include <alsa/asoundlib.h>
#include <glm/glm.hpp>

snd_pcm_t* sSoundDevice = nullptr;
snd_pcm_uframes_t sPlaybackFrames = 0;
uint32_t sMixBufferLen = 0;
int16_t* sMixBuffer = nullptr;

struct SoundVoice
{
    int32_t mSampleRate = 44100;
    float mPitch = 1.0f;
    float mVolumeL = 1.0f;
    float mVolumeR = 1.0f;
    uint8_t* mSrcBuffer = nullptr;
    uint32_t mSrcBufferLen = 0;
    uint32_t mSrcFrames = 0;
    float mCurFrame = 0;
    uint32_t mNumChannels = 2;
    uint32_t mBytesPerSample = 2;
    bool mLoop = false;
    bool mActive = false;
};

static SoundVoice sVoices[AUDIO_MAX_VOICES];

void AUD_Initialize()
{
    int err = snd_pcm_open( &sSoundDevice, "default", SND_PCM_STREAM_PLAYBACK, 0 );
    snd_pcm_hw_params_t* hw_params = nullptr;

    if( err < 0 )
    {
        LogError("Cannot open audio device");
        assert(0);
        return;
    }
    else
    {
        LogDebug("Audio device opened.");
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0)
    {
        LogError("Failed to allocate hardware params");
        assert(0);
        return;
    }

    if ((err = snd_pcm_hw_params_any(sSoundDevice, hw_params)) < 0)
    {
        LogError("Failed to initialize hardware params");
        assert(0);
        return;
    }

    // Create enough buffer room for a 30 fps update rate.
    sPlaybackFrames = snd_pcm_uframes_t((1 / 15.0f) * 44100);

    err = snd_pcm_hw_params_set_rate_resample(sSoundDevice, hw_params, 1);
    err = snd_pcm_hw_params_set_access(sSoundDevice, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    err = snd_pcm_hw_params_set_format(sSoundDevice, hw_params, SND_PCM_FORMAT_S16_LE);
    err = snd_pcm_hw_params_set_channels(sSoundDevice, hw_params, 2);
    err = snd_pcm_hw_params_set_buffer_size(sSoundDevice, hw_params, sPlaybackFrames);

    unsigned int playbackRate = 44100;
    err = snd_pcm_hw_params_set_rate_near(sSoundDevice, hw_params, &playbackRate, 0);

    err = snd_pcm_hw_params(sSoundDevice, hw_params);

    snd_pcm_uframes_t bufferSize;
    snd_pcm_hw_params_get_buffer_size( hw_params, &bufferSize );
    LogDebug("Buffer size = %d frames", (int32_t) bufferSize);
    sPlaybackFrames = bufferSize;
    LogDebug("Significant bits for linear samples = %d",snd_pcm_hw_params_get_sbits(hw_params));

    snd_pcm_uframes_t periodFrames = 0;
	snd_pcm_hw_params_get_period_size(hw_params, &periodFrames, 0);
	LogDebug("Period Frames: %lu\n", periodFrames);

    snd_pcm_hw_params_free(hw_params);
    err = snd_pcm_prepare(sSoundDevice);

    sMixBufferLen = sPlaybackFrames * 4; // ( 2 samples (L/R) * 2 bytes per sample)
    sMixBuffer = new int16_t[sMixBufferLen / 2];
    memset(sMixBuffer, 0, sMixBufferLen);
    snd_pcm_writei(sSoundDevice, sMixBuffer, sPlaybackFrames);
    //snd_pcm_start(sSoundDevice);

    LogDebug("PCM name: '%s'", snd_pcm_name(sSoundDevice));
    LogDebug("PCM state: %s", snd_pcm_state_name(snd_pcm_state(sSoundDevice)));
}

void AUD_Shutdown()
{
    delete [] sMixBuffer;
    sMixBuffer = nullptr;

    if (sSoundDevice != nullptr)
    {
        snd_pcm_close(sSoundDevice);
    }
}

void AUD_Update()
{   
    int32_t frames = (int32_t) snd_pcm_avail(sSoundDevice);
    frames = glm::min(int32_t(sMixBufferLen) / 4, frames);

    //LogDebug("Frames to mix: %d", frames);
    //LogDebug("PCM state: %s", snd_pcm_state_name(snd_pcm_state(sSoundDevice)));

    if (frames > 0)
    {
        // We need to mix an audio buffer
        //assert(frames * 4  <= int32_t(sMixBufferLen));
        memset(sMixBuffer, 0, frames * 4);

        for (uint32_t i = 0; i < AUDIO_MAX_VOICES; ++i)
        {
            if (sVoices[i].mActive)
            {
                SoundVoice& voice = sVoices[i];
                assert(voice.mSrcFrames > 0);

                // If the voice is active, that means we need to mix *frames* number of frames
                // into the mix buffer. The src voice may move at a faster or slower pace based on the 
                // pitch value, so we will need to interpolate between frames.

                // TODO: Handle pitch
                float srcDeltaFrame = 1 * voice.mPitch * (voice.mSampleRate / 44100.0f);

                for (int32_t dstFrame = 0; dstFrame < frames; ++dstFrame)
                {
                    float srcFrameFloat = voice.mCurFrame + (dstFrame * srcDeltaFrame);

                    int32_t srcFrames[2] = { int32_t(srcFrameFloat), int32_t(srcFrameFloat) + 1 };
                    float frameInterpAlpha = fmod(srcFrameFloat, 1.0f);

                    int16_t srcSampleL[2] = { 0, 0 };
                    int16_t srcSampleR[2] = { 0, 0 };

                    if (voice.mLoop)
                    {
                        if (srcFrames[0] >= int32_t(voice.mSrcFrames))
                            srcFrames[0] = srcFrames[0] % voice.mSrcFrames;
                        if (srcFrames[1] >= int32_t(voice.mSrcFrames))
                            srcFrames[1] = srcFrames[1] % voice.mSrcFrames;
                    }

                    // Interpolate between the two src frames
                    for (int32_t f = 0; f < 2; ++f)
                    {
                        int32_t frameIndex = srcFrames[f];

                        if (frameIndex >= int32_t(voice.mSrcFrames))
                        {
                            srcSampleL[f] = 0;
                            srcSampleR[f] = 0;
                        }
                        else if (voice.mNumChannels == 1)
                        {
                            // Use same src sample for left and right dst samples

                            if (voice.mBytesPerSample == 1)
                            {
                                // uint8 samples
                                // Convert from uint8_t to int16_t
                                srcSampleL[f] = *((uint8_t*) (voice.mSrcBuffer + (frameIndex * 1 * 1)));
                                srcSampleL[f] = srcSampleL[f] * 256 - 32767;
                                srcSampleR[f] = srcSampleL[f];   
                            }
                            else
                            {
                                // int16 samples
                                srcSampleL[f] = *((int16_t*) (voice.mSrcBuffer + (frameIndex * 1 * 2)));
                                srcSampleR[f] = srcSampleL[f];
                            }
                        }
                        else
                        {
                            if (voice.mBytesPerSample == 1)
                            {
                                // uint8 samples
                                srcSampleL[f] = *((uint8_t*) (voice.mSrcBuffer + (frameIndex * 2 * 1)));
                                srcSampleR[f] = *((uint8_t*) (voice.mSrcBuffer + (frameIndex * 2 * 1 + 1)));
                                srcSampleL[f] = srcSampleL[f] * 256 - 32767;
                                srcSampleR[f] = srcSampleR[f] * 256 - 32767;
                            }
                            else
                            {
                                // int16 samples
                                srcSampleL[f] = *((int16_t*) (voice.mSrcBuffer + (frameIndex * 2 * 2)));
                                srcSampleR[f] = *((int16_t*) (voice.mSrcBuffer + (frameIndex * 2 * 2 + 2)));
                            }
                        }
                    }

                    // We have four samples now, so now we need to linearly interpolate between the left and right channels
                    // to get a final 2 samples that we will accumulate into the mix buffer.
                    int16_t finalSampleL = glm::mix(srcSampleL[0], srcSampleL[1], frameInterpAlpha);
                    int16_t finalSampleR = glm::mix(srcSampleR[0], srcSampleR[1], frameInterpAlpha);
                    finalSampleL *= voice.mVolumeL;
                    finalSampleR *= voice.mVolumeR;
                    sMixBuffer[dstFrame * 2 + 0] = glm::clamp(finalSampleL + sMixBuffer[dstFrame * 2 + 0], -32768, 32767);
                    sMixBuffer[dstFrame * 2 + 1] = glm::clamp(finalSampleR + sMixBuffer[dstFrame * 2 + 1], -32768, 32767);
                }

                // We've finished getting all of the samples for this voice. Increase the current sample value
                // to keep track of where to pick up next frame. If the sound is looping we need to mod the frame value
                // otherwise let the curFrame exceed the *voice.mSrcFrames* count so we can determine that it is finished playing.

                voice.mCurFrame += (frames * srcDeltaFrame);

                if (voice.mLoop)
                {
                    voice.mCurFrame = fmod(voice.mCurFrame, (float) voice.mSrcFrames);
                }
            }
        }
    }

    snd_pcm_sframes_t framesWritten = 0;
    // We've generated the frames we need for this update, so now we just need to sent it to the audio driver.
    if (frames < 0 ||
        (frames > 0 && (framesWritten = snd_pcm_writei(sSoundDevice, sMixBuffer, frames)) == -EPIPE))
    {
        //LogWarning("Audio buffer underrun.");
        snd_pcm_prepare(sSoundDevice);
    } 
    else if (framesWritten < 0)
    {
        LogError("Can't write to PCM device. %s", snd_strerror(framesWritten));
        return;
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
    assert(!sVoices[voiceIndex].mActive);

    sVoices[voiceIndex].mActive = true;
    sVoices[voiceIndex].mBytesPerSample = soundWave->GetBitsPerSample() / 8;
    sVoices[voiceIndex].mCurFrame = 0.0f;
    sVoices[voiceIndex].mLoop = loop;
    sVoices[voiceIndex].mNumChannels = soundWave->GetNumChannels();
    sVoices[voiceIndex].mPitch = pitch;
    sVoices[voiceIndex].mSampleRate = soundWave->GetSampleRate();
    sVoices[voiceIndex].mSrcBuffer = soundWave->GetWaveData();
    sVoices[voiceIndex].mSrcBufferLen = soundWave->GetWaveDataSize();
    sVoices[voiceIndex].mVolumeL = spatial ? 0.0f : volume;
    sVoices[voiceIndex].mVolumeR = spatial ? 0.0f : volume;
    
    int32_t bytesPerFrame = sVoices[voiceIndex].mBytesPerSample * sVoices[voiceIndex].mNumChannels;
    sVoices[voiceIndex].mSrcFrames = sVoices[voiceIndex].mSrcBufferLen / bytesPerFrame;

    assert(sVoices[voiceIndex].mSrcBufferLen % bytesPerFrame == 0);
    assert(bytesPerFrame > 0 &&
           bytesPerFrame <= 4);
}

void AUD_Stop(uint32_t voiceIndex)
{
    sVoices[voiceIndex].mActive = false;
}

bool AUD_IsPlaying(uint32_t voiceIndex)
{
    return sVoices[voiceIndex].mActive &&
           sVoices[voiceIndex].mCurFrame < sVoices[voiceIndex].mSrcFrames;
}

void AUD_SetVolume(uint32_t voiceIndex, float leftVolume, float rightVolume)
{
    sVoices[voiceIndex].mVolumeL = leftVolume;
    sVoices[voiceIndex].mVolumeR = rightVolume;
}

void AUD_SetPitch(uint32_t voiceIndex, float pitch)
{
    sVoices[voiceIndex].mPitch = pitch;
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
