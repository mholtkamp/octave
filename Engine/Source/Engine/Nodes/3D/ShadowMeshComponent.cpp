#include "Nodes/3D/ShadowMeshComponent.h"
#include "Graphics/Graphics.h"

FORCE_LINK_DEF(ShadowMeshComponent);
DEFINE_NODE(ShadowMeshComponent);

ShadowMeshComponent::ShadowMeshComponent()
{
    mName = "Shadow Mesh";
    mBakeLighting = false;
}

ShadowMeshComponent::~ShadowMeshComponent()
{

}

const char* ShadowMeshComponent::GetTypeName() const
{
    return "ShadowMesh";
}

void ShadowMeshComponent::GatherProperties(std::vector<Property>& outProps)
{
    StaticMeshComponent::GatherProperties(outProps);
}

void ShadowMeshComponent::Create()
{
    StaticMeshComponent::Create();
}

void ShadowMeshComponent::Destroy()
{
    StaticMeshComponent::Destroy();
}

void ShadowMeshComponent::Render()
{
    GFX_DrawShadowMeshComp(this);
}

void ShadowMeshComponent::SaveStream(Stream& stream)
{
    StaticMeshComponent::SaveStream(stream);
}

void ShadowMeshComponent::LoadStream(Stream& stream)
{
    StaticMeshComponent::LoadStream(stream);
}

bool ShadowMeshComponent::IsShadowMeshComponent()
{
    return true;
}

void ShadowMeshComponent::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    StaticMeshComponent::GatherProxyDraws(inoutDraws);

    DebugDraw debugDraw;
    debugDraw.mMesh = mStaticMesh.Get<StaticMesh>();
    debugDraw.mTransform = mTransform;
    debugDraw.mColor = glm::vec4(0.6f, 0.0f, 1.0f, 1.0f);
    debugDraw.mActor = GetOwner();
    debugDraw.mComponent = this;
    inoutDraws.push_back(debugDraw);
}
