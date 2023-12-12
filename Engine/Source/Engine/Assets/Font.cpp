#include "Assets/Font.h"
#include "Assets/Texture.h"

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
            LogError("HANDLE FONT RESIZE");
        }

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
        // For using STB Truetype, I am referencing https://github.com/justinmeiners/stb-truetype-example/tree/master

#if 1
        Stream fontStream;
        fontStream.ReadFile(path.c_str(), false);

        if (fontStream.GetSize() > 0)
        {
            int bakeResult = -1;
            int32_t texWidth = 128;
            int32_t texHeight = 128;

            stbtt_bakedchar charData[96];
            float fontSize = 48.0f;

            while (bakeResult < 0)
            {
                uint8_t* tempBitmap = new uint8_t[texWidth * texHeight];
                memset(tempBitmap, 0, texWidth * texHeight);

                bakeResult = stbtt_BakeFontBitmap((uint8_t*)fontStream.GetData(), 0, fontSize, tempBitmap, texWidth, texHeight, 32, 96, charData); // no guarantee this fits!

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
                    texture->SetFilterType(FilterType::Linear);
                    texture->SetWrapMode(WrapMode::Clamp);
                    texture->SetFormat(PixelFormat::LA4);
                    texture->SetMipmapped(true);
                    texture->Create();
                    mTexture = texture;

                    delete[] dataRgba;

                    // 3. Gather all of the ASCII Character data
                    mCharacters.clear();

                    // This engine really only supports ASCII right now. :skull-emoji:
                    // Visible character range is from space to ~
                    for (int32_t i = 32; i < 127; ++i)
                    //for (int32_t i = (int32_t)'A'; i < (int32_t)'H'; ++i)
                    {
                        //float xPos = 0.0f;
                        //float yPos = 0.0f;
                        //stbtt_aligned_quad quad;
                        //stbtt_GetBakedQuad(
                        //    charData, texWidth, texHeight,  // same data as above
                        //    i,                              // character to display
                        //    &xPos, &yPos,                   // pointers to current position in screen pixel space
                        //    &quad,                          // output: quad to draw
                        //    true);                          // true if opengl fill rule; false if DX9 or earlier

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
            }
        }
#elif 0
        Stream fontStream;
        fontStream.ReadFile(path.c_str(), false);

        if (fontStream.GetSize() > 0)
        {
            stbtt_fontinfo info;
            if (!stbtt_InitFont(&info, (uint8_t*)fontStream.GetData(), 0))
            {
                LogError("Failed to initialize ttf font.");
            }

            int32_t b_w = 512;
            int32_t b_h = 128;
            uint8_t* bitmap = (uint8_t*)calloc(b_w * b_h, sizeof(uint8_t));

            float fontSize = 32.0f;
            float scale = stbtt_ScaleForPixelHeight(&info, fontSize);
            float invScale = 1 / scale;

            int32_t x = 0;
            int32_t lineY = 0;

            int32_t ascent = 0;
            int32_t descent = 0;
            int32_t lineGap = 0;
            stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

            ascent = (int32_t)roundf(ascent * scale);
            descent = (int32_t)roundf(descent * scale);

            // This engine really only supports ASCII right now. :skull-emoji:
            // Visible character range is from space to ~
            for (int32_t i = 32; i < 127; ++i)
            //for (int32_t i = (int32_t)'A'; i < (int32_t)'H'; ++i)
            {
                int32_t ax;
                int32_t lsb;
                stbtt_GetCodepointHMetrics(&info, i, &ax, &lsb);

                int32_t c_x1;
                int32_t c_y1;
                int32_t c_x2;
                int32_t c_y2;
                stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

                if (x + c_x2 > b_w)
                {
                    // Move to next line
                    //lineY += (int32_t)(fontSize);
                    lineY += (int32_t)(ascent - descent);

                    // Reset horizontal position
                    x = 0;

                    // Recalculate bitmap box.
                    int32_t c_x1;
                    int32_t c_y1;
                    int32_t c_x2;
                    int32_t c_y2;
                    stbtt_GetCodepointBitmapBox(&info, i, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
                }

                int y = lineY + ascent + c_y1;

                int byteOffset = int32_t(x + roundf(lsb * scale) + (y * b_w));
                stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, i);

                x += (int32_t)roundf(ax * scale);

                int32_t kern = stbtt_GetCodepointKernAdvance(&info, i, i);
                x += (int32_t)roundf(kern * scale);
            }

            stbi_write_png("C:/Scrap/font_test.png", b_w, b_h, 1, bitmap, b_w);
        }

#else

            /* load font file */
        long size;
        unsigned char* fontBuffer;

        FILE* fontFile = fopen(path.c_str(), "rb");
        fseek(fontFile, 0, SEEK_END);
        size = ftell(fontFile); /* how long is the file ? */
        fseek(fontFile, 0, SEEK_SET); /* reset */

        fontBuffer = (unsigned char*) malloc(size);

        fread(fontBuffer, size, 1, fontFile);
        fclose(fontFile);

        /* prepare font */
        stbtt_fontinfo info;
        if (!stbtt_InitFont(&info, fontBuffer, 0))
        {
            printf("failed\n");
        }

        int b_w = 512; /* bitmap width */
        int b_h = 128; /* bitmap height */
        int l_h = 64; /* line height */

        /* create a bitmap for the phrase */
        unsigned char* bitmap = (unsigned char*)calloc(b_w * b_h, sizeof(unsigned char));

        /* calculate font scaling */
        float scale = stbtt_ScaleForPixelHeight(&info, (float) l_h);

        char* word = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890~!@#$%^&*()_+=-";

        int x = 0;

        int ascent, descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

        ascent = (int32_t) roundf(ascent * scale);
        descent = (int32_t) roundf(descent * scale);

        int i;
        for (i = 0; i < strlen(word); ++i)
        {
            /* how wide is this character */
            int ax;
            int lsb;
            stbtt_GetCodepointHMetrics(&info, word[i], &ax, &lsb);
            /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character word[i].) */

            /* get bounding box for character (may be offset to account for chars that dip above or below the line) */
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&info, word[i], scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

            /* compute y (different characters have different heights) */
            int y = ascent + c_y1;

            /* render character (stride and offset is important here) */
            int byteOffset = int( x + roundf(lsb * scale) + (y * b_w) );
            stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, b_w, scale, scale, word[i]);

            /* advance x */
            x += (int) roundf(ax * scale);

            /* add kerning */
            int kern;
            kern = stbtt_GetCodepointKernAdvance(&info, word[i], word[i + 1]);
            x += (int) roundf(kern * scale);
        }

        /* save out a 1 channel image */
        stbi_write_png("C:/Scrap/out.png", b_w, b_h, 1, bitmap, b_w);

        /*
         Note that this example writes each character directly into the target image buffer.
         The "right thing" to do for fonts that have overlapping characters is
         MakeCodepointBitmap to a temporary buffer and then alpha blend that onto the target image.
         See the stb_truetype.h header for more info.
        */

        free(fontBuffer);
        free(bitmap);
