#include "Nodes/Widgets/Quad.h"
#include "Renderer.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

#include <cmath>
#include <cstring>

FORCE_LINK_DEF(Quad);
DEFINE_NODE(Quad, Widget);

static const char* sObjectFitStrings[] =
{
    "Fill",
    "Contain",
    "Cover",
    "None"
};
static_assert(int32_t(ObjectFit::Count) == 4, "Need to update ObjectFit string table");

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

    quad->MarkDirty();

    return success;
}

Quad::Quad() :
    mTexture(nullptr),
    mUvScale(glm::vec2(1, 1)),
    mUvOffset(glm::vec2(0, 0))
{
    SetName("Quad");
    
}

Quad::~Quad()
{

}

void Quad::Create()
{
    Widget::Create();

    InitVertexData();
    GFX_CreateQuadResource(this);
}

void Quad::Destroy()
{
    if (mBorderResourceCreated)
    {
        GFX_DestroyQuadBorderResource(this);
        mBorderResourceCreated = false;
    }

    GFX_DestroyQuadResource(this);

    Widget::Destroy();
}

QuadResource* Quad::GetResource()
{
    return &mResource;
}

void Quad::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);
    
    SCOPED_CATEGORY("Quad");
    GatherQuadProperties(outProps);
}

void Quad::GatherQuadProperties(std::vector<Property>& outProps)
{
    outProps.push_back(Property(DatumType::Asset, "Texture", this, &mTexture, 1, Quad::HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "Object Fit", this, &mObjectFit, 1, Quad::HandlePropChange, NULL_DATUM, int32_t(ObjectFit::Count), sObjectFitStrings));
    outProps.push_back(Property(DatumType::Vector2D, "UV Scale", this, &mUvScale, 1, Quad::HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Offset", this, &mUvOffset, 1, Quad::HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Corner Radius", this, &mCornerRadius, 1, Quad::HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Border Width", this, &mBorderWidth, 1, Quad::HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Border Color", this, &mBorderColor, 1, Quad::HandlePropChange));
}

DrawData Quad::GetDrawData()
{
    DrawData data = {};

    data.mNode = this;

    return data;
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
        for (uint32_t i = 0; i < mNumVertices; ++i)
        {
            mVertices[i].mColor = 0xffffffff;
        }

        MarkDirty();
    }
}

