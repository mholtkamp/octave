#include "Widgets/Quad.h"
#include "Renderer.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

FORCE_LINK_DEF(Quad);
DEFINE_WIDGET(Quad, Widget);

bool Quad::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Quad* quad = static_cast<Quad*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Texture")
    {
        quad->SetTexture(*(Texture**)newValue);
        success = true;
    }

    return success;
}

Quad::Quad() :
    mTexture(nullptr),
    mTint(glm::vec4(1, 1, 1, 1)),
    mUvScale(glm::vec2(1, 1)),
    mUvOffset(glm::vec2(0, 0))
{
    SetName("Quad");
    InitVertexData();
    GFX_CreateQuadResource(this);
}

Quad::~Quad()
{
    GFX_DestroyQuadResource(this);
}

QuadResource* Quad::GetResource()
{
    return &mResource;
}

void Quad::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Asset, "Texture", this, &mTexture, 1, Quad::HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Color, "Tint", this, &mTint, 1, Quad::HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Scale", this, &mUvScale, 1, Quad::HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Offset", this, &mUvOffset, 1, Quad::HandlePropChange));
}

void Quad::Update()
{
    Widget::Update();

    if (IsDirty())
    {
        UpdateVertexData();
        GFX_UpdateQuadResource(this);
    }
}

void Quad::SetTexture(class Texture* texture)
{
    mTexture = texture;
    MarkDirty();
}

Texture* Quad::GetTexture()
{
    return mTexture.Get<Texture>();
}

void Quad::SetColor(glm::vec4 color)
{
    if (mColor != color)
    {
        Widget::SetColor(color);

        // Overriding vertex colors.
        for (int32_t i = 0; i < 4; ++i)
        {
            mVertices[i].mColor = 0xffffffff;
        }

        MarkDirty();
    }
}

void Quad::SetColor(glm::vec4 colors[4])
{
    for (int32_t i = 0; i < 4; ++i)
    {
        uint32_t color32 = ColorFloat4ToUint32(colors[i]);
        mVertices[i].mColor = color32;
    }

    MarkDirty();
}

void Quad::SetColor(glm::vec4 topLeft,
    glm::vec4 topRight,
    glm::vec4 bottomLeft,
    glm::vec4 bottomRight)
{
    glm::vec4 colors[4] = { topLeft, bottomLeft, topRight, bottomRight };
    SetColor(colors);
}

void Quad::SetTint(glm::vec4 tint)
{
    mTint = tint;
    MarkDirty();
}

glm::vec4 Quad::GetTint() const
{
    return mTint;
}

void Quad::SetUvScale(glm::vec2 scale)
{
    mUvScale = scale;
    MarkDirty();
}

glm::vec2 Quad::GetUvScale() const
{
    return mUvScale;
}

void Quad::SetUvOffset(glm::vec2 offset)
{
    mUvOffset = offset;
    MarkDirty();
}

glm::vec2 Quad::GetUvOffset() const
{
    return mUvOffset;
}

void Quad::InitVertexData()
{
    mVertices[0].mPosition.x = 0.0f;
    mVertices[0].mPosition.y = 0.0f;
    mVertices[0].mTexcoord.x = (0.0f + mUvOffset.x) * mUvScale.x;
    mVertices[0].mTexcoord.y = (0.0f + mUvOffset.y) * mUvScale.y;
    mVertices[0].mColor = 0xffffffff;

    mVertices[1].mPosition.x = 0.0f;
    mVertices[1].mPosition.y = 1.0f;
    mVertices[1].mTexcoord.x = (0.0f + mUvOffset.x) * mUvScale.x;
    mVertices[1].mTexcoord.y = (1.0f + mUvOffset.y) * mUvScale.y;
    mVertices[1].mColor = 0xffffffff;

    mVertices[2].mPosition.x = 1.0f;
    mVertices[2].mPosition.y = 0.0f;
    mVertices[2].mTexcoord.x = (1.0f + mUvOffset.x) * mUvScale.x;
    mVertices[2].mTexcoord.y = (0.0f + mUvOffset.y) * mUvScale.y;
    mVertices[2].mColor = 0xffffffff;

    mVertices[3].mPosition.x = 1.0f;
    mVertices[3].mPosition.y = 1.0f;
    mVertices[3].mTexcoord.x = (1.0f + mUvOffset.x) * mUvScale.x;
    mVertices[3].mTexcoord.y = (1.0f + mUvOffset.y) * mUvScale.y;
    mVertices[3].mColor = 0xffffffff;
}

void Quad::UpdateVertexData()
{
    mVertices[0].mPosition.x = mRect.mX;
    mVertices[0].mPosition.y = mRect.mY;
    mVertices[0].mTexcoord.x = (0.0f + mUvOffset.x) * mUvScale.x;
    mVertices[0].mTexcoord.y = (0.0f + mUvOffset.y) * mUvScale.y;

    mVertices[1].mPosition.x = mRect.mX;
    mVertices[1].mPosition.y = mRect.mY + mRect.mHeight;
    mVertices[1].mTexcoord.x = (0.0f + mUvOffset.x) * mUvScale.x;
    mVertices[1].mTexcoord.y = (1.0f + mUvOffset.y) * mUvScale.y;

    mVertices[2].mPosition.x = mRect.mX + mRect.mWidth;
    mVertices[2].mPosition.y = mRect.mY;
    mVertices[2].mTexcoord.x = (1.0f + mUvOffset.x) * mUvScale.x;
    mVertices[2].mTexcoord.y = (0.0f + mUvOffset.y) * mUvScale.y;

    mVertices[3].mPosition.x = mRect.mX + mRect.mWidth;
    mVertices[3].mPosition.y = mRect.mY + mRect.mHeight;
    mVertices[3].mTexcoord.x = (1.0f + mUvOffset.x) * mUvScale.x;
    mVertices[3].mTexcoord.y = (1.0f + mUvOffset.y) * mUvScale.y;
}

void Quad::Render()
{
    Widget::Render();
    GFX_DrawQuad(this);
}

VertexUI* Quad::GetVertices()
{
    return mVertices;
}
