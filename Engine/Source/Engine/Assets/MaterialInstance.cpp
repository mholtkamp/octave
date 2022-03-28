#include "Assets/MaterialInstance.h"
#include "AssetManager.h"

DEFINE_ASSET(MaterialInstance);

MaterialInstance* MaterialInstance::New(const Material* src)
{
    MaterialInstance* ret = new MaterialInstance();
    if (src != nullptr)
    {
        ret->mParams = src->GetParams();
    }
    ret->Create();

    AssetManager::Get()->RegisterTransientAsset(ret);

    return ret;
}

MaterialInstance::MaterialInstance()
{

}

MaterialInstance::~MaterialInstance()
{

}

bool MaterialInstance::IsTransient() const
{
    return true;
}

bool MaterialInstance::IsMaterialInstance() const
{
    return true;
}

void MaterialInstance::SaveStream(Stream& stream)
{
    assert(0);
    // Should not be saved
}

void MaterialInstance::LoadStream(Stream& stream)
{
    assert(0);
    // Should not be loaded.
}
