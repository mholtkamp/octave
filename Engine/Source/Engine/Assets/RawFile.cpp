#include "Assets/RawFile.h"
#include "Stream.h"

FORCE_LINK_DEF(RawFile);
DEFINE_ASSET(RawFile);

bool RawFile::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    RawFile* rawFile = static_cast<RawFile*>(prop->mOwner);
    bool success = false;

    return success;
}

RawFile::RawFile()
{
    mType = RawFile::GetStaticType();
}

RawFile::~RawFile()
{

}

void RawFile::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    uint32_t size = stream.ReadUint32();
    mData.resize(size);
    stream.ReadBytes(mData.data(), size);
}

void RawFile::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteUint32((uint32_t)mData.size());
    stream.WriteBytes(mData.data(), (uint32_t)mData.size());
}

void RawFile::Create()
{
    Asset::Create();
}

void RawFile::Destroy()
{
    Asset::Destroy();
}

void RawFile::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);

    Stream stream;
    stream.ReadFile(path.c_str(), false);

    mData.resize(stream.GetSize());

    if (stream.GetSize() > 0)
    {
        memcpy(mData.data(), stream.GetData(), stream.GetSize());
    }
}

void RawFile::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

}

glm::vec4 RawFile::GetTypeColor()
{
    return glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
}

const char* RawFile::GetTypeName()
{
    return "RawFile";
}

const char* RawFile::GetTypeImportExt()
{
    // No specific extension. Any file can be imported as a raw file!
    return "";
}

const std::vector<uint8_t>& RawFile::GetData() const
{
    return mData;
}

uint32_t RawFile::GetDataSize() const
{
    return uint32_t(mData.size());
}

const uint8_t* RawFile::GetDataPointer()
{
    return mData.data();
}

Stream RawFile::GetStream() const
{
    Stream retStream((char*)mData.data(), (uint32_t)mData.size());
    return retStream;
}
