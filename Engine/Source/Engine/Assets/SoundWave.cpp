#include "Assets/SoundWave.h"

#include "Log.h"
#include "Stream.h"
#include "Property.h"
#include "AudioManager.h"

#include "Audio/Audio.h"
#include "System/System.h"

FORCE_LINK_DEF(SoundWave);
DEFINE_ASSET(SoundWave);

SoundWave::SoundWave()
{
    mType = SoundWave::GetStaticType();
}

SoundWave::~SoundWave()
{

}

void SoundWave::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    // Properties
    mVolumeMultiplier = stream.ReadFloat();
    mPitchMultiplier = stream.ReadFloat();

    // Waveform Format
    mNumChannels = stream.ReadUint32();
    mBitsPerSample = stream.ReadUint32();
    mSampleRate = stream.ReadUint32();
    mNumSamples = stream.ReadUint32();
    mBlockAlign = stream.ReadUint32();
    mByteRate = stream.ReadUint32();

    // Waveform
    mWaveDataSize = stream.ReadUint32();
    mWaveData = AUD_AllocWaveBuffer(mWaveDataSize);
    for (uint32_t i = 0; i < mWaveDataSize; ++i)
    {
        mWaveData[i] = stream.ReadUint8();
    }

    AUD_ProcessWaveBuffer(this);
}

void SoundWave::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    // Properties
    stream.WriteFloat(mVolumeMultiplier);
    stream.WriteFloat(mPitchMultiplier);

    // Waveform Format
    stream.WriteUint32(mNumChannels);
    stream.WriteUint32(mBitsPerSample);
    stream.WriteUint32(mSampleRate);
    stream.WriteUint32(mNumSamples);
    stream.WriteUint32(mBlockAlign);
    stream.WriteUint32(mByteRate);

    // Waveform
    stream.WriteUint32(mWaveDataSize);
    for (uint32_t i = 0; i < mWaveDataSize; ++i)
    {
        stream.WriteUint8(mWaveData[i]);
    }
}

void SoundWave::Create()
{
    Asset::Create();
}

void SoundWave::Destroy()
{
    Asset::Destroy();

    if (mWaveData != nullptr)
    {
        AudioManager::StopSounds(this);
        AUD_FreeWaveBuffer(mWaveData);
        mWaveData = nullptr;
    }
}


void SoundWave::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);

#if EDITOR
    Stream wavStream;
    wavStream.ReadFile(path.c_str());
    uint8_t* wavData = (uint8_t*) wavStream.GetData();

    char fileFormat[5] = {};
    memcpy(fileFormat, wavData + 8, 4);
    OCT_ASSERT(strncmp(fileFormat, "WAVE", 4) == 0);

    uint16_t audioFormat = *((uint16_t*)(wavData + 20));
    OCT_ASSERT(audioFormat == 1);

    uint16_t numChannels = *((uint16_t*)(wavData + 22));
    uint32_t sampleRate = *((uint32_t*)(wavData + 24));
    uint32_t byteRate = *((uint32_t*)(wavData + 28));
    uint16_t blockAlign = *((uint16_t*)(wavData + 32));
    uint16_t bitsPerSample = *((uint16_t*)(wavData + 34));

    uint32_t wavSampleDataSize = *((uint32_t*)(wavData + 40));
    uint8_t* wavSampleData = (wavData + 44);

    uint32_t bytesPerSample = bitsPerSample / 8;
    uint32_t numSamples = wavSampleDataSize / bytesPerSample;

    bool validBitDepth = (bitsPerSample == 8 || bitsPerSample == 16);
    bool validNumChannels = (numChannels == 1 || numChannels == 2);

    if (validBitDepth && validNumChannels)
    {
        mWaveDataSize = numSamples * bytesPerSample;
        mWaveData = (uint8_t*)SYS_AlignedMalloc(mWaveDataSize, 32);
        OCT_ASSERT(mWaveDataSize == wavSampleDataSize);
        memcpy(mWaveData, wavSampleData, wavSampleDataSize);
    }
    else
    {
        LogError("Unsupported WAV Format");
        LogError("BitDepth = %dh (expected 8 or 16)", bitsPerSample);
        LogError("NumChannels = %dh (expected 1 or 2)", numChannels);
    }

    mNumChannels = (uint32_t)numChannels;
    mBitsPerSample = (uint32_t)bitsPerSample;
    mSampleRate = sampleRate;
    mNumSamples = numSamples;
    mBlockAlign = blockAlign;
    mByteRate = byteRate;

    AUD_ProcessWaveBuffer(this);
#endif
}

void SoundWave::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Float, "Volume Multiplier", this, &mVolumeMultiplier));
    outProps.push_back(Property(DatumType::Float, "Pitch Multiplier", this, &mPitchMultiplier));
}

glm::vec4 SoundWave::GetTypeColor()
{
    return glm::vec4(0.5f, 0.1f, 1.0f, 1.0f);
}

const char* SoundWave::GetTypeName()
{
    return "SoundWave";
}

const char* SoundWave::GetTypeImportExt()
{
    return ".wav";
}

void SoundWave::SetPitchMultiplier(float pitch)
{
    mPitchMultiplier = pitch;
}

float SoundWave::GetPitchMultiplier() const
{
    return mPitchMultiplier;
}

void SoundWave::SetVolumeMultiplier(float volume)
{
    mVolumeMultiplier = volume;
}

float SoundWave::GetVolumeMultiplier() const
{
    return mVolumeMultiplier;
}

uint8_t* SoundWave::GetWaveData() const
{
    return mWaveData;
}

uint32_t SoundWave::GetWaveDataSize() const
{
    return mWaveDataSize;
}

uint32_t SoundWave::GetNumChannels() const
{
    return mNumChannels;
}

uint32_t SoundWave::GetBitsPerSample() const
{
    return mBitsPerSample;
}

uint32_t SoundWave::GetSampleRate() const
{
    return mSampleRate;
}

uint32_t SoundWave::GetNumSamples() const
{
    return mNumSamples;
}

float SoundWave::GetDuration() const
{
    return float(mNumSamples) / mSampleRate;
}

uint32_t SoundWave::GetBlockAlign() const
{
    return mBlockAlign;
}

uint32_t SoundWave::GetByteRate() const
{
    return mByteRate;
}
