#include "Assets/Font.h"

#if EDITOR
#include <IrrXML/irrXML.h>
using namespace irr;
using namespace io;
#endif

DEFINE_ASSET(Font);

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
    stream.ReadAsset(mTexture);

    int32_t numChars = stream.ReadInt32();
    mCharacters.resize(numChars);
    for (int32_t i = 0; i < numChars; ++i)
    {
        Character& charData = mCharacters[i];
        charData.mCodePoint = stream.ReadInt32();
        charData.mX = stream.ReadInt32();
        charData.mY = stream.ReadInt32();
        charData.mWidth = stream.ReadInt32();
        charData.mHeight = stream.ReadInt32();
        charData.mOriginX = stream.ReadInt32();
        charData.mOriginY = stream.ReadInt32();
        charData.mAdvance = stream.ReadInt32();
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
    stream.WriteAsset(mTexture);

    stream.WriteInt32((int32_t)mCharacters.size());
    for (uint32_t i = 0; i < mCharacters.size(); ++i)
    {
        const Character& charData = mCharacters[i];
        stream.WriteInt32(charData.mCodePoint);
        stream.WriteInt32(charData.mX);
        stream.WriteInt32(charData.mY);
        stream.WriteInt32(charData.mWidth);
        stream.WriteInt32(charData.mHeight);
        stream.WriteInt32(charData.mOriginX);
        stream.WriteInt32(charData.mOriginY);
        stream.WriteInt32(charData.mAdvance);
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

void Font::Import(const std::string& path)
{
    Asset::Import(path);

#if EDITOR
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

#endif
}

void Font::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Asset, "Texture", this, &mTexture, 1, nullptr, int32_t(Texture::GetStaticType())));
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
    return ".xml";
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
