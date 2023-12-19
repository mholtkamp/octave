#include "Assets/MaterialBase.h"
#include "Assets/MaterialInstance.h"
#include "Assets/Texture.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Constants.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Camera3d.h"

#include "Graphics/Graphics.h"

const char* gBlendModeStrings[] =
{
    "Opaque",
    "Masked",
    "Transparent",
    "Additive"
};
static_assert(int32_t(BlendMode::Count) == 4, "Need to update string conversion table");

const char* gCullModeStrings[] =
{
    "None",
    "Back",
    "Front"
};
static_assert(int32_t(CullMode::Count) == 3, "Need to update string conversion table");

FORCE_LINK_DEF(MaterialBase);
DEFINE_ASSET(MaterialBase);

bool MaterialBase::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    MaterialBase* materialBase = static_cast<MaterialBase*>(prop->mOwner);
    bool success = false;

    materialBase->MarkDirty();
    materialBase->MarkStale();

    return success;
}

MaterialBase::MaterialBase()
{
    mType = MaterialBase::GetStaticType();

    MarkDirty();
}

MaterialBase::~MaterialBase()
{

}

void MaterialBase::LoadStream(Stream& stream, Platform platform)
{
    Material::LoadStream(stream, platform);

    stream.ReadString(mShader);
    mBlendMode = (BlendMode)stream.ReadUint32();
    mMaskCutoff = stream.ReadFloat();
    mSortPriority = stream.ReadInt32();
    mDisableDepthTest = stream.ReadBool();
    mApplyFog = stream.ReadBool();
    mCullMode = (CullMode)stream.ReadUint8();
}

void MaterialBase::SaveStream(Stream& stream, Platform platform)
{
    Material::SaveStream(stream, platform);

    stream.WriteString(mShader);
    stream.WriteUint32((uint32_t)mBlendMode);
    stream.WriteFloat(mMaskCutoff);
    stream.WriteInt32(mSortPriority);
    stream.WriteBool(mDisableDepthTest);
    stream.WriteBool(mApplyFog);
    stream.WriteUint8((uint8_t)mCullMode);
}

void MaterialBase::Create()
{
    Material::Create();

    Renderer* renderer = Renderer::Get();

    // Do we want to default all textures to White? Or just leave null.
    // If we do, this should probably be movedup to Material::Create().
#if 0
    for (uint32_t i = 0; i < mParameters.size(); ++i)
    {
        if (mParameters[i].mType == ShaderParameterType::Texture;
            mParameters[i].mTextureValue == nullptr)
        {
            mParameters[i].mTextureValue = renderer->mWhiteTexture;
        }
    }
#endif

    GFX_CreateMaterialResource(this);
    MarkDirty();
}

void MaterialBase::Destroy()
{
    Material::Destroy();

    GFX_DestroyMaterialResource(this);
}

void MaterialBase::Import(const std::string& path, ImportOptions* options)
{
    Material::Import(path, options);
}

void MaterialBase::GatherProperties(std::vector<Property>& outProps)
{
    Material::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::String, "Shader", this, &mShader, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Blend Mode", this, &mBlendMode, 1, HandlePropChange, 0, int32_t(BlendMode::Count), gBlendModeStrings));
    outProps.push_back(Property(DatumType::Byte, "Cull Mode", this, &mCullMode, 1, HandlePropChange, 0, int32_t(CullMode::Count), gCullModeStrings));
    outProps.push_back(Property(DatumType::Float, "Mask Cutoff", this, &mMaskCutoff, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Sort Priority", this, &mSortPriority, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Disable Depth Test", this, &mDisableDepthTest, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Apply Fog", this, &mApplyFog, 1, HandlePropChange));
}

glm::vec4 MaterialBase::GetTypeColor()
{
    return glm::vec4(0.2f, 1.0f, 0.25f, 1.0f);
}

const char* MaterialBase::GetTypeName()
{
    return "MaterialBase";
}

bool MaterialBase::IsBase() const
{
    return true;
}

void MaterialBase::Compile()
{
#error Perform full compilation process
    // Compile MaterialBase:
    // (1) Get user shader code from .glsl file.
    // (2) Get template vert + frag code
    // (3) Create the combined file (Header + User + Template)
    // (4) Compile with shaderc / glslc to get the spirv (save to members)
    // (5) Fillout parameters using SpirvReflect
    // (6) Call GFX_BuildMaterial() to create pipelines/descriptor layout.

#if EDITOR
    // Relink any loaded material instances that use this base.
    std::unordered_map<std::string, AssetStub*>& assetMap = AssetManager::Get()->GetAssetMap();

    for (auto& pair : assetMap)
    {
        if (pair.second->mAsset && 
            pair.second->mType == MaterialInstance::GetStaticType())
        {
            MaterialInstance* inst = pair.second->mAsset->As<MaterialInstance>();
            if (inst && inst->GetBaseMaterial() == this)
            {
                inst->LinkToBase();
            }
        }
    }
#endif
}

void MaterialBase::MarkStale()
{
#if EDITOR
    mCompilationStale = true;
#endif
}

BlendMode MaterialBase::GetBlendMode() const
{
    return mBlendMode;
}

void MaterialBase::SetBlendMode(BlendMode blendMode)
{
    if (mBlendMode != blendMode)
    {
        mBlendMode = blendMode;
        MarkStale();
    }
}

float MaterialBase::GetMaskCutoff() const
{
    return mMaskCutoff;
}

void MaterialBase::SetMaskCutoff(float cutoff)
{
    if (mMaskCutoff != cutoff)
    {
        mMaskCutoff = cutoff;
        MarkStale();
    }
}

int32_t MaterialBase::GetSortPriority() const
{
    return mSortPriority;
}

void MaterialBase::SetSortPriority(int32_t priority)
{
    if (mSortPriority != priority)
    {
        mSortPriority = priority;
        MarkStale();
    }
}

bool MaterialBase::IsDepthTestDisabled() const
{
    return mDisableDepthTest;
}

void MaterialBase::SetDepthTestDisabled(bool depthTestDis)
{
    if (mDisableDepthTest != depthTestDis)
    {
        mDisableDepthTest = depthTestDis;
        MarkStale();
    }
}

bool MaterialBase::ShouldApplyFog() const
{
    return mApplyFog;
}

void MaterialBase::SetApplyFog(bool applyFog)
{
    if (mApplyFog != applyFog)
    {
        mApplyFog = applyFog;
        MarkStale();
    }
}

CullMode MaterialBase::GetCullMode() const
{
    return mCullMode;
}

void MaterialBase::SetCullMode(CullMode cullMode)
{
    if (mCullMode != cullMode)
    {
        mCullMode = cullMode;
        MarkStale();
    }
}

