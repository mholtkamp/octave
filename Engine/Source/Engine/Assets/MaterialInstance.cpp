#include "Assets/MaterialInstance.h"
#include "AssetManager.h"

#include "MaterialBase.h"

FORCE_LINK_DEF(MaterialInstance);
DEFINE_ASSET(MaterialInstance);

MaterialInstance* MaterialInstance::New(Material* src)
{
    MaterialInstance* ret = NewTransientAsset<MaterialInstance>();
    if (src != nullptr)
    {
        MaterialBase* baseMaterial = src->IsMaterialBase() ? (MaterialBase*)src : nullptr;

        if (!baseMaterial)
        {
            MaterialInstance* srcInst = src->IsMaterialInstance() ? (MaterialInstance*)src : nullptr;
            baseMaterial = srcInst->GetBaseMaterial();
        }

        ret->SetBaseMaterial(baseMaterial);
        ret->mParameters = src->GetParameters();
    }
    ret->Create();

    return ret;
}

MaterialInstance::MaterialInstance()
{
    mType = MaterialInstance::GetStaticType();

}

MaterialInstance::~MaterialInstance()
{

}


void MaterialInstance::SaveStream(Stream& stream, Platform platform)
{
    Material::SaveStream(stream, platform);
}

void MaterialInstance::LoadStream(Stream& stream, Platform platform)
{
    Material::LoadStream(stream, platform);
}


void MaterialInstance::GatherProperties(std::vector<Property>& outProps)
{
    Material::GatherProperties(outProps);
}

void MaterialInstance::Create()
{
    Material::Create();
}

void MaterialInstance::Destroy()
{
    Material::Destroy();
}

void MaterialInstance::Import(const std::string& path, ImportOptions* options)
{
    Material::Import(path, options);
}

glm::vec4 MaterialInstance::GetTypeColor()
{
    return glm::vec4(0.2f, 0.85f, 0.35f, 1.0f);
}

const char* MaterialInstance::GetTypeName()
{
    return "MaterialInstance";
}

MaterialBase* MaterialInstance::GetBaseMaterial() const
{
    return mBaseMaterial.Get<MaterialBase>();
}

void MaterialInstance::SetBaseMaterial(MaterialBase* material)
{
    if (GetBaseMaterial() != material)
    {
        mBaseMaterial = material;
        LinkToBase();
    }
}

void MaterialInstance::LinkToBase()
{
#error Implement LinkToBase(). Need to grab parent's
}

BlendMode MaterialInstance::GetBlendMode() const
{
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();
    return base ? base->GetBlendMode() : BlendMode::Count;
}

float MaterialInstance::GetMaskCutoff() const
{
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();
    return base ? base->GetMaskCutoff() : 0.5f;
}

int32_t MaterialInstance::GetSortPriority() const
{
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();
    return base ? base->GetSortPriority() : 0;
}

bool MaterialInstance::IsDepthTestDisabled() const
{
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();
    return base ? base->IsDepthTestDisabled() : false;
}

bool MaterialInstance::ShouldApplyFog() const
{
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();
    return base ? base->ShouldApplyFog() : true;
}

CullMode MaterialInstance::GetCullMode() const
{
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();
    return base ? base->GetCullMode() : CullMode::Back;
}
