#pragma once

#include "Asset.h"
#include "AssetRef.h"
#include "Texture.h"

struct Character
{
    int32_t mCodePoint = 0;
    float mX = 0;
    float mY = 0;
    float mWidth = 0;
    float mHeight = 0;
    float mOriginX = 0;
    float mOriginY = 0;
    float mAdvance = 0;
};

class Font : public Asset
{
public:

    DECLARE_ASSET(Font, Asset);

    Font();
    virtual ~Font();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    int32_t GetSize() const;
    int32_t GetWidth() const;
    int32_t GetHeight() const;
    const std::vector<Character>& GetCharacters() const;
    Texture* GetTexture() const;
    bool IsBold() const;
    bool IsItalic() const;

    void SetTexture(Texture* texture);

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    int32_t mSize = 0;
    int32_t mWidth = 0;
    int32_t mHeight = 0;
    std::vector<Character> mCharacters;
    TextureRef mTexture;
    bool mBold = false;
    bool mItalic = false;
};
