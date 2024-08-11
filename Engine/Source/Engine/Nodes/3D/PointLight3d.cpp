#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Camera3d.h"
#include "Renderer.h"
#include "Assets/StaticMesh.h"
#include "Engine.h"
#include "AssetManager.h"
#include "Maths.h"

#if EDITOR
#include "EditorState.h"
#endif

#undef min
#undef max

FORCE_LINK_DEF(PointLight3D);
DEFINE_NODE(PointLight3D, Light3D);

PointLight3D::PointLight3D() :
    mRadius(5)
{
    mName = "Point Light";
}

PointLight3D::~PointLight3D()
{
}

void PointLight3D::Create()
{
    Light3D::Create();
}

void PointLight3D::Destroy()
{
    Light3D::Destroy();
}

const char* PointLight3D::GetTypeName() const
{
    return "PointLight";
}

void PointLight3D::GatherProperties(std::vector<Property>& outProps)
{
    Light3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Light");

    outProps.push_back(Property(DatumType::Float, "Radius", this, &mRadius));
}

void PointLight3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
#if DEBUG_DRAW_ENABLED

    glm::vec4 color = glm::vec4(0.8f, 0.8f, 0.3f, 1.0f);

    if (mDomain == LightingDomain::Static)
    {
        color = glm::vec4(0.8f, 0.5f, 0.3f, 1.0f);
    }
    else if (mDomain == LightingDomain::Dynamic)
    {
        color = glm::vec4(0.8f, 0.8f, 0.6f, 1.0f);
    }

    {
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        debugDraw.mTransform = glm::scale(mTransform, { 0.2f, 0.2f, 0.2f });
        inoutDraws.push_back(debugDraw);
    }

#if EDITOR
    if (GetEditorState()->GetSelectedNode() == this)
    {
        DebugDraw debugDraw;
        debugDraw.mMesh = LoadAsset<StaticMesh>("SM_Sphere");
        debugDraw.mNode = this;
        debugDraw.mColor = color;
        debugDraw.mTransform = glm::scale(mTransform, { mRadius, mRadius, mRadius });
        inoutDraws.push_back(debugDraw);
    }
#endif // EDITOR

#endif // DEBUG_DRAW_ENABLED
}

bool PointLight3D::IsPointLight3D() const
{
    return true;
}

bool PointLight3D::IsDirectionalLight3D() const
{
    return false;
}

void PointLight3D::SetRadius(float radius)
{
    mRadius = radius;
}

float PointLight3D::GetRadius() const
{
    return mRadius;
}
