#include "Stream.h"
#include "Asset.h"
#include "AssetRef.h"
#include "AssetManager.h"
#include "Log.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <glm/gtc/type_ptr.hpp>

#define MAX_FILE_SIZE (1024 * 1024 * 1024)
#define MAX_STRING_SIZE (1024 * 16)

Stream::Stream() :
    mData(nullptr),
    mSize(0),
    mCapacity(0),
    mPos(0),
    mAsyncRequest(nullptr),
    mExternal(false)
{

}

// TODO: Avoid const cast?
Stream::Stream(const char* externalData, uint32_t externalSize) :
    mData(const_cast<char*>(externalData)),
    mSize(externalSize),
    mCapacity(externalSize),
    mPos(0),
    mAsyncRequest(nullptr),
    mExternal(true)
{

}

Stream::~Stream()
{
    if (!mExternal && mData != nullptr)
    {
        free(mData);
        mData = nullptr;
    }
}

char* Stream::GetData()
{
    return mData;
}

uint32_t Stream::GetSize() const
{
    return mSize;
}

uint32_t Stream::GetPos()
{
    return mPos;
}

void Stream::SetPos(uint32_t pos)
{
    mPos = pos;

    if (pos > mSize)
    {
        mPos = mSize;
    }
}

void Stream::ReadFile(const char* path, int32_t maxSize)
{
    OCT_ASSERT(!mExternal);
    if (mExternal)
    {
        LogError("Cannot ReadFile() using an external Stream");
        return;
    }

    FILE* file = fopen(path, "rb");
    OCT_ASSERT(file != nullptr);

    if (file != nullptr)
    {
        int32_t fileSize = 0;
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (maxSize > 0)
        {
            fileSize = glm::min(fileSize, maxSize);
        }

        OCT_ASSERT(fileSize <= MAX_FILE_SIZE);
        Reserve(fileSize);
        fread(mData, fileSize, 1, file);

        fclose(file);
        file = nullptr;

        mSize = fileSize;
        mPos = 0;
    }
    else
    {
        LogError("Stream failed to read file: %s", path);
    }
}

void Stream::WriteFile(const char* path)
{
    FILE* file = fopen(path, "wb");
    OCT_ASSERT(file != nullptr);

    if (file != nullptr)
    {
        fwrite(mData, mSize, 1, file);

        fclose(file);
        file = nullptr;
    }
}

void Stream::SetAsyncRequest(AsyncLoadRequest* request)
{
    mAsyncRequest = request;
}

void Stream::ReadAsset(AssetRef& asset)
{
    // TODO: Resort to default asset if failed to load?
    std::string assetName;
    ReadString(assetName);

    if (mAsyncRequest != nullptr)
    {
        // If we are currently async loading, then we need to also async load this dependency.
        Asset* reqAsset = FetchAsset(assetName);

        if (reqAsset != nullptr)
        {
            asset = reqAsset;
        }
        else
        {
            AssetStub* stub = AssetManager::Get()->GetAssetStub(assetName);

            if (stub != nullptr)
            {
                // The asset does exist, so we need to load it.
                AsyncLoadAsset(assetName, &asset);

                // But also... we need to make sure that this dependency loads before the current async load asset.
                // So we can add this asset stub to the list of dependent assets on the AsyncLoadRequest object
                bool hasDependency = false;
                for (uint32_t i = 0; i < mAsyncRequest->mDependentAssets.size(); ++i)
                {
                    if (mAsyncRequest->mDependentAssets[i] == stub)
                    {
                        hasDependency = true;
                        break;
                    }
                }

                if (!hasDependency)
                {
                    mAsyncRequest->mDependentAssets.push_back(stub);
                }

            }
            else
            {
                LogWarning("Could not find asset %s", assetName.c_str());
            }
        }
    }
    else
    {
        asset = LoadAsset(assetName);
    }
}

void Stream::WriteAsset(const AssetRef& asset)
{
    // Things like MaterialInstance shouldnt be saved since those are only temporary.
    std::string assetName = (asset.Get() && !asset.Get()->IsTransient()) ? asset.Get()->GetName() : "";
    WriteString(assetName);
}

void Stream::ReadString(std::string& dst)
{
    // If updating the integer size for string length, please update STREAM_STRING_LEN_BYTES
    OCT_ASSERT(mPos + sizeof(uint32_t) <= mSize);
    uint32_t stringSize = ReadUint32();
    OCT_ASSERT(stringSize <= MAX_STRING_SIZE);
    dst.resize(stringSize);

    if (stringSize > 0)
    {
        OCT_ASSERT(mPos + stringSize <= mSize);
        dst.assign(&mData[mPos], stringSize);
        mPos += stringSize;
    }
    else
    {
        dst = "";
    }
}

void Stream::WriteString(const std::string& src)
{
    OCT_ASSERT(src.size() <= MAX_STRING_SIZE);
    uint32_t deltaSize = uint32_t(sizeof(uint32_t) + src.size());

    if (mPos + deltaSize > mSize)
    {
        Grow(mPos + deltaSize);
    }

    // If updating the integer size for string length, please update STREAM_STRING_LEN_BYTES
    WriteUint32(uint32_t(src.size()));

    if (src.size() > 0)
    {
        memcpy(&mData[mPos], src.data(), src.size());
        mPos += uint32_t(src.size());
    }
}

