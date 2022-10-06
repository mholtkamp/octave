#include "Components/TextMeshComponent.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Log.h"

#include "Assets/Font.h"
#include "Assets/MaterialInstance.h"

#include "Graphics/Graphics.h"

FORCE_LINK_DEF(TextMeshComponent);
DEFINE_COMPONENT(TextMeshComponent);

static const char* sJustificationStrings[] =
{
    "Left",
    "Center",
    "Right"
};
static_assert(int32_t(Justification::Count) == 3, "Need to update string conversion table");

extern const char* gBlendModeStrings[];

bool TextMeshComponent::HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    assert(prop != nullptr);
    TextMeshComponent* textComp = static_cast<TextMeshComponent*>(prop->mOwner);

    textComp->MarkVertexBufferDirty();

    return false;
}

TextMeshComponent::TextMeshComponent()
{
    mName = "Text Mesh";

    mFont = LoadAsset<Font>("F_Roboto32");
    mText = "Text";

    MarkVertexBufferDirty();
}

TextMeshComponent::~TextMeshComponent()
{

}

const char* TextMeshComponent::GetTypeName() const
{
    return "TextMesh";
}

void TextMeshComponent::GatherProperties(std::vector<Property>& outProps)
{
    MeshComponent::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Asset, "Font", this, &mFont, 1, HandlePropChange, int32_t(Font::GetStaticType())));
    outProps.push_back(Property(DatumType::String, "Text", this, &mText, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Color", this, &mColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Enum, "Justification", this, &mJustification, 1, HandlePropChange, 0, (int32_t)Justification::Count, sJustificationStrings));
    outProps.push_back(Property(DatumType::Enum, "Blend Mode", this, &mBlendMode, 1, HandlePropChange, 0, (int32_t)BlendMode::Count, gBlendModeStrings));
}

void TextMeshComponent::Create()
{
    MeshComponent::Create();
    GFX_CreateTextMeshCompResource(this);

    // To make things easier, each TextMeshComponent creates a material instance
    // to use in the case that a material override is not provided.
    // Each frame, the default mat instance is updated so that it's TEXTURE_0 is the font texture
    // And its color is set to mColor.
    mDefaultMatInstance = MaterialInstance::New(LoadAsset<Material>("M_DefaultUnlit"));
    mDefaultMatInstance.Get<Material>()->SetBlendMode(mBlendMode);
}

void TextMeshComponent::Destroy()
{
    MeshComponent::Destroy();
    GFX_DestroyTextMeshCompResource(this);
}

TextMeshCompResource* TextMeshComponent::GetResource()
{
    return &mResource;
}

void TextMeshComponent::SaveStream(Stream& stream)
{
    MeshComponent::SaveStream(stream);
    stream.WriteAsset(mFont);
    stream.WriteString(mText);
    stream.WriteVec4(mColor);
    stream.WriteUint32((uint32_t)mJustification);
}

void TextMeshComponent::LoadStream(Stream& stream)
{
    MeshComponent::LoadStream(stream);
    stream.ReadAsset(mFont);
    stream.ReadString(mText);
    mColor = stream.ReadVec4();
    mJustification = (Justification)stream.ReadUint32();
}

void TextMeshComponent::Tick(float deltaTime)
{
    MeshComponent::Tick(deltaTime);
    UpdateVertexData();

    // Update default material instance
    Material* matInst = mDefaultMatInstance.Get<Material>();
    Font* font = mFont.Get<Font>();

    if (font)
    {
        mDefaultMatInstance.Get<Material>()->SetTexture(TEXTURE_0, font->GetTexture());
    }

    mDefaultMatInstance.Get<Material>()->SetColor(mColor);
    mDefaultMatInstance.Get<Material>()->SetBlendMode(mBlendMode);
}


bool TextMeshComponent::IsStaticMeshComponent() const
{
    return false;
}

bool TextMeshComponent::IsSkeletalMeshComponent() const
{
    return false;
}

Material* TextMeshComponent::GetMaterial()
{
    Material* mat = mMaterialOverride.Get<Material>();

    if (mat == nullptr)
    {
        mat = mDefaultMatInstance.Get<Material>();
    }

    return mat;
}

void TextMeshComponent::Render()
{
    GFX_DrawTextMeshComp(this);
}

void TextMeshComponent::SetText(const std::string& text)
{
    if (mText != text)
    {
        mText = text;
        MarkVertexBufferDirty();
    }
}

const std::string& TextMeshComponent::GetText() const
{
    return mText;
}


void TextMeshComponent::SetFont(Font* font)
{
    if (mFont != font)
    {
        mFont = font;
        MarkVertexBufferDirty();
    }
}

Font* TextMeshComponent::GetFont() const
{
    return mFont.Get<Font>();
}

void TextMeshComponent::SetColor(glm::vec4 color)
{
    mColor = color;
}

glm::vec4 TextMeshComponent::GetColor() const
{
    return mColor;
}

Bounds TextMeshComponent::GetLocalBounds() const
{
    return mBounds;
}

void TextMeshComponent::MarkVertexBufferDirty()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mVertexBufferDirty[i] = true;
    }
}

bool TextMeshComponent::IsVertexBufferDirty() const
{
    return mVertexBufferDirty[Renderer::Get()->GetFrameIndex()];
}

int32_t TextMeshComponent::GetNumVisibleCharacters() const
{
    return mVisibleCharacters;
}

