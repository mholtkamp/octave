#include "Assets/SoundWave.h"

#include "Log.h"
#include "Stream.h"
#include "Property.h"
#include "AudioManager.h"

#include "Audio/Audio.h"
#include "System/System.h"

FORCE_LINK_DEF(SoundWave);
DEFINE_ASSET(SoundWave);

// This is causing very loud sound corruption on 3DS, need to
// debug this if we are going to turn it back on.
#define LQ_CONSOLE_AUDIO 0

bool SoundWave::HandlePreviewPropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    SoundWave* soundWave = (SoundWave*)prop->mOwner;

    if (prop->mName == "Play")
    {
        AudioManager::PlaySound2D(soundWave);
    }
    else if (prop->mName == "Stop")
    {
        AudioManager::StopSounds(soundWave);
    }

    return true;
}

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
    mAudioClass = stream.ReadInt8();
    mCompress = stream.ReadBool();
    mCompressInternal = stream.ReadBool();

    // Waveform Format
    mNumChannels = stream.ReadUint32();
    mBitsPerSample = stream.ReadUint32();
    mSampleRate = stream.ReadUint32();
    mNumSamples = stream.ReadUint32();
    mBlockAlign = stream.ReadUint32();
    mByteRate = stream.ReadUint32();

    bool compressed = stream.ReadBool();

    if (compressed)
    {
        uint32_t compressedSize = stream.ReadUint32();
        OCT_UNUSED(compressedSize); // Unused in non-editor

#if EDITOR
        // In Editor, we want to keep the compressed data around so in case we save the file again,
        // we won't be recompressing the sound a second time (adding more artifacts / distortion).
        mCompressedData = new uint8_t[compressedSize];
        mCompressedSize = compressedSize;
        memcpy(mCompressedData, stream.GetData() + stream.GetPos(), compressedSize);
#endif

        Stream outStream;
        PcmFormat format;
        format.mBytesPerSample = (mBitsPerSample / 8);
        format.mNumChannels = mNumChannels;
        format.mSampleRate = mSampleRate;
        AUD_DecodeVorbis(stream, outStream, format);

        mWaveDataSize = outStream.GetSize();
        mWaveData = AUD_AllocWaveBuffer(mWaveDataSize);
        memcpy(mWaveData, outStream.GetData(), mWaveDataSize);
    }
    else
    {
        // Waveform
        mWaveDataSize = stream.ReadUint32();
        mWaveData = AUD_AllocWaveBuffer(mWaveDataSize);
        for (uint32_t i = 0; i < mWaveDataSize; ++i)
        {
            mWaveData[i] = stream.ReadUint8();
        }
    }

    AUD_ProcessWaveBuffer(this);
}

void SoundWave::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    // Properties
    stream.WriteFloat(mVolumeMultiplier);
    stream.WriteFloat(mPitchMultiplier);
    stream.WriteInt8(mAudioClass);
    stream.WriteBool(mCompress);
    stream.WriteBool(mCompressInternal);

    uint32_t numChannels = mNumChannels;
    uint32_t bitsPerSample = mBitsPerSample;
    uint32_t sampleRate = mSampleRate;
    uint32_t numSamples = mNumSamples;
    uint32_t blockAlign = mBlockAlign;
    uint32_t byteRate = mByteRate;

    uint8_t* waveData = mWaveData;
    uint32_t waveDataSize = mWaveDataSize;

    uint8_t* lqWaveData = nullptr;
    bool lqConvert = false;

#if LQ_CONSOLE_AUDIO
    if (platform == Platform::GameCube ||
        platform == Platform::Wii ||
        platform == Platform::N3DS)
    {
        // Maybe only temporary, these platforms will use low-quality audio.
        // In the future allow compressing to OGG
        lqConvert = true;

        lqWaveData = new uint8_t[mNumSamples];
        memset(lqWaveData, 0, mNumSamples);

        const uint8_t* srcData = mWaveData;
        uint8_t* dstData = lqWaveData;

        uint32_t numSamples = mNumSamples;
        uint32_t lqWaveDataSize = mWaveDataSize;

        // First convert all samples to 8 bit
        for (uint32_t i = 0; i < numSamples; ++i)
        {
            if (mBitsPerSample == 8)
            {
                *dstData = *srcData;

                dstData++;
                srcData++;
            }
            else
            {
                int16_t sample16;
                memcpy(&sample16, srcData, sizeof(int16_t));
                uint8_t sample8 = (uint8_t)((sample16 + 32768) >> 8);
                *dstData = sample8;

                dstData++;
                srcData += 2;
            }
        }

        if (mBitsPerSample == 16)
        {
            lqWaveDataSize /= 2;
        }

        // Then convert to mono by averaging left + right channels
        if (mNumChannels == 2)
        {
            for (uint32_t i = 0; i < numSamples / 2; ++i)
            {
                uint8_t sample1 = lqWaveData[i * 2 + 0];
                uint8_t sample2 = lqWaveData[i * 2 + 1];

                float sampleAvg = (float(sample1) + float(sample2)) / 2.0f;
                lqWaveData[i] = uint8_t(sampleAvg + 0.5);
            }

            numSamples /= 2;
            lqWaveDataSize /= 2;
        }

        // Lastly merge samples to make 22050 Hz (if original is 44100 Hz).
        uint32_t lowSampleRate = mSampleRate;
        if (mSampleRate == 44100)
        {
            lowSampleRate = 22050;

            for (uint32_t i = 0; i < numSamples / 2; ++i)
            {
                uint8_t sample1 = lqWaveData[i * 2 + 0];
                uint8_t sample2 = lqWaveData[i * 2 + 1];

                float sampleAvg = (float(sample1) + float(sample2)) / 2.0f;
                lqWaveData[i] = uint8_t(sampleAvg + 0.5);
            }

            numSamples /= 2;
            lqWaveDataSize /= 2;
        }

        // Then write the data to stream (using mono, 8-bit, 22050 Hz settings)
        numChannels = 1;
        bitsPerSample = 8;
        sampleRate = lowSampleRate;
        numSamples = numSamples;
        blockAlign = 1;
        byteRate = lowSampleRate;

        waveData = lqWaveData;
        waveDataSize = lqWaveDataSize;
    }
