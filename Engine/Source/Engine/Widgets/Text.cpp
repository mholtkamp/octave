#include "Widgets/Text.h"
#include "Renderer.h"
#include "Assets/Font.h"
#include "AssetManager.h"
#include "Vertex.h"
#include "Maths.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

FORCE_LINK_DEF(Text);
DEFINE_WIDGET(Text, Widget);

static const char* sHoriJustStrings[] =
{
    "Left",
    "Center",
    "Right"
};
static_assert(int32_t(Justification::Count) == 3, "Need to update string conversion table");

static const char* sVertJustStrings[] =
{
    "Top",
    "Center",
    "Bottom"
};
static_assert(int32_t(Justification::Count) == 3, "Need to update string conversion table");

bool Text::HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Text* text = static_cast<Text*>(prop->mOwner);
    bool success = false;

    text->MarkDirty();
    text->MarkVerticesDirty();

    return success;
}

float Text::GetJustificationRatio(Justification just)
{
    float ret = 0.0f;

    switch (just)
    {
    case Justification::Left:
        ret = 0.0f;
        break;
    case Justification::Right:
        ret = 1.0f;
        break;
    case Justification::Center:
        ret = 0.5f;
        break;
    }

    return ret;
}

Text::Text() :
    mFont(nullptr),
    mText("Text"),
    mCutoff(0.55f),
    mOutlineSize(0.0f),
    mTextSize(16.0f),
    mSoftness(0.125f),
    mOutlineColor(0.0f, 0.0f, 0.0, 1.0f),
    mVertices(nullptr),
    mVisibleCharacters(0),
    mNumCharactersAllocated(0)
{
    SetName("Text");
    mFont = LoadAsset<Font>("F_Roboto32");
    MarkVerticesDirty();
    GFX_CreateTextResource(this);
}

Text::~Text()
{
    GFX_DestroyTextResource(this);
}

TextResource* Text::GetResource()
{
    return &mResource;
}

void Text::GatherProperties(std::vector<Property>& outProps, bool editor)
{
    Widget::GatherProperties(outProps, editor);

    outProps.push_back(Property(DatumType::Asset, "Font", this, &mFont, 1, Text::HandlePropChange, int32_t(Font::GetStaticType())));
    outProps.push_back(Property(DatumType::String, "Text", this, &mText, 1, Text::HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Text Size", this, &mTextSize, 1, Text::HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Word Wrap", this, &mWordWrap, 1, Text::HandlePropChange));
    outProps.push_back(Property(DatumType::Byte, "Hori Justification", this, &mHoriJust, 1, Text::HandlePropChange, 0, int32_t(Justification::Count), sHoriJustStrings));
    outProps.push_back(Property(DatumType::Byte, "Vert Justification", this, &mVertJust, 1, Text::HandlePropChange, 0, int32_t(Justification::Count), sVertJustStrings));
}

void Text::Update()
{
    Widget::Update();

    UpdateVertexData();
    UploadVertexData();

    if (IsDirty())
    {
        GFX_UpdateTextResourceUniformData(this);
    }
}

void Text::SetFont(class Font* font)
{
    if (mFont != font)
    {
        mFont = font;
        MarkVerticesDirty();
        MarkDirty();
    }
}

Font* Text::GetFont()
{
    return mFont.Get<Font>();
}

void Text::SetColor(glm::vec4 color)
{
    if (color != mColor)
    {
        Widget::SetColor(color);
        MarkVerticesDirty();
    }
}

void Text::MarkDirty()
{
    Widget::MarkDirty();

    if (mWordWrap)
    {
        MarkVerticesDirty();
    }
}

void Text::SetOutlineColor(glm::vec4 color)
{
    if (mColor != color)
    {
        mOutlineColor = color;
        MarkDirty();
    }
}

glm::vec4 Text::GetOutlineColor() const
{
    return mOutlineColor;
}

void Text::SetTextSize(float size)
{
    if (mTextSize != size)
    {
        mTextSize = size;
        MarkDirty();
    }
}

float Text::GetTextSize() const
{
    return mTextSize;
}

float Text::GetScaledTextSize() const
{
    return mTextSize * glm::min(mAbsoluteScale.x, mAbsoluteScale.y);
}

float Text::GetOutlineSize() const
{
    return mOutlineSize;
}

float Text::GetSoftness() const
{
    return mSoftness;
}

float Text::GetCutoff() const
{
    return mCutoff;
}

void Text::SetHorizontalJustification(Justification just)
{
    if (mHoriJust != just)
    {
        mHoriJust = just;
        MarkVerticesDirty();
    }
}

Justification Text::GetHorizontalJustification() const
{
    return mHoriJust;
}

void Text::SetVerticalJustification(Justification just)
{
    if (mVertJust != just)
    {
        mVertJust = just;
        MarkVerticesDirty();
    }
}

Justification Text::GetVerticalJustification() const
{
    return mVertJust;
}

bool Text::IsWordWrapEnabled() const
{
    return mWordWrap;
}

void Text::EnableWordWrap(bool wrap)
{
    if (mWordWrap != wrap)
    {
        mWordWrap = wrap;
        MarkVerticesDirty();
    }
}

