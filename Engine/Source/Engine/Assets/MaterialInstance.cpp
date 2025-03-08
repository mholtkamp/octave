#include "Assets/MaterialInstance.h"
#include "AssetManager.h"

#include "MaterialBase.h"

FORCE_LINK_DEF(MaterialInstance);
DEFINE_ASSET(MaterialInstance);


bool MaterialInstance::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    bool success = false;

#if EDITOR
    MaterialInstance* matInst = static_cast<MaterialInstance*>(prop->mOwner);

    if (prop->mName == "Base Material")
    {
        MaterialBase* baseMaterial = (*(Asset**)newValue)->As<MaterialBase>();
        matInst->SetBaseMaterial(baseMaterial);
        success = true;
    }
#endif

    return success;
}

MaterialInstance* MaterialInstance::New(Material* src)
{
    MaterialInstance* ret = NewTransientAsset<MaterialInstance>();
    if (src != nullptr)
    {
        MaterialBase* baseMaterial = src->IsBase() ? (MaterialBase*)src : nullptr;

        if (!baseMaterial)
        {
            MaterialInstance* srcInst = src->IsInstance() ? (MaterialInstance*)src : nullptr;
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
    stream.WriteAsset(mBaseMaterial);
}

void MaterialInstance::LoadStream(Stream& stream, Platform platform)
{
    Material::LoadStream(stream, platform);
    stream.ReadAsset(mBaseMaterial);
}


void MaterialInstance::GatherProperties(std::vector<Property>& outProps)
{
    Material::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Asset, "Base Material", this, &mBaseMaterial, 1, HandlePropChange, int32_t(MaterialBase::GetStaticType())));
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

bool MaterialInstance::IsInstance() const
{
    return true;
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
    MaterialBase* base = mBaseMaterial.Get<MaterialBase>();

    if (base != nullptr)
    {
        std::vector<ShaderParameter> origParams = mParameters;
        std::vector<ShaderParameter> baseParams = base->GetParameters();

        mParameters = baseParams;

        Material::OverwriteShaderParameters(mParameters, origParams);
    }
    else
    {
        mParameters.clear();
    }
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
