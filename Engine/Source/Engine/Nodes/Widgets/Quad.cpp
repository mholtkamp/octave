#include "Nodes/Widgets/Quad.h"
#include "Renderer.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

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
        for (int32_t i = 0; i < 4; ++i)
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
                // Display at native texture size from widget's position
                posW = texW;
                posH = texH;
            }
        }
    }

    // Apply mUvScale/mUvOffset on top of computed base UVs
    mVertices[0].mPosition.x = posX;
    mVertices[0].mPosition.y = posY;
    mVertices[0].mTexcoord.x = (uvX0 + mUvOffset.x) * mUvScale.x;
    mVertices[0].mTexcoord.y = (uvY0 + mUvOffset.y) * mUvScale.y;

    mVertices[1].mPosition.x = posX;
    mVertices[1].mPosition.y = posY + posH;
    mVertices[1].mTexcoord.x = (uvX0 + mUvOffset.x) * mUvScale.x;
    mVertices[1].mTexcoord.y = (uvY1 + mUvOffset.y) * mUvScale.y;

    mVertices[2].mPosition.x = posX + posW;
    mVertices[2].mPosition.y = posY;
    mVertices[2].mTexcoord.x = (uvX1 + mUvOffset.x) * mUvScale.x;
    mVertices[2].mTexcoord.y = (uvY0 + mUvOffset.y) * mUvScale.y;

    mVertices[3].mPosition.x = posX + posW;
    mVertices[3].mPosition.y = posY + posH;
    mVertices[3].mTexcoord.x = (uvX1 + mUvOffset.x) * mUvScale.x;
    mVertices[3].mTexcoord.y = (uvY1 + mUvOffset.y) * mUvScale.y;
}

void Quad::Render()
{
    Widget::Render();
    GFX_DrawQuad(this);
}

void Quad::PreRender()
{
    Super::PreRender();

    if (IsDirty())
    {
        UpdateVertexData();
        GFX_UpdateQuadResourceVertexData(this);
    }
}

VertexUI* Quad::GetVertices()
{
    return mVertices;
}
