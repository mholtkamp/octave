#pragma once

#include "Components/TransformComponent.h"

#include "Assets/SoundWave.h"

class AudioComponent : public TransformComponent
{
public:

    DECLARE_NODE(AudioComponent, TransformComponent);

    AudioComponent();
    ~AudioComponent();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void BeginPlay() override;
    virtual void Tick(float deltaTime) override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    void SetSoundWave(SoundWave* soundWave);
    SoundWave* GetSoundWave();

    void SetInnerRadius(float innerRadius);
    float GetInnerRadius() const;

    void SetOuterRadius(float outerRadius);
    float GetOuterRadius() const;

    void SetVolume(float volume);
    float GetVolume() const;

    void SetPitch(float pitch);
    float GetPitch() const;

    void SetStartOffset(float startOffset);
    float GetStartOffset() const;

    void SetPriority(int32_t priority);
    int32_t GetPriority() const;

    void SetAttenuationFunc(AttenuationFunc func);
    AttenuationFunc GetAttenuationFunc() const;

    void SetAudioClass(int8_t audioClass);
    int8_t GetAudioClass() const;

    void SetLoop(bool loop);
    bool GetLoop() const;

    void SetAutoPlay(bool autoPlay);
    bool GetAutoPlay() const;

    float GetPlayTime() const;
    bool IsPlaying() const;
    bool IsAudible() const;

    void Play();
    void Pause();
    void Stop();
    void Reset();

    void NotifyAudible(bool audible);

protected:

    // Properties
    SoundWaveRef mSoundWave;
    float mInnerRadius = 0.0f;
    float mOuterRadius = 15.0f;
    float mVolume = 1.0f;
    float mPitch = 1.0f;
    float mStartOffset = 0.0f;
    int32_t mPriority = 0;
    AttenuationFunc mAttenuationFunc = AttenuationFunc::Linear;
    int8_t mAudioClass = -1;
    bool mLoop = false;
    bool mAutoPlay = false;

    // State
    float mPlayTime = 0.0f;
    bool mPlaying = false;
    bool mAudible = false;
};