#endif

    // Waveform Format
    stream.WriteUint32(numChannels);
    stream.WriteUint32(bitsPerSample);
    stream.WriteUint32(sampleRate);
    stream.WriteUint32(numSamples);
    stream.WriteUint32(blockAlign);
    stream.WriteUint32(byteRate);

    bool compress = mCompress;
    if (platform == Platform::Count)
    {
        // In editor, we only compress the data (destructive) if the compress internal flag is set.
        // This feature is to reduce file sizes for big sounds like music. But once you compress internally,
        // storing the data uncompressed later will not result in any better audio. Use only when needed.
        compress = (mCompress && mCompressInternal);
    }

    stream.WriteBool(compress);

    if (compress)
    {
        if (mCompressedData != nullptr && !lqConvert)
        {
            // Writeout the already-computed compressed OGG data
            stream.WriteUint32(mCompressedSize);
            stream.WriteBytes(mCompressedData, mCompressedSize);
        }
        else
        {
            // Output to a separate stream so we can determine the total size of the compressed data.
            Stream outStream;
            Stream inStream((char*)waveData, waveDataSize);

            PcmFormat format;
            format.mBytesPerSample = (bitsPerSample / 8);
            format.mNumChannels = numChannels;
            format.mSampleRate = sampleRate;

            AUD_EncodeVorbis(inStream, outStream, format);

            uint32_t compressedSize = outStream.GetSize();
            stream.WriteUint32(compressedSize);
            stream.WriteBytes((uint8_t*)outStream.GetData(), outStream.GetSize());
        }
    }
    else
    {
        // Waveform
        stream.WriteUint32(waveDataSize);
        for (uint32_t i = 0; i < waveDataSize; ++i)
        {
            stream.WriteUint8(waveData[i]);
        }
    }

    if (lqWaveData != nullptr)
    {
        delete lqWaveData;
        lqWaveData = nullptr;
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

    if (mCompressedData != nullptr)
    {
#if !EDITOR
        // We should only have compressed data in EDITOR.
        OCT_ASSERT(0);
#endif
        delete mCompressedData;
    }
}


void SoundWave::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);

#if EDITOR
    Stream wavStream;
    wavStream.ReadFile(path.c_str(), false);
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

    static bool sFakePlay = false;
    outProps.push_back(Property(DatumType::Bool, "Play", this, &sFakePlay, 1, HandlePreviewPropChange));

    static bool sFakeStop = false;
    outProps.push_back(Property(DatumType::Bool, "Stop", this, &sFakeStop, 1, HandlePreviewPropChange));

    outProps.push_back(Property(DatumType::Float, "Volume Multiplier", this, &mVolumeMultiplier));
    outProps.push_back(Property(DatumType::Float, "Pitch Multiplier", this, &mPitchMultiplier));
    outProps.push_back(Property(DatumType::Byte, "Audio Class", this, &mAudioClass));
    outProps.push_back(Property(DatumType::Bool, "Compress", this, &mCompress));
    outProps.push_back(Property(DatumType::Bool, "Compress Internal", this, &mCompressInternal));
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

void SoundWave::SetAudioClass(int8_t audioClass)
{
    mAudioClass = audioClass;
}

int8_t SoundWave::GetAudioClass() const
{
    return mAudioClass;
}

void SoundWave::SetVolumeMultiplier(float volume)
{
    mVolumeMultiplier = volume;
}

void SoundWave::SetPcmData(uint8_t* data, uint32_t size, uint32_t numSamples, uint32_t bitsPerSample, uint32_t numChannels, uint32_t sampleRate)
{
    // Caller should make sure data is allocated with AUD_AllocWaveBuffer and freeing the previous buffer.
    mWaveData = data;
    mWaveDataSize = size;
    mNumSamples = numSamples;
    mBitsPerSample = bitsPerSample;
    mNumChannels = numChannels;
    mSampleRate = sampleRate;
    mBlockAlign = (numChannels * bitsPerSample) / 8;
    mByteRate = (sampleRate * numChannels * bitsPerSample) / 8;
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