void Stream::ReadBytes(uint8_t* dst, uint32_t length)
{
    // If updating the integer size for string length, please update STREAM_STRING_LEN_BYTES
    OCT_ASSERT(mPos + length <= mSize);

    if (length > 0)
    {
        memcpy(dst, &mData[mPos], length);
        mPos += length;
    }
}

void Stream::WriteBytes(uint8_t* src, uint32_t length)
{
    if (mPos + length > mSize)
    {
        Grow(mPos + length);
    }

    if (length > 0)
    {
        memcpy(&mData[mPos], src, length);
        mPos += length;
    }
}

int32_t Stream::ReadInt32()
{
    int32_t ret = 0;
    Read(ret);
    return ret;
}

uint32_t Stream::ReadUint32()
{
    uint32_t ret = 0;
    Read(ret);
    return ret;
}

int16_t Stream::ReadInt16()
{
    int16_t ret = 0;
    Read(ret);
    return ret;
}

uint16_t Stream::ReadUint16()
{
    uint16_t ret = 0;
    Read(ret);
    return ret;
}

int8_t Stream::ReadInt8()
{
    int8_t ret = 0;
    Read(ret);
    return ret;
}

uint8_t Stream::ReadUint8()
{
    uint8_t ret = 0;
    Read(ret);
    return ret;
}

float Stream::ReadFloat()
{
    float ret = 0;
    Read(ret);
    return ret;
}

bool Stream::ReadBool()
{
    uint8_t ret = 0;
    Read(ret);
    return ret != 0;
}

glm::vec2 Stream::ReadVec2()
{
    glm::vec2 ret;
    ret.x = ReadFloat();
    ret.y = ReadFloat();
    return ret;
}

glm::vec3 Stream::ReadVec3()
{
    glm::vec3 ret;
    ret.x = ReadFloat();
    ret.y = ReadFloat();
    ret.z = ReadFloat();
    return ret;
}

glm::vec4 Stream::ReadVec4()
{
    glm::vec4 ret;
    ret.x = ReadFloat();
    ret.y = ReadFloat();
    ret.z = ReadFloat();
    ret.w = ReadFloat();
    return ret;
}

glm::quat Stream::ReadQuat()
{
    glm::quat ret;
    ret.x = ReadFloat();
    ret.y = ReadFloat();
    ret.z = ReadFloat();
    ret.w = ReadFloat();
    return ret;
}

glm::mat4 Stream::ReadMatrix()
{
    glm::mat4 ret;
    float* dst = glm::value_ptr(ret);
    for (uint32_t i = 0; i < 16; ++i)
    {
        dst[i] = ReadFloat();
    }
    return ret;
}

void Stream::WriteInt32(const int32_t& src)
{
    Write(src);
}

void Stream::WriteUint32(const uint32_t& src)
{
    Write(src);
}

void Stream::WriteInt16(const int16_t& src)
{
    Write(src);
}

void Stream::WriteUint16(const uint16_t& src)
{
    Write(src);
}

void Stream::WriteInt8(const int8_t& src)
{
    Write(src);
}

void Stream::WriteUint8(const uint8_t& src)
{
    Write(src);
}

void Stream::WriteFloat(const float& src)
{
    Write(src);
}

void Stream::WriteBool(const bool& src)
{
    uint8_t uintBool = src ? 1 : 0;
    WriteUint8(uintBool);
}

void Stream::WriteVec2(const glm::vec2& src)
{
    Write(src.x);
    Write(src.y);
}

void Stream::WriteVec3(const glm::vec3& src)
{
    Write(src.x);
    Write(src.y);
    Write(src.z);
}

void Stream::WriteVec4(const glm::vec4& src)
{
    Write(src.x);
    Write(src.y);
    Write(src.z);
    Write(src.w);
}

void Stream::WriteQuat(const glm::quat& src)
{
    Write(src.x);
    Write(src.y);
    Write(src.z);
    Write(src.w);
}

void Stream::WriteMatrix(const glm::mat4& src)
{
    const float* srcArray = glm::value_ptr(src);

    for (uint32_t i = 0; i < 16; ++i)
    {
        Write(srcArray[i]);
    }
}

void Stream::Grow(uint32_t newSize)
{
    // Realloc space for new data (will fail and assert if mExternal is true)
    if (!mExternal)
    {
        if (newSize > mCapacity)
        {
            uint32_t newCapacity = glm::max(newSize, mCapacity * 2);
            Reserve(newCapacity);
        }
    }
    else
    {
        OCT_ASSERT(newSize <= mCapacity);
    }

    mSize = newSize;
}

void Stream::Reserve(uint32_t capacity)
{
    if (mExternal)
    {
        LogError("Cannot Reserve() external stream. Capacity is locked at construction time.");
        OCT_ASSERT(0);
        return;
    }

    if (capacity > mCapacity)
    {
        char* newBuffer = (char*) malloc(capacity);

        if (mData != nullptr)
        {
            memcpy(newBuffer, mData, mSize);
            free(mData);
        }

        mData = newBuffer;
        mCapacity = capacity;
    }
}