#endif


    }
    else
    {
        LogError("Invalid truetype font file.");
    }

#if 0
    else
    {
        // This xml reading code is taken from the sample in irrXML.h
        IrrXMLReader* xml = createIrrXMLReader(path.c_str());

        while (xml && xml->read())
        {
            switch (xml->getNodeType())
            {
            case EXN_TEXT:
                // in this xml file, the only text which occurs is the messageText
                //messageText = xml->getNodeData();
                break;
            case EXN_ELEMENT:
            {
                if (!strcmp("font", xml->getNodeName()))
                {
                    std::string size = xml->getAttributeValue("size");
                    std::string width = xml->getAttributeValue("width");
                    std::string height = xml->getAttributeValue("height");
                    std::string bold = xml->getAttributeValue("bold");
                    std::string italic = xml->getAttributeValue("italic");

                    try
                    {
                        mSize = std::stoi(size);
                        mWidth = std::stoi(width);
                        mHeight = std::stoi(height);
                        mBold = (bold == "true");
                        mItalic = (italic == "true");
                    }
                    catch (...)
                    {

                    }

                }
                else if (!strcmp("character", xml->getNodeName()))
                {
                    std::string codePoint = xml->getAttributeValue("text");
                    std::string x = xml->getAttributeValue("x");
                    std::string y = xml->getAttributeValue("y");
                    std::string width = xml->getAttributeValue("width");
                    std::string height = xml->getAttributeValue("height");
                    std::string originX = xml->getAttributeValue("origin-x");
                    std::string originY = xml->getAttributeValue("origin-y");
                    std::string advance = xml->getAttributeValue("advance");

                    if (codePoint == "&quot")
                        codePoint = "\"";
                    else if (codePoint == "&amp")
                        codePoint = "&";
                    else if (codePoint == "&lt")
                        codePoint = "<";
                    else if (codePoint == "&gt")
                        codePoint = ">";

                    if (codePoint.length() >= 1)
                    {
                        try
                        {
                            Character newChar;
                            newChar.mCodePoint = codePoint[0];
                            newChar.mX = std::stoi(x);
                            newChar.mY = std::stoi(y);
                            newChar.mWidth = std::stoi(width);
                            newChar.mHeight = std::stoi(height);
                            newChar.mOriginX = std::stoi(originX);
                            newChar.mOriginY = std::stoi(originY);
                            newChar.mAdvance = std::stoi(advance);

                            mCharacters.push_back(newChar);
                        }
                        catch (...)
                        {

                        }
                    }
                }
            }
            break;
            }
        }

        delete xml;
        xml = nullptr;
    }
#endif


#endif
}

void Font::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    //outProps.push_back(Property(DatumType::Asset, "Texture", this, &mTexture, 1, nullptr, int32_t(Texture::GetStaticType())));
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