void Text::SetText(const std::string& text)
{
    SetText(text.c_str());
}

void Text::SetText(const char* text)
{
    if (mText != text)
    {
        mText = text;
        MarkVerticesDirty();
        MarkDirty();
    }
}

const std::string& Text::GetText() const
{
    return mText;
}

VertexUI* Text::GetVertices()
{
    return mVertices;
}

uint32_t Text::GetNumCharactersAllocated() const
{
    return mNumCharactersAllocated;
}

uint32_t Text::GetNumVisibleCharacters() const
{
    return mVisibleCharacters;
}

void Text::MarkVerticesDirty()
{
    mReconstructVertices = true;
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mUploadVertices[i] = true;
    }
}

float Text::GetTextWidth()
{
    UpdateVertexData();
    glm::vec2 scaledMax = GetScaledMaxExtent();
    glm::vec2 scaledMin = GetScaledMinExtent();
    return (scaledMax.x - scaledMin.x);
}

float Text::GetTextHeight()
{
    UpdateVertexData();
    glm::vec2 scaledMax = GetScaledMaxExtent();
    glm::vec2 scaledMin = GetScaledMinExtent();
    return (scaledMax.y - scaledMin.y);
}

glm::vec2 Text::GetScaledMinExtent()
{
    UpdateVertexData();

    // TODO: Need to account for GetJustifiedOffset().

    Font* font = mFont.Get<Font>();
    float scale = font ? (mTextSize / font->GetSize()) : 1.0f;
    return mMinExtent * scale;
}

glm::vec2 Text::GetScaledMaxExtent()
{
    UpdateVertexData();

    // TODO: Need to account for GetJustifiedOffset().

    Font* font = mFont.Get<Font>();
    float scale = font ? (mTextSize / font->GetSize()) : 1.0f;
    return mMaxExtent * scale;
}

bool Text::ContainsPoint(int32_t x, int32_t y)
{
    // This code isn't tested? 
    // But I think it's right.
    glm::vec2 minExt = GetScaledMinExtent();
    glm::vec2 maxExt = GetScaledMaxExtent();

    Rect textRect;
    textRect.mX = minExt.x;
    textRect.mY = minExt.y;
    textRect.mWidth = (maxExt.x - minExt.x);
    textRect.mHeight = (maxExt.y - minExt.y);

    textRect.Clamp(mCachedScissorRect);
    return textRect.ContainsPoint((float)x, (float)y);
}

