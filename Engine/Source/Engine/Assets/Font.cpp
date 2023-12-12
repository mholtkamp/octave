#include "Assets/Font.h"
#include "Assets/Texture.h"
#include "Renderer.h"

#include "Nodes/Widgets/Text.h"
#include "Nodes/3D/TextMesh3d.h"

#include "AssetManager.h"

#if EDITOR
#include <stb_truetype.h>
#include <stb_image_write.h>
#include <IrrXML/irrXML.h>
using namespace irr;
using namespace io;
#endif

#define USE_TTF_IMPORT 1

FORCE_LINK_DEF(Font);
DEFINE_ASSET(Font);

extern const char* gFilterEnumStrings[];
extern const char* gWrapEnumStrings[];

bool Font::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Font* font = static_cast<Font*>(prop->mOwner);
    bool success = false;

#if EDITOR
    if (prop->mName == "Size")
    {
        int32_t newSize = *((int32_t*)newValue);
        if (font->mSize != newSize)
        {
            font->mSize = newSize;
        }

        success = true;
    }
    else if (prop->mName == "Rebuild Font")
    {
        font->RebuildFont();

        success = true;
    }

#endif

    return success;
}

Font::Font()
{
    mType = Font::GetStaticType();
}

Font::~Font()
{

}

void Font::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mSize = stream.ReadInt32();
    mWidth = stream.ReadInt32();
    mHeight = stream.ReadInt32();
    mBold = stream.ReadBool();
    mItalic = stream.ReadBool();

    mFilterType = (FilterType)stream.ReadUint8();
    mWrapMode = (WrapMode)stream.ReadUint8();
    mMipmapped = stream.ReadBool();

    int32_t numChars = stream.ReadInt32();
    mCharacters.resize(numChars);
    for (int32_t i = 0; i < numChars; ++i)
    {
        Character& charData = mCharacters[i];
        charData.mCodePoint = stream.ReadInt32();
        charData.mX = stream.ReadFloat();
        charData.mY = stream.ReadFloat();
        charData.mWidth = stream.ReadFloat();
        charData.mHeight = stream.ReadFloat();
        charData.mOriginX = stream.ReadFloat();
        charData.mOriginY = stream.ReadFloat();
        charData.mAdvance = stream.ReadFloat();
    }

    bool validTexture = stream.ReadBool();

    if (validTexture)
    {
        Texture* texture = NewTransientAsset<Texture>();
        texture->LoadStream(stream, platform);
        texture->Create();

        mTexture = texture;
    }
}

void Font::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteInt32(mSize);
    stream.WriteInt32(mWidth);
    stream.WriteInt32(mHeight);
    stream.WriteBool(mBold);
    stream.WriteBool(mItalic);

    stream.WriteUint8((uint8_t)mFilterType);
    stream.WriteUint8((uint8_t)mWrapMode);
    stream.WriteBool(mMipmapped);

    stream.WriteInt32((int32_t)mCharacters.size());
    for (uint32_t i = 0; i < mCharacters.size(); ++i)
    {
        const Character& charData = mCharacters[i];
        stream.WriteInt32(charData.mCodePoint);
        stream.WriteFloat(charData.mX);
        stream.WriteFloat(charData.mY);
        stream.WriteFloat(charData.mWidth);
        stream.WriteFloat(charData.mHeight);
        stream.WriteFloat(charData.mOriginX);
        stream.WriteFloat(charData.mOriginY);
        stream.WriteFloat(charData.mAdvance);
    }

    Texture* texture = mTexture.Get<Texture>();
    bool validTexture = (texture != nullptr);
    stream.WriteBool(validTexture);

    if (validTexture)
    {
        stream.WriteAsset(mTexture);
    }
}

void Font::Create()
{
    Asset::Create();
}

void Font::Destroy()
{
    Asset::Destroy();
}

void Font::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);

    bool ttf = path.size() > 4 && !(path.substr(path.size() - 4) == ".xml");

#if EDITOR

    if (ttf)
    {
        // Reset the ttf data.
        mTtfData.Reset();
        mTtfData.ReadFile(path.c_str(), false);
        RebuildFont();
    }
    else
    {
        LogError("Invalid truetype font file.");
    }

#endif // EDITOR
}

void Font::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    //outProps.push_back(Property(DatumType::Asset, "Texture", this, &mTexture, 1, nullptr, int32_t(Texture::GetStaticType())));

    outProps.push_back(Property(DatumType::Integer, "Size", this, &mSize, 1, Font::HandlePropChange));

    outProps.push_back(Property(DatumType::Bool, "Mipmapped", this, &mMipmapped));
    outProps.push_back(Property(DatumType::Integer, "Filter Type", this, &mFilterType, 1, Font::HandlePropChange, 0, int32_t(FilterType::Count), gFilterEnumStrings));
    outProps.push_back(Property(DatumType::Integer, "Wrap Mode", this, &mWrapMode, 1, Font::HandlePropChange, 0, int32_t(WrapMode::Count), gWrapEnumStrings));

