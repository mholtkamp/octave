#include "Nodes/3D/Mesh3d.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Assets/MaterialInstance.h"
#include "Assets/MaterialLite.h"
#include "AssetManager.h"
#include "Engine.h"
#include "World.h"

DEFINE_OBJECT(Mesh3D);

bool Mesh3D::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Mesh3D* meshComp = static_cast<Mesh3D*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Material Override")
    {
        meshComp->SetMaterialOverride(*(Material**)newValue);
        success = true;
    }

    return success;
}

Mesh3D::Mesh3D() :
    mMaterialOverride(nullptr)
{

}

Mesh3D::~Mesh3D()
{

}

const char* Mesh3D::GetTypeName() const
{
    return "Mesh";
}

void Mesh3D::GatherProperties(std::vector<Property>& outProps)
{
    Primitive3D::GatherProperties(outProps);

    SCOPED_CATEGORY("Mesh");

    outProps.push_back(Property(DatumType::Asset, "Material Override", this, &mMaterialOverride, 1, HandlePropChange, int32_t(Material::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Billboard", this, &mBillboard));
}

static const uint32_t sMesh3dExtraMagic = 0x4D455348; // "MESH"

void Mesh3D::SaveStream(Stream& stream, Platform platform)
{
    stream.WriteUint32(sMesh3dExtraMagic);

    Material* mat = mMaterialOverride.Get<Material>();
    bool hasInlineMat = (mat != nullptr && mat->IsTransient() && mat->IsLite());
    stream.WriteBool(hasInlineMat);

    if (hasInlineMat)
    {
        static_cast<MaterialLite*>(mat)->SaveLiteParams(stream);
    }
}

void Mesh3D::LoadStream(Stream& stream, Platform platform, uint32_t version)
{
    uint32_t startPos = stream.GetPos();

    if (stream.GetSize() - startPos >= 4)
    {
        uint32_t magic = stream.ReadUint32();

        if (magic == sMesh3dExtraMagic)
        {
            bool hasInlineMat = stream.ReadBool();

            if (hasInlineMat)
            {
                MaterialLite* mat = NewTransientAsset<MaterialLite>();
                mat->LoadLiteParams(stream, version);
                mat->Create();
                mMaterialOverride = mat;
            }
        }
        else
        {
            stream.SetPos(startPos);

            // Handle legacy v18 format (bool-first, no magic) from early dev builds.
            // Those scenes were saved with a version bump to 18 that wrote a bare bool
            // before the material data, without the magic prefix.
            if (version >= 18)
            {
                bool hasInlineMat = stream.ReadBool();

                if (hasInlineMat)
                {
                    MaterialLite* mat = NewTransientAsset<MaterialLite>();
                    mat->LoadLiteParams(stream, version);
                    mat->Create();
                    mMaterialOverride = mat;
                }
            }
        }
    }

    // Fallback for old scenes saved before inline material support:
    // Recreate the skybox material if this looks like a skybox node that lost its transient material.
    if (mMaterialOverride.Get() == nullptr && GetName() == "Skybox")
    {
        MaterialLite* skyMat = MaterialLite::New(GetMaterial());
        skyMat->SetCullMode(CullMode::Front);
        skyMat->SetDepthTestDisabled(true);
        skyMat->SetSortPriority(-1000);
        mMaterialOverride = skyMat;
    }
}

bool Mesh3D::IsStaticMesh3D() const
{
    return false;
}

bool Mesh3D::IsSkeletalMesh3D() const
{
    return false;
}

bool Mesh3D::IsInstancedMesh3D() const
{
    return false;
}

bool Mesh3D::IsShadowMesh3D() const
{
    return false;
}

Material* Mesh3D::GetMaterialOverride()
{
    return mMaterialOverride.Get<Material>();
}

void Mesh3D::SetMaterialOverride(Material* material)
{
    mMaterialOverride = material;
}

Material* Mesh3D::InstantiateMaterial()
{
    Material* mat = GetMaterial();
    Material* newInst = nullptr;
    if (mat == nullptr || mat->IsLite())
    {
        newInst = MaterialLite::New(mat);
    }
    else
    {
        newInst = MaterialInstance::New(mat);
    }
    SetMaterialOverride(newInst);
    return newInst;
}

bool Mesh3D::IsBillboard() const
{
    return mBillboard;
}

void Mesh3D::SetBillboard(bool billboard)
{
    mBillboard = billboard;
}

glm::mat4 Mesh3D::ComputeBillboardTransform()
{
    Camera3D* camera = GetWorld()->GetActiveCamera();
    glm::quat camQuat = camera->GetWorldRotationQuat();

    glm::mat4 transform = glm::mat4(1);

    glm::vec3 pos = GetWorldPosition();
    glm::vec3 scale = GetWorldScale();
    glm::quat rot = GetRotationQuat(); // Not absolute

    transform = glm::translate(transform, pos);
    transform *= glm::toMat4(camQuat);
    transform *= glm::toMat4(rot);
    transform = glm::scale(transform, scale);

    return transform;
}

glm::mat4 Mesh3D::GetRenderTransform()
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

DrawData Mesh3D::GetDrawData()
{
    DrawData data = {};
    Material* material = GetMaterial();

    data.mNode = this;
    data.mMaterial = material;
    data.mBlendMode = material ? material->GetBlendMode() : BlendMode::Opaque;
    data.mPosition = GetWorldPosition();
    data.mBounds = GetBounds();
    data.mSortPriority = material ? material->GetSortPriority() : 0;
    data.mDepthless = material ? material->IsDepthTestDisabled() : false;

    return data;
}
