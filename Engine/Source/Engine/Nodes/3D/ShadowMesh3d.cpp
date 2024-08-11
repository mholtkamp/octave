#include "Nodes/3D/ShadowMesh3d.h"
#include "Graphics/Graphics.h"

FORCE_LINK_DEF(ShadowMesh3D);
DEFINE_NODE(ShadowMesh3D, StaticMesh3D);

ShadowMesh3D::ShadowMesh3D()
{
    mName = "Shadow Mesh";
    mBakeLighting = false;
}

ShadowMesh3D::~ShadowMesh3D()
{

}

const char* ShadowMesh3D::GetTypeName() const
{
    return "ShadowMesh";
}

void ShadowMesh3D::GatherProperties(std::vector<Property>& outProps)
{
    StaticMesh3D::GatherProperties(outProps);
    SCOPED_CATEGORY("Mesh");
}

void ShadowMesh3D::Create()
{
    StaticMesh3D::Create();
}

void ShadowMesh3D::Destroy()
{
    StaticMesh3D::Destroy();
}

void ShadowMesh3D::Render()
{
    GFX_DrawShadowMeshComp(this);
}

bool ShadowMesh3D::IsShadowMesh3D() const
{
    return true;
}

void ShadowMesh3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    StaticMesh3D::GatherProxyDraws(inoutDraws);

    DebugDraw debugDraw;
    debugDraw.mMesh = mStaticMesh.Get<StaticMesh>();
    debugDraw.mTransform = mTransform;
    debugDraw.mColor = glm::vec4(0.6f, 0.0f, 1.0f, 1.0f);
    debugDraw.mNode = this;
    inoutDraws.push_back(debugDraw);
}
