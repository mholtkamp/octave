#include "Nodes/3D/TextMesh3d.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Log.h"

#include "Assets/Font.h"
#include "Assets/MaterialInstance.h"
#include "Assets/MaterialLite.h"

#include "Graphics/Graphics.h"

FORCE_LINK_DEF(TextMesh3D);
DEFINE_NODE(TextMesh3D, Mesh3D);

extern const char* gBlendModeStrings[];

bool TextMesh3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    TextMesh3D* textComp = static_cast<TextMesh3D*>(prop->mOwner);

    textComp->MarkVerticesDirty();

    return false;
}

TextMesh3D::TextMesh3D()
{
    mName = "Text Mesh";

    mFont = LoadAsset<Font>("F_Roboto32");
    mText = "Text";

    MarkVerticesDirty();
}

TextMesh3D::~TextMesh3D()
{

}

const char* TextMesh3D::GetTypeName() const
{
    return "TextMesh";
}

void TextMesh3D::GatherProperties(std::vector<Property>& outProps)
{
    Mesh3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Text");

    outProps.push_back(Property(DatumType::Asset, "Font", this, &mFont, 1, HandlePropChange, int32_t(Font::GetStaticType())));
    outProps.push_back(Property(DatumType::String, "Text", this, &mText, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Color", this, &mColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Horizontal Justification", this, &mHorizontalJustification, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Vertical Justification", this, &mVerticalJustification, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Blend Mode", this, &mBlendMode, 1, HandlePropChange, 0, (int32_t)BlendMode::Count, gBlendModeStrings));
}

void TextMesh3D::Create()
{
    Mesh3D::Create();
    GFX_CreateTextMeshCompResource(this);

    // To make things easier, each TextMesh3D creates a material instance
    // to use in the case that a material override is not provided.
    // Each frame, the default mat instance is updated so that it's TEXTURE_0 is the font texture
    // And its color is set to mColor.
    mDefaultMatInstance = MaterialLite::New(LoadAsset<MaterialLite>("M_DefaultUnlit"));
    mDefaultMatInstance.Get<MaterialLite>()->SetBlendMode(mBlendMode);
}

void TextMesh3D::Destroy()
{
    Mesh3D::Destroy();
    GFX_DestroyTextMeshCompResource(this);
}

TextMeshCompResource* TextMesh3D::GetResource()
{
    return &mResource;
}

void TextMesh3D::Tick(float deltaTime)
{
    Mesh3D::Tick(deltaTime);
    TickCommon(deltaTime);
}

void TextMesh3D::EditorTick(float deltaTime)
{
    Mesh3D::EditorTick(deltaTime);
    TickCommon(deltaTime);
}

void TextMesh3D::TickCommon(float deltaTime)
{
    UpdateVertexData();
    UploadVertexData();

    // TODO: Need to update TextMesh3D to properly use new material system.

    // Update default material instance
    MaterialLite* matInst = mDefaultMatInstance.Get<MaterialLite>();
    Font* font = mFont.Get<Font>();

    if (font)
    {
        matInst->SetTexture(TEXTURE_0, font->GetTexture());
    }

    matInst->SetColor(mColor);
    matInst->SetOpacity(mColor.a);
    matInst->SetBlendMode(mBlendMode);
}

bool TextMesh3D::IsStaticMesh3D() const
{
    return false;
}

bool TextMesh3D::IsSkeletalMesh3D() const
{
    return false;
}

Material* TextMesh3D::GetMaterial()
{
    Material* mat = mMaterialOverride.Get<Material>();

    if (mat == nullptr)
    {
        mat = mDefaultMatInstance.Get<Material>();
    }

    return mat;
}

void TextMesh3D::Render()
{
    GFX_DrawTextMeshComp(this);
}

void TextMesh3D::SetText(const std::string& text)
{
    if (mText != text)
    {
        mText = text;
        MarkVerticesDirty();
    }
}

const std::string& TextMesh3D::GetText() const
{
    return mText;
}


void TextMesh3D::SetFont(Font* font)
{
    if (mFont != font)
    {
        mFont = font;
        MarkVerticesDirty();
    }
}

Font* TextMesh3D::GetFont() const
{
    return mFont.Get<Font>();
}

void TextMesh3D::SetColor(glm::vec4 color)
{
    mColor = color;
}

glm::vec4 TextMesh3D::GetColor() const
{
    return mColor;
}

