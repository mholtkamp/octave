#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Assets/MaterialInstance.h"
#include "Engine.h"
#include "World.h"

DEFINE_RTTI(MeshComponent);

static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    MeshComponent* meshComp = static_cast<MeshComponent*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Material Override")
    {
        meshComp->SetMaterialOverride(*(Material**)newValue);
        success = true;
    }

    return success;
}

MeshComponent::MeshComponent() :
    mMaterialOverride(nullptr)
{

}

MeshComponent::~MeshComponent()
{

}

const char* MeshComponent::GetTypeName() const
{
    return "Mesh";
}

void MeshComponent::GatherProperties(std::vector<Property>& outProps)
{
    PrimitiveComponent::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Asset, "Material Override", this, &mMaterialOverride, 1, HandlePropChange, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Billboard", this, &mBillboard));
}

void MeshComponent::SaveStream(Stream& stream)
{
    PrimitiveComponent::SaveStream(stream);
    stream.WriteAsset(mMaterialOverride);
    stream.WriteBool(mBillboard);
}

void MeshComponent::LoadStream(Stream& stream)
{
    PrimitiveComponent::LoadStream(stream);
    stream.ReadAsset(mMaterialOverride);
    mBillboard = stream.ReadBool();
}

bool MeshComponent::IsShadowMeshComponent()
{
    return false;
}

Material* MeshComponent::GetMaterialOverride()
{
    return mMaterialOverride.Get<Material>();
}

void MeshComponent::SetMaterialOverride(Material* material)
{
    mMaterialOverride = material;
}

MaterialInstance* MeshComponent::InstantiateMaterial()
{
    Material* mat = GetMaterial();
    MaterialInstance* matInst = MaterialInstance::New(mat);
    SetMaterialOverride(matInst);
    return matInst;
}

bool MeshComponent::IsBillboard() const
{
    return mBillboard;
}

void MeshComponent::SetBillboard(bool billboard)
{
    mBillboard = billboard;
}

glm::mat4 MeshComponent::ComputeBillboardTransform()
{
    CameraComponent* camera = GetWorld()->GetActiveCamera();
    glm::quat camQuat = camera->GetAbsoluteRotationQuat();

    glm::mat4 transform = glm::mat4(1);

    glm::vec3 pos = GetAbsolutePosition();
    glm::vec3 scale = GetAbsoluteScale();
    glm::quat rot = GetRotationQuat(); // Not absolute

    transform = glm::translate(transform, pos);
    transform *= glm::toMat4(camQuat);
    transform *= glm::toMat4(rot);
    transform = glm::scale(transform, scale);

    return transform;
}

glm::mat4 MeshComponent::GetRenderTransform()
{
    glm::mat4 transform;
    if (IsBillboard())
    {
        transform = ComputeBillboardTransform();
    }
    else
    {
        transform = GetTransform();
    }
    return transform;
}

DrawData MeshComponent::GetDrawData()
{
    DrawData data = {};
    Material* material = GetMaterial();

    data.mComponent = static_cast<PrimitiveComponent*>(this);
    data.mMaterial = material;
    data.mShadingModel = material ? material->GetShadingModel() : ShadingModel::Lit;
    data.mBlendMode = material ? material->GetBlendMode() : BlendMode::Opaque;
    data.mPosition = GetAbsolutePosition();
    data.mBounds = GetBounds();
    data.mSortPriority = material ? material->GetSortPriority() : 0;
    data.mDepthless = material ? material->IsDepthTestDisabled() : false;

    return data;
}
