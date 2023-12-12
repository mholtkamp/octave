#pragma once

#include <vector>
#include <string>
#include "Assertion.h"
#include "Maths.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "System/SystemConstants.h"

#define STREAM_STRING_LEN_BYTES 4

class AssetRef;
struct AsyncLoadRequest;

class Stream
{
public:

    Stream();
    Stream(const char* externalData, uint32_t externalSize);
    ~Stream();

    char* GetData();
    uint32_t GetSize() const;
    uint32_t GetPos();
    void SetPos(uint32_t pos);
    void Reset();

    void ReadFile(const char* path, bool isAsset, int32_t maxSize = 0);
    void WriteFile(const char* path);

    void SetAsyncRequest(AsyncLoadRequest* request);

    void ReadAsset(AssetRef& asset);
    void WriteAsset(const AssetRef& asset);

    void ReadString(std::string& dst);
    void WriteString(const std::string& src);

    void ReadBytes(uint8_t* dst, uint32_t length);
    void WriteBytes(uint8_t* src, uint32_t length);

    uint32_t ReadBytesMax(uint8_t* dst, uint32_t length);

    int32_t ReadInt32();
    uint32_t ReadUint32();
    int16_t ReadInt16();
    uint16_t ReadUint16();
    int8_t ReadInt8();
    uint8_t ReadUint8();
    float ReadFloat();
    bool ReadBool();
    glm::vec2 ReadVec2();
    glm::vec3 ReadVec3();
    glm::vec4 ReadVec4();
    glm::quat ReadQuat();
    glm::mat4 ReadMatrix();

    void WriteInt32(const int32_t& src);
    void WriteUint32(const uint32_t& src);
    void WriteInt16(const int16_t& src);
    void WriteUint16(const uint16_t& src);
    void WriteInt8(const int8_t& src);
    void WriteUint8(const uint8_t& src);
    void WriteFloat(const float& src);
    void WriteBool(const bool& src);
    void WriteVec2(const glm::vec2& src);
    void WriteVec3(const glm::vec3& src);
    void WriteVec4(const glm::vec4& src);
    void WriteQuat(const glm::quat& src);
    void WriteMatrix(const glm::mat4& src);

    std::string GetLine();
    int32_t Scan(const char* format, ...);

private:

    void Grow(uint32_t newSize);
    void Reserve(uint32_t capacity);

    template<typename T>
    void Swap32(T& dst)
    {
        OCT_ASSERT(sizeof(T) == 4);

        uint8_t* charArray = reinterpret_cast<uint8_t*>(&dst);
        uint8_t c0 = charArray[0];
        uint8_t c1 = charArray[1];
        uint8_t c2 = charArray[2];
        uint8_t c3 = charArray[3];

        charArray[0] = c3;
        charArray[1] = c2;
        charArray[2] = c1;
        charArray[3] = c0;
    }

    template<typename T>
    void Swap16(T& dst)
    {
        OCT_ASSERT(sizeof(T) == 2);

        uint8_t* charArray = reinterpret_cast<uint8_t*>(&dst);
        uint8_t c0 = charArray[0];
        uint8_t c1 = charArray[1];

        charArray[0] = c1;
        charArray[1] = c0;
    }

    template<typename T>
    void Read(T& dst)
    {
        OCT_ASSERT(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4);
        OCT_ASSERT(mPos + sizeof(T) <= mSize);
        memcpy(&dst, &mData[mPos], sizeof(T));
        mPos += sizeof(T);

#if ENDIAN_SWAP
        if (sizeof(T) == 4)
        {
            Swap32(dst);
        }
        else if (sizeof(T) == 2)
        {
            Swap16(dst);
        }
#endif
    }

    template<typename T>
    void Write(const T& src)
    {
        OCT_ASSERT(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4);

        T srcSwapped = src;

#if ENDIAN_SWAP
        if (sizeof(T) == 4)
        {
            Swap32(srcSwapped);
        }
        else if (sizeof(T) == 2)
        {
            Swap16(srcSwapped);
        }
#endif

        uint32_t deltaSize = uint32_t(sizeof(T));
        if (mPos + deltaSize > mSize)
        {
            Grow(mPos + deltaSize);
        }
        memcpy(&mData[mPos], &srcSwapped, sizeof(T));
        mPos += deltaSize;
    }

private:

    char* mData;
    uint32_t mSize;
    uint32_t mCapacity;
    uint32_t mPos;
    AsyncLoadRequest* mAsyncRequest;
    bool mExternal;
};