#if EDITOR
    static bool sFakeRebuild = false;
    outProps.push_back(Property(DatumType::Bool, "Rebuild Font", this, &sFakeRebuild, 1, Font::HandlePropChange));
#endif
}

glm::vec4 Font::GetTypeColor()
{
    return glm::vec4(0.65f, 1.0f, 0.45f, 1.0f);
}

const char* Font::GetTypeName()
{
    return "Font";
}

const char* Font::GetTypeImportExt()
{
    return ".ttf";
}

int32_t Font::GetSize() const
{
    return mSize;
}

int32_t Font::GetWidth() const
{
    return mWidth;
}

int32_t Font::GetHeight() const
{
    return mHeight;
}

const std::vector<Character>& Font::GetCharacters() const
{
    return mCharacters;
}

Texture* Font::GetTexture() const
{
    return mTexture.Get<Texture>();
}

bool Font::IsBold() const
{
    return mBold;
}

bool Font::IsItalic() const
{
    return mItalic;
}

void Font::SetTexture(Texture* texture)
{
    mTexture = texture;
}

void Font::RebuildFont()
{
#if EDITOR
    if (mTtfData.GetSize() > 0)
    {
        int bakeResult = -1;
        int32_t texWidth = 128;
        int32_t texHeight = 128;

        stbtt_bakedchar charData[96] = {};
        float fontSize = (float)mSize;

        while (bakeResult < 0)
        {
            uint8_t* tempBitmap = new uint8_t[texWidth * texHeight];
            memset(tempBitmap, 0, texWidth * texHeight);

            bakeResult = stbtt_BakeFontBitmap((uint8_t*)mTtfData.GetData(), 0, fontSize, tempBitmap, texWidth, texHeight, 32, 96, charData); // no guarantee this fits!

            if (bakeResult >= 0)
            {
                // Debug: Write out .png
                stbi_write_png("C:/Scrap/font_test.png", texWidth, texHeight, 1, tempBitmap, texWidth);

                // 1. Gather font metadata
                mSize = int32_t(fontSize + 0.5f);
                mWidth = texWidth;
                mHeight = texHeight;
                mBold = false;
                mItalic = false;

                // 2. Create transient Texture asset for the font.
                Texture* texture = NewTransientAsset<Texture>();
                // Texture stores pixel data in RGBA8 always in Editor.
                uint8_t* dataRgba = new uint8_t[texWidth * texHeight * 4];

                uint32_t numPixels = texWidth * texHeight;
                for (uint32_t i = 0; i < numPixels; ++i)
                {
                    dataRgba[i * 4 + 0] = 255;
                    dataRgba[i * 4 + 1] = 255;
                    dataRgba[i * 4 + 2] = 255;
                    dataRgba[i * 4 + 3] = tempBitmap[i];
                }

                texture->Init(texWidth, texHeight, dataRgba);
                texture->SetFilterType(mFilterType);
                texture->SetWrapMode(mWrapMode);
                texture->SetMipmapped(mMipmapped);
                texture->SetFormat(PixelFormat::LA4);
                texture->Create();
                mTexture = texture;

                delete[] dataRgba;

                // 3. Gather all of the ASCII Character data
                mCharacters.clear();

                // This engine really only supports ASCII right now. :skull-emoji:
                // Visible character range is from space to ~
                for (int32_t i = 32; i < 127; ++i)
                {
                    int32_t arrayIdx = i - 32;
                    stbtt_bakedchar& bc = charData[arrayIdx];

                    Character newChar;
                    newChar.mCodePoint = i;
                    newChar.mX = (float)bc.x0;
                    newChar.mY = (float)bc.y0;
                    newChar.mWidth = (float)(bc.x1 - bc.x0);
                    newChar.mHeight = (float)(bc.y1 - bc.y0);
                    newChar.mOriginX = -bc.xoff;
                    newChar.mOriginY = -bc.yoff;
                    newChar.mAdvance = bc.xadvance;

                    mCharacters.push_back(newChar);
                }
            }
            else
            {
                if (texHeight < texWidth)
                    texHeight = texHeight * 2;
                else
                    texWidth = texWidth * 2;
            }

            delete[] tempBitmap;
            tempBitmap = nullptr;

            // Dirty all text
            if (GetWorld() != nullptr)
            {
                Node* rootNode = GetWorld()->GetRootNode();

                auto dirtyText = [&](Node* node) -> bool
                {
                    Text* text = node->As<Text>();
                    if (text)
                    {
                        text->MarkDirty();
                        text->MarkVerticesDirty();
                    }
                    
                    TextMesh3D* text3d = node->As<TextMesh3D>();
                    if (text3d)
                    {
                        text3d->MarkVerticesDirty();
                    }

                    return true;
                };

                if (rootNode != nullptr)
                {
                    rootNode->Traverse(dirtyText);
                }
            }
        }
    }
#endif
}