void TextMesh3D::SetBlendMode(BlendMode blendMode)
{
    mBlendMode = blendMode;
}

BlendMode TextMesh3D::GetBlendMode() const
{
    return mBlendMode;
}

void TextMesh3D::SetHorizontalJustification(float just)
{
    if (mHorizontalJustification != just)
    {
        mHorizontalJustification = just;
        MarkVerticesDirty();
    }
}

float TextMesh3D::GetHorizontalJustification() const
{
    return mHorizontalJustification;
}

void TextMesh3D::SetVerticalJustification(float just)
{
    if (mVerticalJustification != just)
    {
        mVerticalJustification = just;
        MarkVerticesDirty();
    }
}

float TextMesh3D::GetVerticalJustification() const
{
    return mVerticalJustification;
}

Bounds TextMesh3D::GetLocalBounds() const
{
    Bounds retBounds = mBounds;

    if (IsBillboard())
    {
        float horiJust = glm::clamp(mHorizontalJustification, 0.0f, 1.0f);
        float boundsScale = 1.0f + (fabs(horiJust - 0.5f) * 4);
        retBounds.mRadius *= boundsScale;
    }

    return retBounds;
}

void TextMesh3D::MarkVerticesDirty()
{
    mReconstructVertices = true;
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mUploadVertices[i] = true;
    }
}

void TextMesh3D::UploadVertexData()
{
    // Upload vertices to GPU
    uint32_t frameIndex = Renderer::Get()->GetFrameIndex();
    if (mUploadVertices[frameIndex])
    {
        GFX_UpdateTextMeshCompVertexBuffer(this, mVertices);
        mUploadVertices[frameIndex] = false;
    }
}

int32_t TextMesh3D::GetNumVisibleCharacters() const
{
    return mVisibleCharacters;
}

const Vertex* TextMesh3D::GetVertices() const
{
    return mVertices.data();
}

void TextMesh3D::JustifyLine(glm::vec2& lineMinExtent, glm::vec2& lineMaxExtent, int32_t& lineVertStart)
{
    const int32_t numVerts = mVisibleCharacters * TEXT_VERTS_PER_CHAR;

    if (mHorizontalJustification != 0.0f &&
        lineVertStart < numVerts)
    {
        // Clamp to logical range
        float horiJust = glm::clamp(mHorizontalJustification, 0.0f, 1.0f);
        float deltaX = -(lineMaxExtent.x - lineMinExtent.x) * horiJust;

        for (int32_t i = lineVertStart; i < numVerts; ++i)
        {
            mVertices[i].mPosition.x += deltaX;
        }
    }

    lineMinExtent = glm::vec2(FLT_MAX, FLT_MAX);
    lineMaxExtent = glm::vec2(-FLT_MAX, -FLT_MAX);
    lineVertStart = numVerts;
}

