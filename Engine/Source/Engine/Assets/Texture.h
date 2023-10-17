#pragma once

#include <string>
#include "glm/glm.hpp"
#include "Asset.h"

#include "Graphics/GraphicsTypes.h"

class Texture : public Asset
{

public:

    DECLARE_ASSET(Texture, Asset);

    Texture();
    virtual ~Texture();

    TextureResource* GetResource();

    // Asset Interface
    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    void Init(uint32_t width, uint32_t height, uint8_t* data);

    void SetMipmapped(bool mipmapped);
    bool IsMipmapped() const;
    bool IsRenderTarget() const;

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetMipLevels() const;
    uint32_t GetLayers() const;
    PixelFormat GetFormat() const;
    FilterType GetFilterType() const;
    WrapMode GetWrapMode() const;

    static bool HandlePropChange(class Datum* datum, uint32_t index, const void* newValue);

protected:

    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mMipLevels;
    uint32_t mLayers;
    PixelFormat mFormat;
    FilterType mFilterType;
    WrapMode mWrapMode;
    bool mMipmapped;
    bool mRenderTarget;

    // This pixel array is used as an intermediate storage between LoadStream() and Create()
    // It is cleared and shrunk within Create() except when compiled for EDITOR
    std::vector<uint8_t> mPixels;

    // Graphics Resource
    TextureResource mResource;
};
