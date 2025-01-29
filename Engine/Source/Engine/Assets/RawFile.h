#pragma once

#include "Asset.h"
#include "AssetRef.h"

class RawFile : public Asset
{
public:

    DECLARE_ASSET(RawFile, Asset);

    RawFile();
    virtual ~RawFile();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    const std::vector<uint8_t>& GetData() const;
    uint32_t GetDataSize() const;
    const uint8_t* GetDataPointer();
    Stream GetStream() const;

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    std::vector<uint8_t> mData;

};