void TextMesh3D::UpdateVertexData()
{
    if (!mReconstructVertices ||
        mFont == nullptr)
        return;

    // TODO: See about sharing most of this code with Text (widget)

    // Check if we need to reallocate a bigger buffer.
    mVertices.clear();
    mVertices.reserve(mText.size() * TEXT_VERTS_PER_CHAR);

    Font* font = mFont.Get<Font>();
    OCT_ASSERT(font != nullptr);
    int32_t fontSize = font->GetSize();
    int32_t fontWidth = font->GetWidth();
    int32_t fontHeight = font->GetHeight();
    float lineSpacing = font->GetLineSpacing();
    const std::vector<Character>& fontChars = font->GetCharacters();

    mVisibleCharacters = 0;
    mBounds.mCenter = { 0.0f, 0.0f, 0.0f };
    mBounds.mRadius = 0.0f;

    if (mText.size() == 0)
    {
        mReconstructVertices = false;
        return;
    }

    // Run through each of the characters and construct vertices for it.
    // Not using an index buffer currently, so each character is 6 vertices.
    // Topology is triangles.

    glm::vec2 minExtent = glm::vec2(FLT_MAX, FLT_MAX);
    glm::vec2 maxExtent = glm::vec2(-FLT_MAX, -FLT_MAX);
    glm::vec2 lineMinExtent = minExtent;
    glm::vec2 lineMaxExtent = maxExtent;
    int32_t lineVertStart = 0;

    const char* characters = mText.c_str();
    float cursorX = 0.0f;
    float cursorY = 0.0f;

    for (uint32_t i = 0; i < mText.size(); ++i)
    {
        char textChar = characters[i];
        if (textChar == '\n')
        {
            cursorY -= (fontSize + lineSpacing);
            cursorX = 0.0f;

            JustifyLine(lineMinExtent, lineMaxExtent, lineVertStart);
            continue;
        }

        // Only ASCII is supported.
        if (textChar < ' ' ||
            textChar > '~')
        {
            continue;
        }

        mVertices.resize(mVertices.size() + TEXT_VERTS_PER_CHAR);

        const Character& fontChar = fontChars[textChar - ' '];
        Vertex* vertices = mVertices.data() + (mVisibleCharacters * TEXT_VERTS_PER_CHAR);

        //   0---2  3
        //   |  / / |
        //   | / /  |
        //   1  4---5
        vertices[0].mPosition.x = cursorX - fontChar.mOriginX;
        vertices[0].mPosition.y = cursorY + fontChar.mOriginY;
        vertices[0].mTexcoord0.x = fontChar.mX;
        vertices[0].mTexcoord0.y = fontChar.mY;
        vertices[0].mTexcoord1.x = vertices[0].mPosition.x;
        vertices[0].mTexcoord1.y = vertices[0].mPosition.y;
        vertices[0].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        vertices[1].mPosition.x = cursorX - fontChar.mOriginX;
        vertices[1].mPosition.y = cursorY + fontChar.mOriginY - fontChar.mHeight;
        vertices[1].mTexcoord0.x = fontChar.mX;
        vertices[1].mTexcoord0.y = fontChar.mY + fontChar.mHeight;
        vertices[1].mTexcoord1.x = vertices[1].mPosition.x;
        vertices[1].mTexcoord1.y = vertices[1].mPosition.y;
        vertices[1].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        vertices[2].mPosition.x = cursorX - fontChar.mOriginX + fontChar.mWidth;
        vertices[2].mPosition.y = cursorY + fontChar.mOriginY;
        vertices[2].mTexcoord0.x = fontChar.mX + fontChar.mWidth;
        vertices[2].mTexcoord0.y = fontChar.mY;
        vertices[2].mTexcoord1.x = vertices[2].mPosition.x;
        vertices[2].mTexcoord1.y = vertices[2].mPosition.y;
        vertices[2].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        vertices[3] = vertices[2]; // duplicated
        vertices[4] = vertices[1]; // duplicated

        vertices[5].mPosition.x = cursorX - fontChar.mOriginX + fontChar.mWidth;
        vertices[5].mPosition.y = cursorY + fontChar.mOriginY - fontChar.mHeight;
        vertices[5].mTexcoord0.x = fontChar.mX + fontChar.mWidth;
        vertices[5].mTexcoord0.y = fontChar.mY + fontChar.mHeight;
        vertices[5].mTexcoord1.x = vertices[5].mPosition.x;
        vertices[5].mTexcoord1.y = vertices[5].mPosition.y;
        vertices[5].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        for (int32_t i = 0; i < 6; ++i)
        {
            // Transform texcoords into 0-1 UV space
            vertices[i].mTexcoord0 /= glm::vec2(fontWidth, fontHeight);

            // Put text into approx a 1 unit range
            vertices[i].mPosition /= fontSize;
        }

        // Update the extents
        lineMinExtent.x = glm::min(lineMinExtent.x, vertices[0].mPosition.x);
        lineMinExtent.y = glm::min(lineMinExtent.y, vertices[0].mPosition.y);

        lineMaxExtent.x = glm::max(lineMaxExtent.x, vertices[5].mPosition.x);
        lineMaxExtent.y = glm::max(lineMaxExtent.y, vertices[5].mPosition.y);

        minExtent = glm::min(minExtent, lineMinExtent);
        maxExtent = glm::max(maxExtent, lineMaxExtent);

        mVisibleCharacters++;
        cursorX += fontChar.mAdvance;
    }

    JustifyLine(lineMinExtent, lineMaxExtent, lineVertStart);

    // Justify the entire block
    if (mVerticalJustification != 0.0f)
    {
        float vertJust = glm::clamp(mVerticalJustification, 0.0f, 1.0f);
        float deltaY = -(maxExtent.y - minExtent.y) * vertJust;

        for (uint32_t i = 0; i < mVertices.size(); ++i)
        {
            mVertices[i].mPosition.y -= deltaY;
        }
    }

    UpdateBounds();
    mReconstructVertices = false;
}

void TextMesh3D::UpdateBounds()
{
    mBounds = ComputeBounds(mVertices);
}