void Text::UpdateVertexData()
{
    if (!mReconstructVertices ||
        mFont == nullptr)
        return;

    // Check if we need to reallocate a bigger buffer.
    if (mText.size() > mNumCharactersAllocated)
    {
        if (mVertices != nullptr)
        {
            delete [] mVertices;
            mVertices = nullptr;
        }

        const uint32_t allocGranularity = 32;
        uint32_t numCharsToAllocate = allocGranularity * ((uint32_t(mText.size()) + allocGranularity - 1) / allocGranularity);

        mVertices = new VertexUI[numCharsToAllocate * 6];
        mNumCharactersAllocated = numCharsToAllocate;
    }

    Font* font = mFont.Get<Font>();
    OCT_ASSERT(font != nullptr);
    int32_t fontSize = font->GetSize();
    int32_t fontWidth = font->GetWidth();
    int32_t fontHeight = font->GetHeight();
    const std::vector<Character>& fontChars = font->GetCharacters();

    mVisibleCharacters = 0;
    mMinExtent = glm::vec2(0.0f, 0.0f);
    mMaxExtent = glm::vec2(0.0f, 0.0f);

    if (mText.size() == 0)
    {
        mReconstructVertices = false;
        return;
    }

    // Run through each of the characters and construct vertices for it.
    // Not using an index buffer currently, so each character is 6 vertices.
    // Topology is triangles.
    int32_t lineVertStart = 0;
    int32_t wordVertStart = 0;

    uint32_t color32 = ColorFloat4ToUint32(mColor);

    const char* characters = mText.c_str();
    float cursorX = 0.0f;
    float cursorY = 0.0f + font->GetSize();

    float textScale = GetScaledTextSize() / fontSize;

    for (uint32_t i = 0; i < mText.size(); ++i)
    {
        char textChar = characters[i];
        if (textChar == '\n')
        {
            cursorY += fontSize;
            cursorX = 0.0f;

            JustifyLine(mVertices, mHoriJust, lineVertStart, mVisibleCharacters * TEXT_VERTS_PER_CHAR);
            wordVertStart = lineVertStart;
            continue;
        }

        // Only ASCII is supported.
        if (textChar < ' ' ||
            textChar > '~')
        {
            continue;
        }

        const Character& fontChar = fontChars[textChar - ' '];
        VertexUI* vertices = mVertices + (mVisibleCharacters * TEXT_VERTS_PER_CHAR);

        //   0---2  3
        //   |  / / |
        //   | / /  |
        //   1  4---5
        vertices[0].mPosition.x = cursorX - fontChar.mOriginX;
        vertices[0].mPosition.y = cursorY - fontChar.mOriginY;
        vertices[0].mTexcoord.x = (float)fontChar.mX;
        vertices[0].mTexcoord.y = (float)fontChar.mY;

        vertices[1].mPosition.x = cursorX - fontChar.mOriginX;
        vertices[1].mPosition.y = cursorY - fontChar.mOriginY + fontChar.mHeight;
        vertices[1].mTexcoord.x = (float)fontChar.mX;
        vertices[1].mTexcoord.y = (float)fontChar.mY + fontChar.mHeight;

        vertices[2].mPosition.x = cursorX - fontChar.mOriginX + fontChar.mWidth;
        vertices[2].mPosition.y = cursorY - fontChar.mOriginY;
        vertices[2].mTexcoord.x = (float)fontChar.mX + fontChar.mWidth;
        vertices[2].mTexcoord.y = (float)fontChar.mY;

        vertices[3] = vertices[2]; // duplicated
        vertices[4] = vertices[1]; // duplicated

        vertices[5].mPosition.x = cursorX - fontChar.mOriginX + fontChar.mWidth;
        vertices[5].mPosition.y = cursorY - fontChar.mOriginY + fontChar.mHeight;
        vertices[5].mTexcoord.x = (float)fontChar.mX + fontChar.mWidth;
        vertices[5].mTexcoord.y = (float)fontChar.mY + fontChar.mHeight;

        for (int32_t i = 0; i < 6; ++i)
        {
            // Fill out uniform data first.
            // TODO: Maybe we should just remove color from vertex data.
            vertices[i].mColor = color32;

            // Transform texcoords into 0-1 UV space
            vertices[i].mTexcoord /= glm::vec2(fontWidth, fontHeight);
        }

        mVisibleCharacters++;
        cursorX += fontChar.mAdvance;

        // Check for wordwrap
        if (textChar == ' ')
        {
            wordVertStart = (mVisibleCharacters) * TEXT_VERTS_PER_CHAR;
        }
        else if (mWordWrap &&
            wordVertStart != lineVertStart &&
            (cursorX - fontChar.mOriginX + fontChar.mWidth) * textScale > mRect.mWidth)
        {
            JustifyLine(mVertices, mHoriJust, lineVertStart, wordVertStart);

            float deltaX = -mVertices[wordVertStart].mPosition.x;
            float deltaY = (float)fontSize;

            for (int32_t i = wordVertStart; i < mVisibleCharacters * TEXT_VERTS_PER_CHAR; ++i)
            {
                mVertices[i].mPosition.x += deltaX;
                mVertices[i].mPosition.y += deltaY;
            }

            cursorX += deltaX;
            cursorY += deltaY;
        }
    }

    JustifyLine(mVertices, mHoriJust, lineVertStart, mVisibleCharacters * TEXT_VERTS_PER_CHAR);

    // Update extents
    mMinExtent = glm::vec2(FLT_MAX, FLT_MAX);
    mMaxExtent = glm::vec2(-FLT_MAX, -FLT_MAX);
    for (int32_t i = 0; i < mVisibleCharacters; ++i)
    {
        mMinExtent = glm::min(mMinExtent, mVertices[i * TEXT_VERTS_PER_CHAR + 0].mPosition);
        mMaxExtent = glm::max(mMaxExtent, mVertices[i * TEXT_VERTS_PER_CHAR + 5].mPosition);
    }

    // Vertical Justification
    if (mVertJust != Justification::Top)
    {
        float topGap = mMinExtent.y;
        float vertJust = GetJustificationRatio(mVertJust);
        float deltaY = -(mMaxExtent.y - mMinExtent.y + topGap * 2) * vertJust;

        const int32_t numVerts = mVisibleCharacters * TEXT_VERTS_PER_CHAR;
        for (int32_t i = 0; i < numVerts; ++i)
        {
            mVertices[i].mPosition.y += deltaY;
        }

        mMinExtent.y += deltaY;
        mMaxExtent.y += deltaY;
    }

    mReconstructVertices = false;
}

void Text::UploadVertexData()
{
    uint32_t frameIndex = Renderer::Get()->GetFrameIndex();
    if (mUploadVertices[frameIndex])
    {
        GFX_UpdateTextResourceVertexData(this);
        mUploadVertices[frameIndex] = false;
    }
}

void Text::JustifyLine(VertexUI* vertices, Justification just, int32_t& lineVertStart, int32_t numVerts)
{
    if (just != Justification::Left &&
        lineVertStart < numVerts)
    {
        float horiJust = GetJustificationRatio(just);
        float deltaX = -(vertices[numVerts - 1].mPosition.x - vertices[lineVertStart].mPosition.x);
        deltaX *= horiJust;

        for (int32_t i = lineVertStart; i < numVerts; ++i)
        {
            vertices[i].mPosition.x += deltaX;
        }
    }

    lineVertStart = numVerts;
}

void Text::Render()
{
    Widget::Render();
    GFX_DrawText(this);
}

glm::vec2 Text::GetJustifiedOffset()
{
    glm::vec2 offset = glm::vec2(
        mRect.mWidth * GetJustificationRatio(mHoriJust),
        mRect.mHeight * GetJustificationRatio(mVertJust));

    return offset;
}
