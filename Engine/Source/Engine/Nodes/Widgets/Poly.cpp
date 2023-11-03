#include "Nodes/Widgets/Poly.h"
#include "Renderer.h"
#include "Log.h"
#include "Utilities.h"

#include "Graphics/Graphics.h"

#include "Nodes/Widgets/Poly.h"

FORCE_LINK_DEF(Poly);
DEFINE_NODE(Poly, Widget);

Poly::Poly()
{
    GFX_CreatePolyResource(this);
}

Poly::~Poly()
{
    GFX_DestroyPolyResource(this);
}

PolyResource* Poly::GetResource()
{
    return &mResource;
}

void Poly::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);
    TickCommon(deltaTime);
}

void Poly::EditorTick(float deltaTime)
{
    Widget::EditorTick(deltaTime);
    TickCommon(deltaTime);
}

void Poly::TickCommon(float deltaTime)
{
    uint32_t frameIndex = Renderer::Get()->GetFrameIndex();
    if (mUploadVertices[frameIndex])
    {
        GFX_UpdatePolyResourceVertexData(this);
        mUploadVertices[frameIndex] = false;
    }

    if (IsDirty())
    {
        GFX_UpdatePolyResourceUniformData(this);
    }
}

void Poly::Render()
{
    Widget::Render();
    GFX_DrawPoly(this);
}

DrawData Poly::GetDrawData()
{
    DrawData data = {};

    data.mNode = this;

    return data;
}

void Poly::GatherProperties(std::vector<Property>& outProps)
{
    Widget::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Float, "Line Width", this, &mLineWidth));
}


void Poly::MarkVerticesDirty()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mUploadVertices[i] = true;
    }
}

VertexUI* Poly::GetVertices()
{
    return mVertices.data();
}

void Poly::AddVertex(glm::vec2 position,
    glm::vec4 color,
    glm::vec2 uv)
{
    uint32_t color32 = ColorFloat4ToUint32(color);
    mVertices.push_back({ position, uv, color32 });

    MarkVerticesDirty();
}

void Poly::ClearVertices()
{
    mVertices.clear();
    MarkVerticesDirty();
}

uint32_t Poly::GetNumVertices()
{
    return uint32_t(mVertices.size());
}

VertexUI Poly::GetVertex(uint32_t index)
{
    return mVertices[index];
}

void Poly::SetTexture(Texture* texture)
{
    mTexture = texture;
    MarkDirty();
}

Texture* Poly::GetTexture()
{
    return mTexture.Get<Texture>();
}

float Poly::GetLineWidth() const
{
    return mLineWidth;
}

void Poly::SetLineWidth(float width)
{
    mLineWidth = width;
}
