#pragma once

#include "Timeline/TimelineClip.h"
#include "AssetRef.h"

enum class AudioClipEndMode
{
    Stop,
    Continue,
    Count
};

class AudioClip : public TimelineClip
{
public:

    DECLARE_CLIP(AudioClip, TimelineClip);

    AudioClip();
    virtual ~AudioClip();

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    SoundWaveRef GetSoundWave() const { return mSoundWave; }
    void SetSoundWave(SoundWaveRef soundWave) { mSoundWave = soundWave; }

    float GetVolume() const { return mVolume; }
    float GetPitch() const { return mPitch; }
    float GetFadeInDuration() const { return mFadeInDuration; }
    float GetFadeOutDuration() const { return mFadeOutDuration; }
    AudioClipEndMode GetEndMode() const { return mEndMode; }
    bool GetLoop() const { return mLoop; }

protected:

    SoundWaveRef mSoundWave;
    float mVolume = 1.0f;
    float mPitch = 1.0f;
    float mFadeInDuration = 0.0f;
    float mFadeOutDuration = 0.0f;
    AudioClipEndMode mEndMode = AudioClipEndMode::Stop;
    bool mLoop = false;
};
