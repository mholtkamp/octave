#include "Assets/MaterialInstance.h"
#include "AssetManager.h"

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

}

MaterialInstance::~MaterialInstance()
{

}


void MaterialInstance::SaveStream(Stream& stream, Platform platform)
{

}

void MaterialInstance::LoadStream(Stream& stream, Platform platform)
{

}