void Quad::SetColor(glm::vec4 colors[4])
{
    // Override uniform color
    Widget::SetColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // Per-vertex gradient no longer supported with fan geometry.
    // Use the first color as the uniform color instead.
    Widget::SetColor(colors[0]);

    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        mVertices[i].mColor = 0xffffffff;
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

void Quad::SetObjectFit(ObjectFit fit)
{
    mObjectFit = fit;
    MarkDirty();
}

ObjectFit Quad::GetObjectFit() const
{
    return mObjectFit;
}

void Quad::SetCornerRadius(float radius)
{
    mCornerRadius = radius;
    MarkDirty();
}

float Quad::GetCornerRadius() const
{
    return mCornerRadius;
}

void Quad::SetBorderWidth(float width)
{
    if (mBorderWidth != width)
    {
        mBorderWidth = width;
        MarkDirty();
    }
}

float Quad::GetBorderWidth() const
{
    return mBorderWidth;
}

void Quad::SetBorderColor(glm::vec4 color)
{
    if (mBorderColor != color)
    {
        mBorderColor = color;
        MarkDirty();
    }
}

glm::vec4 Quad::GetBorderColor() const
{
    return mBorderColor;
}

void Quad::UpdateBorders()
{
    if (mBorderWidth > 0.0f)
    {
        float bw = mBorderWidth;

        // Generate a larger rounded rectangle that serves as the border background.
        // Expand position by -bw and dimensions by +2*bw on each axis.
        float borderPosX = mRect.mX - bw;
        float borderPosY = mRect.mY - bw;
        float borderW = mRect.mWidth + 2.0f * bw;
        float borderH = mRect.mHeight + 2.0f * bw;

        // Use same corner radius + border width so the outer edge curves match.
        float borderRadius = mCornerRadius > 0.0f ? mCornerRadius + bw : 0.0f;

        // Solid color fill — trivial 0-1 UVs with white texture.
        mBorderNumVertices = GenerateRoundedFan(
            mBorderVertices,
            borderPosX, borderPosY, borderW, borderH,
            borderRadius,
            0.0f, 0.0f, 1.0f, 1.0f);

        if (!mBorderResourceCreated)
        {
            GFX_CreateQuadBorderResource(this);
            mBorderResourceCreated = true;
        }

        GFX_UpdateQuadBorderResourceVertexData(this);
    }
    else
    {
        mBorderNumVertices = 0;
    }
}

void Quad::InitVertexData()
{
    // Zero-fill entire vertex array to avoid garbage in unused slots.
    memset(mVertices, 0, sizeof(mVertices));
    memset(mBorderVertices, 0, sizeof(mBorderVertices));

    // Default non-rounded fan: center + 4 corners + close = 6 vertices.
    mNumVertices = GenerateRoundedFan(mVertices, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
}

uint32_t Quad::GenerateRoundedFan(
    VertexUI* outVertices,
    float posX, float posY, float posW, float posH,
    float cornerRadius,
    float uvX0, float uvY0, float uvX1, float uvY1)
{
    float uvRangeX = uvX1 - uvX0;
    float uvRangeY = uvY1 - uvY0;

    auto makeVertex = [&](float nx, float ny) -> VertexUI
    {
        VertexUI v;
        v.mPosition.x = posX + nx * posW;
        v.mPosition.y = posY + ny * posH;
        v.mTexcoord.x = uvX0 + nx * uvRangeX;
        v.mTexcoord.y = uvY0 + ny * uvRangeY;
        v.mColor = 0xffffffff;
        return v;
    };

    float r = cornerRadius;

    if (r > 0.0f && posW > 0.0f && posH > 0.0f)
    {
        r = glm::min(r, glm::min(posW * 0.5f, posH * 0.5f));

        float nrx = r / posW;
        float nry = r / posH;

        uint32_t idx = 0;

        outVertices[idx++] = makeVertex(0.5f, 0.5f);

        static constexpr float kPi = 3.14159265358979323846f;
        static constexpr float kHalfPi = kPi * 0.5f;

        struct CornerInfo { float cx, cy, startAngle; };
        CornerInfo corners[4] =
        {
            { 1.0f - nrx, nry,        -kHalfPi },
            { 1.0f - nrx, 1.0f - nry,  0.0f    },
            { nrx,        1.0f - nry,  kHalfPi  },
            { nrx,        nry,         kPi      },
        };

        float segAngle = kHalfPi / (float)kCornerSegments;

        for (int32_t c = 0; c < 4; ++c)
        {
            for (int32_t s = 0; s <= kCornerSegments; ++s)
            {
                float angle = corners[c].startAngle + s * segAngle;
                float nx = corners[c].cx + nrx * cosf(angle);
                float ny = corners[c].cy + nry * sinf(angle);
                outVertices[idx++] = makeVertex(nx, ny);
            }
        }

        outVertices[idx] = outVertices[1];
        idx++;

        return idx;
    }
    else
    {
        outVertices[0] = makeVertex(0.5f, 0.5f);
        outVertices[1] = makeVertex(0.0f, 0.0f);
        outVertices[2] = makeVertex(1.0f, 0.0f);
        outVertices[3] = makeVertex(1.0f, 1.0f);
        outVertices[4] = makeVertex(0.0f, 1.0f);
        outVertices[5] = makeVertex(0.0f, 0.0f);
        return 6;
    }
}

void Quad::UpdateVertexData()
{
    float posX = mRect.mX;
    float posY = mRect.mY;
    float posW = mRect.mWidth;
    float posH = mRect.mHeight;

    float uvX0 = 0.0f;
    float uvY0 = 0.0f;
    float uvX1 = 1.0f;
    float uvY1 = 1.0f;

    Texture* tex = mTexture.Get<Texture>();

    if (mObjectFit != ObjectFit::Fill && tex != nullptr && posW > 0.0f && posH > 0.0f)
    {
        float texW = (float)tex->GetWidth();
        float texH = (float)tex->GetHeight();

        if (texW > 0.0f && texH > 0.0f)
        {
            float texAR = texW / texH;
            float widgetAR = posW / posH;

            if (mObjectFit == ObjectFit::Contain)
            {
                float fitW, fitH;
                if (widgetAR > texAR)
                {
                    fitH = posH;
                    fitW = fitH * texAR;
                }
                else
                {
                    fitW = posW;
                    fitH = fitW / texAR;
                }

                posX = mRect.mX + (posW - fitW) * 0.5f;
                posY = mRect.mY + (posH - fitH) * 0.5f;
                posW = fitW;
                posH = fitH;
            }
            else if (mObjectFit == ObjectFit::Cover)
            {
                if (widgetAR > texAR)
                {
                    float uvH = texAR / widgetAR;
                    uvY0 = (1.0f - uvH) * 0.5f;
                    uvY1 = uvY0 + uvH;
                }
                else
                {
                    float uvW = widgetAR / texAR;
                    uvX0 = (1.0f - uvW) * 0.5f;
                    uvX1 = uvX0 + uvW;
                }
            }
            else if (mObjectFit == ObjectFit::None)
            {
                posW = texW;
                posH = texH;
            }
        }
    }

    // Apply UV scale/offset after generating the fan
    mNumVertices = GenerateRoundedFan(
        mVertices,
        posX, posY, posW, posH,
        mCornerRadius,
        uvX0, uvY0, uvX1, uvY1);

    // Apply UV scale and offset to generated vertices
    for (uint32_t i = 0; i < mNumVertices; ++i)
    {
        mVertices[i].mTexcoord.x = (mVertices[i].mTexcoord.x + mUvOffset.x) * mUvScale.x;
        mVertices[i].mTexcoord.y = (mVertices[i].mTexcoord.y + mUvOffset.y) * mUvScale.y;
    }
}

void Quad::Render()
{
    Widget::Render();

    if (mBorderNumVertices > 0)
    {
        GFX_DrawQuadBorder(this);
    }

    GFX_DrawQuad(this);
}

void Quad::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateVertexData();
        UpdateBorders();
        GFX_UpdateQuadResourceVertexData(this);
    }
}

VertexUI* Quad::GetVertices()
{
    return mVertices;
}

uint32_t Quad::GetNumVertices() const
{
    return mNumVertices;
}

QuadResource* Quad::GetBorderResource()
{
    return &mBorderResource;
}

VertexUI* Quad::GetBorderVertices()
{
    return mBorderVertices;
}

uint32_t Quad::GetBorderNumVertices() const
{
    return mBorderNumVertices;
}
