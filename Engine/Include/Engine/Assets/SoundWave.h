#pragma once

#include "Asset.h"

class SoundWave : public Asset
{
public:

    DECLARE_ASSET(SoundWave, Asset);

    SoundWave();
    ~SoundWave();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    void SetVolumeMultiplier(float volume);
    float GetVolumeMultiplier() const;

    void SetPitchMultiplier(float pitch);
    float GetPitchMultiplier() const;

    void SetAudioClass(int8_t audioClass);
    int8_t GetAudioClass() const;

    uint8_t* GetWaveData() const;
    uint32_t GetWaveDataSize() const;
    uint32_t GetNumChannels() const;
    uint32_t GetBitsPerSample() const;
    uint32_t GetSampleRate() const;
    uint32_t GetNumSamples() const;
    uint32_t GetBlockAlign() const;
    uint32_t GetByteRate() const;

    float GetDuration() const;

protected:

    static bool HandlePreviewPropChange(Datum* datum, uint32_t index, const void* newValue);

    uint8_t* mWaveData = nullptr;
    uint32_t mWaveDataSize = 0;

    // Properties
    float mVolumeMultiplier = 1.0f;
    float mPitchMultiplier = 1.0f;
    int8_t mAudioClass = 0;

    // Soundwave Format
    uint32_t mNumChannels = 1;
    uint32_t mBitsPerSample = 8;
    uint32_t mSampleRate = 22050;
    uint32_t mNumSamples = 0;
    uint32_t mBlockAlign = 0;
    uint32_t mByteRate = 0;
};