void TextMeshComponent::UpdateVertexData()
{
    if (!IsVertexBufferDirty() ||
        mFont == nullptr)
        return;

    // TODO: See about sharing most of this code with Text (widget)

    // Check if we need to reallocate a bigger buffer.
    mVertices.resize(mText.size() * TEXT_VERTS_PER_CHAR);

    Font* font = mFont.Get<Font>();
    assert(font != nullptr);
    int32_t fontSize = font->GetSize();
    int32_t fontWidth = font->GetWidth();
    int32_t fontHeight = font->GetHeight();
    const std::vector<Character>& fontChars = font->GetCharacters();

    mVisibleCharacters = 0;
    mBounds.mCenter = { 0.0f, 0.0f, 0.0f };
    mBounds.mRadius = 0.0f;

    if (mText.size() == 0)
        return;

    // Run through each of the characters and construct vertices for it.
    // Not using an index buffer currently, so each character is 6 vertices.
    // Topology is triangles.

    glm::vec2 minExtent = glm::vec2(FLT_MAX, FLT_MAX);
    glm::vec2 maxExtent = glm::vec2(-FLT_MAX, -FLT_MAX);

    uint32_t color32 = ColorFloat4ToUint32(mColor);

    const char* characters = mText.c_str();
    float cursorX = 0.0f;
    float cursorY = 0.0f + font->GetSize();

    for (uint32_t i = 0; i < mText.size(); ++i)
    {
        char textChar = characters[i];
        if (textChar == '\n')
        {
            cursorY += fontSize;
            cursorX = 0.0f;
            continue;
        }

        // Only ASCII is supported.
        if (textChar < ' ' ||
            textChar > '~')
        {
            continue;
        }

        const Character& fontChar = fontChars[textChar - ' '];
        Vertex* vertices = mVertices.data() + (mVisibleCharacters * TEXT_VERTS_PER_CHAR);

        //   0---2  3
        //   |  / / |
        //   | / /  |
        //   1  4---5
        vertices[0].mPosition.x = cursorX - fontChar.mOriginX;
        vertices[0].mPosition.y = cursorY - fontChar.mOriginY;
        vertices[0].mTexcoord0.x = (float)fontChar.mX;
        vertices[0].mTexcoord0.y = (float)fontChar.mY;
        vertices[0].mTexcoord1.x = vertices[0].mPosition.x;
        vertices[0].mTexcoord1.y = vertices[0].mPosition.y;
        vertices[0].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        vertices[1].mPosition.x = cursorX - fontChar.mOriginX;
        vertices[1].mPosition.y = cursorY - fontChar.mOriginY + fontChar.mHeight;
        vertices[1].mTexcoord0.x = (float)fontChar.mX;
        vertices[1].mTexcoord0.y = (float)fontChar.mY + fontChar.mHeight;
        vertices[1].mTexcoord1.x = vertices[1].mPosition.x;
        vertices[1].mTexcoord1.y = vertices[1].mPosition.y;
        vertices[1].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        vertices[2].mPosition.x = cursorX - fontChar.mOriginX + fontChar.mWidth;
        vertices[2].mPosition.y = cursorY - fontChar.mOriginY;
        vertices[2].mTexcoord0.x = (float)fontChar.mX + fontChar.mWidth;
        vertices[2].mTexcoord0.y = (float)fontChar.mY;
        vertices[2].mTexcoord1.x = vertices[2].mPosition.x;
        vertices[2].mTexcoord1.y = vertices[2].mPosition.y;
        vertices[2].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        vertices[3] = vertices[2]; // duplicated
        vertices[4] = vertices[1]; // duplicated

        vertices[5].mPosition.x = cursorX - fontChar.mOriginX + fontChar.mWidth;
        vertices[5].mPosition.y = cursorY - fontChar.mOriginY + fontChar.mHeight;
        vertices[5].mTexcoord0.x = (float)fontChar.mX + fontChar.mWidth;
        vertices[5].mTexcoord0.y = (float)fontChar.mY + fontChar.mHeight;
        vertices[5].mTexcoord1.x = vertices[5].mPosition.x;
        vertices[5].mTexcoord1.y = vertices[5].mPosition.y;
        vertices[5].mNormal = glm::vec3(0.0f, 0.0f, 1.0f);

        // Update the extents
        minExtent.x = glm::min(minExtent.x, vertices[0].mPosition.x);
        minExtent.y = glm::min(minExtent.y, vertices[0].mPosition.y);

        maxExtent.x = glm::max(maxExtent.x, vertices[5].mPosition.x);
        maxExtent.y = glm::max(maxExtent.y, vertices[5].mPosition.y);

        for (int32_t i = 0; i < 6; ++i)
        {
            // Transform texcoords into 0-1 UV space
            vertices[i].mTexcoord0 /= glm::vec2(fontWidth, fontHeight);
        }

        mVisibleCharacters++;
        cursorX += fontChar.mAdvance;
    }

    mBounds.mCenter = glm::vec3((minExtent + maxExtent) / 2.0f, 0.0f);
    mBounds.mRadius = glm::distance(glm::vec3(maxExtent, 0.0f), mBounds.mCenter);

    GFX_UpdateTextMeshCompVertexBuffer(this, mVertices);
    mVertexBufferDirty[Renderer::Get()->GetFrameIndex()] = false;
}
