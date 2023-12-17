#include "Assets/MaterialInstance.h"
#include "AssetManager.h"

FORCE_LINK_DEF(MaterialInstance);
DEFINE_ASSET(MaterialInstance);

MaterialInstance* MaterialInstance::New(MaterialInterface* src)
{
    MaterialInstance* ret = NewTransientAsset<MaterialInstance>();
    if (src != nullptr)
    {
        Material* baseMaterial = src->IsBase() ? (Material*)src : nullptr;

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

}

MaterialInstance::~MaterialInstance()
{

}


void MaterialInstance::SaveStream(Stream& stream)
{
    OCT_ASSERT(0);
    // Should not be saved
}

void MaterialInstance::LoadStream(Stream& stream)
{
    OCT_ASSERT(0);
    // Should not be loaded.
}
