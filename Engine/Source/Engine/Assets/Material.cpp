#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "Constants.h"
#include "Log.h"
#include "Engine.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/CameraComponent.h"

#include "Graphics/Graphics.h"

static const char* sShadingModelStrings[] =
{
    "Unlit",
    "Lit",
    "Toon"
};
static_assert(int32_t(ShadingModel::Count) == 3, "Need to update string conversion table");


static const char* sBlendModeStrings[] =
{
    "Opaque",
    "Masked",
    "Transparent",
    "Additive"
};
static_assert(int32_t(BlendMode::Count) == 4, "Need to update string conversion table");

static const char* sTevModeStrings[] = 
{
    "Replace",
    "Modulate",
    "Decal",
    "Add",
    "Signed Add",
    "Subtract",
    "Interpolate",
    "Pass"
};
static_assert(int32_t(TevMode::Count) == 8, "Need to update string conversion table");

FORCE_LINK_DEF(Material);
DEFINE_ASSET(Material);

static bool HandlePropChange(Datum* datum, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    assert(prop != nullptr);
    Material* material = static_cast<Material*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Texture 0")
    {
        material->SetTexture(TEXTURE_0, *(Texture**) newValue);
        success = true;
    }
    else if (prop->mName == "Texture 1")
    {
        material->SetTexture(TEXTURE_1, *(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Texture 2")
    {
        material->SetTexture(TEXTURE_2, *(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Texture 3")
    {
        material->SetTexture(TEXTURE_3, *(Texture**)newValue);
        success = true;
    }

    material->MarkDirty();

    return success;
}

Material::Material()
{
    mType = Material::GetStaticType();

    MarkDirty();
}

Material::~Material()
{

}

MaterialResource* Material::GetResource()
{
    return &mResource;
}

void Material::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mParams.mShadingModel = ShadingModel(stream.ReadUint32());
    mParams.mBlendMode = BlendMode(stream.ReadUint32());

    for (uint32_t i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        stream.ReadAsset(mParams.mTextures[i]);
        //mParams.mUvMaps[i] = stream.ReadUint8();
        //mParams.mTevModes[i] = (TevMode) stream.ReadUint8();
    }

    mParams.mUvOffset = stream.ReadVec2();
    mParams.mUvScale = stream.ReadVec2();
    mParams.mColor = stream.ReadVec4();
    mParams.mFresnelColor = stream.ReadVec4();
    mParams.mFresnelPower = stream.ReadFloat();
    mParams.mSpecular = stream.ReadFloat();
    mParams.mToonSteps = stream.ReadUint32();
    mParams.mOpacity = stream.ReadFloat();
    mParams.mMaskCutoff = stream.ReadFloat();
    //mParams.mShininess = stream.ReadFloat();
    mParams.mSortPriority = stream.ReadInt32();
    mParams.mDisableDepthTest = stream.ReadBool();
    //mParams.mFresnelEnabled = stream.ReadBool();
}

void Material::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteUint32(uint32_t(mParams.mShadingModel));
    stream.WriteUint32(uint32_t(mParams.mBlendMode));

    for (uint32_t i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        stream.WriteAsset(mParams.mTextures[i]);
        //stream.WriteUint8(mParams.mUvMaps[i]);
        //stream.WriteUint8((uint8_t)mParams.mTevModes[i]);
    }

    stream.WriteVec2(mParams.mUvOffset);
    stream.WriteVec2(mParams.mUvScale);
    stream.WriteVec4(mParams.mColor);
    stream.WriteVec4(mParams.mFresnelColor);
    stream.WriteFloat(mParams.mFresnelPower);
    stream.WriteFloat(mParams.mSpecular);
    stream.WriteUint32(mParams.mToonSteps);
    stream.WriteFloat(mParams.mOpacity);
    stream.WriteFloat(mParams.mMaskCutoff);
    //stream.WriteFloat(mParams.mShininess);
    stream.WriteInt32(mParams.mSortPriority);
    stream.WriteBool(mParams.mDisableDepthTest);
    //stream.WriteBool(mParams.mFresnelEnabled);
}

void Material::Create()
{
    Asset::Create();

    Renderer* renderer = Renderer::Get();

    for (uint32_t i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        if (mParams.mTextures[i].Get() == nullptr)
        {
            mParams.mTextures[i] = renderer->mWhiteTexture;
        }
    }

    GFX_CreateMaterialResource(this);
    MarkDirty();
}

void Material::Destroy()
{
    Asset::Destroy();

    GFX_DestroyMaterialResource(this);

    for (uint32_t i = 0; i < MATERIAL_MAX_TEXTURES; ++i)
    {
        mParams.mTextures[i] = nullptr;
    }
}

void Material::Import(const std::string& path)
{
    Asset::Import(path);
}

void Material::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Enum, "Shading Model", this, &mParams.mShadingModel, 1, HandlePropChange, 0, int32_t(ShadingModel::Count), sShadingModelStrings));
    outProps.push_back(Property(DatumType::Enum, "Blend Mode", this, &mParams.mBlendMode, 1, HandlePropChange, 0, int32_t(BlendMode::Count), sBlendModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 0", this, &mParams.mTextures[TEXTURE_0], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 0", this, &mParams.mUvMaps[TEXTURE_0], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Enum, "TEV Mode 0", this, &mParams.mTevModes[TEXTURE_0], 1, HandlePropChange, 0, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 1", this, &mParams.mTextures[TEXTURE_1], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 1", this, &mParams.mUvMaps[TEXTURE_1], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Enum, "TEV Mode 1", this, &mParams.mTevModes[TEXTURE_1], 1, HandlePropChange, 0, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 2", this, &mParams.mTextures[TEXTURE_2], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 2", this, &mParams.mUvMaps[TEXTURE_2], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Enum, "TEV Mode 2", this, &mParams.mTevModes[TEXTURE_2], 1, HandlePropChange, 0, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 3", this, &mParams.mTextures[TEXTURE_3], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 3", this, &mParams.mUvMaps[TEXTURE_3], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Enum, "TEV Mode 3", this, &mParams.mTevModes[TEXTURE_3], 1, HandlePropChange, 0, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Vector2D, "UV Offset", this, &mParams.mUvOffset, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Scale", this, &mParams.mUvScale, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Color", this, &mParams.mColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Opacity", this, &mParams.mOpacity, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Mask Cutoff", this, &mParams.mMaskCutoff, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Sort Priority", this, &mParams.mSortPriority, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Disable Depth Test", this, &mParams.mDisableDepthTest, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Fresnel", this, &mParams.mFresnelEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Fresnel Color", this, &mParams.mFresnelColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Fresnel Power", this, &mParams.mFresnelPower, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Specular", this, &mParams.mSpecular, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Shininess", this, &mParams.mShininess, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Toon Steps", this, &mParams.mToonSteps, 1, HandlePropChange));
}

glm::vec4 Material::GetTypeColor()
{
    return glm::vec4(0.2f, 1.0f, 0.25f, 1.0f);
}

const char* Material::GetTypeName()
{
    return "Material";
}

bool Material::IsMaterialInstance() const
{
    return false;
}

void Material::MarkDirty()
{
    for (uint32_t i = 0; i < MAX_FRAMES; ++i)
    {
        mDirty[i] = true;
    }
}

void Material::ClearDirty(uint32_t frameIndex)
{
    mDirty[frameIndex] = false;
}

bool Material::IsDirty(uint32_t frameIndex)
{
    return mDirty[frameIndex];
}

const MaterialParams& Material::GetParams() const
{
    return mParams;
}

void Material::SetParams(const MaterialParams& params)
{
    mParams = params;
    MarkDirty();
}

void Material::SetTexture(TextureSlot slot, Texture* texture)
{
    if (mParams.mTextures[slot].Get<Texture>() != texture)
    {
        mParams.mTextures[slot] = texture;
        MarkDirty();
    }
}

Texture* Material::GetTexture(TextureSlot slot)
{
    return mParams.mTextures[slot].Get<Texture>();
}

ShadingModel Material::GetShadingModel() const
{
    return mParams.mShadingModel;
}

void Material::SetShadingModel(ShadingModel shadingModel)
{
    mParams.mShadingModel = shadingModel;
    MarkDirty();
}

BlendMode Material::GetBlendMode() const
{
    return mParams.mBlendMode;
}

void Material::SetBlendMode(BlendMode blendMode)
{
    mParams.mBlendMode = blendMode;
    MarkDirty();
}

glm::vec2 Material::GetUvOffset() const
{
    return mParams.mUvOffset;
}

void Material::SetUvOffset(glm::vec2 offset)
{
    mParams.mUvOffset = offset;
    MarkDirty();
}

glm::vec2 Material::GetUvScale() const
{
    return mParams.mUvScale;
}

void Material::SetUvScale(glm::vec2 scale)
{
    mParams.mUvScale = scale;
    MarkDirty();
}

glm::vec4 Material::GetColor() const
{
    return mParams.mColor;
}

void Material::SetColor(const glm::vec4& color)
{
    mParams.mColor = color;
    MarkDirty();
}

glm::vec4 Material::GetFresnelColor() const
{
    return mParams.mFresnelColor;
}

void Material::SetFresnelColor(const glm::vec4& color)
{
    mParams.mFresnelColor = color;
    MarkDirty();
}

float Material::GetFresnelPower() const
{
    return mParams.mFresnelPower;
}

void Material::SetFresnelPower(float power)
{
    mParams.mFresnelPower = power;
    MarkDirty();
}

float Material::GetSpecular() const
{
    return mParams.mSpecular;
}

void Material::SetSpecular(float specular)
{
    mParams.mSpecular = specular;
    MarkDirty();
}

uint32_t Material::GetToonSteps() const
{
    return mParams.mToonSteps;
}

void Material::SetToonSteps(uint32_t steps)
{
    mParams.mToonSteps = steps;
    MarkDirty();
}

float Material::GetOpacity() const
{
    return mParams.mOpacity;
}

void Material::SetOpacity(float opacity)
{
    mParams.mOpacity = opacity;
    MarkDirty();
}

float Material::GetMaskCutoff() const
{
    return mParams.mMaskCutoff;
}

void Material::SetMaskCutoff(float cutoff)
{
    mParams.mMaskCutoff = cutoff;
    MarkDirty();
}

float Material::GetShininess() const
{
    return mParams.mShininess;
}

void Material::SetShininess(float shininess)
{
    mParams.mShininess = shininess;
    MarkDirty();
}

int32_t Material::GetSortPriority() const
{
    return mParams.mSortPriority;
}

void Material::SetSortPriority(int32_t priority)
{
    mParams.mSortPriority = priority;
    MarkDirty();
}

bool Material::IsDepthTestDisabled() const
{
    return mParams.mDisableDepthTest;
}

void Material::SetDepthTestDisabled(bool depthTest)
{
    mParams.mDisableDepthTest = depthTest;
    MarkDirty();
}

bool Material::IsFresnelEnabled() const
{
    return mParams.mFresnelEnabled;
}

void Material::SetFresnelEnabled(bool enable)
{
    mParams.mFresnelEnabled = enable;
    MarkDirty();
}

uint32_t Material::GetUvMap(uint32_t textureSlot)
{
    assert(textureSlot < MATERIAL_MAX_TEXTURES);
    if (textureSlot < MATERIAL_MAX_TEXTURES)
    {
        return mParams.mUvMaps[textureSlot];
    }

    return 0;
}

void Material::SetUvMap(uint32_t textureSlot, uint32_t uvMapIndex)
{
    assert(textureSlot < MATERIAL_MAX_TEXTURES);
    assert(uvMapIndex < MAX_UV_MAPS);
    if (textureSlot < MATERIAL_MAX_TEXTURES &&
        uvMapIndex < MAX_UV_MAPS)
    {
        mParams.mUvMaps[textureSlot] = uvMapIndex;
        MarkDirty();
    }
}

TevMode Material::GetTevMode(uint32_t textureSlot)
{
    assert(textureSlot < MATERIAL_MAX_TEXTURES);
    if (textureSlot < MATERIAL_MAX_TEXTURES)
    {
        return mParams.mTevModes[textureSlot];
    }

    return TevMode::Count;
}

void Material::SetTevMode(uint32_t textureSlot, TevMode mode)
{
    assert(textureSlot < MATERIAL_MAX_TEXTURES);
    assert(mode != TevMode::Count);

    if (textureSlot < MATERIAL_MAX_TEXTURES &&
        mode != TevMode::Count)
    {
        mParams.mTevModes[textureSlot] = mode;
        MarkDirty();
    }
}
