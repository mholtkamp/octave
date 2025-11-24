#include "Assets/MaterialLite.h"

#include "Assets/Texture.h"
#include "Assets/MaterialBase.h"
#include "Assets/MaterialInstance.h"

#include "Renderer.h"
#include "AssetManager.h"
#include "Constants.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/Camera3d.h"

#include "Graphics/Graphics.h"

#if EDITOR
#include "EditorImgui.h"
#endif

extern const char* gBlendModeStrings[];
extern const char* gCullModeStrings[];

extern const char* gShadingModelStrings[] =
{
    "Unlit",
    "Lit",
    "Toon"
};
static_assert(int32_t(ShadingModel::Count) == 3, "Need to update string conversion table");

extern const char* gVertexColorModeStrings[] =
{
    "None",
    "Modulate",
    "TextureBlend"
};
static_assert(int32_t(VertexColorMode::Count) == 3, "Need to update string conversion table");

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

FORCE_LINK_DEF(MaterialLite);
DEFINE_ASSET(MaterialLite);

bool MaterialLite::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    MaterialLite* material = static_cast<MaterialLite*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Texture 0")
    {
        material->SetTexture(0, *(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Texture 1")
    {
        material->SetTexture(1, *(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Texture 2")
    {
        material->SetTexture(2, *(Texture**)newValue);
        success = true;
    }
    else if (prop->mName == "Texture 3")
    {
        material->SetTexture(3, *(Texture**)newValue);
        success = true;
    }

    HandleAssetPropChange(datum, index, newValue);

    return success;
}

MaterialLite* MaterialLite::New(Material* src)
{
    MaterialLite* ret = NewTransientAsset<MaterialLite>();

    if (src != nullptr)
    {
        if (src->IsLite())
        {
            MaterialLite* srcLite = (MaterialLite*)src;
            ret->SetLiteParams(srcLite->GetLiteParams());
        }
        else
        {
            // Find first texture and set it as the Texture0 lite param.
            Texture* tex = nullptr;
            const std::vector<ShaderParameter>& srcParams = src->GetParameters();
            for (uint32_t i = 0; i < srcParams.size(); ++i)
            {
                if (srcParams[i].mType == ShaderParameterType::Texture)
                {
                    tex = srcParams[i].mTextureValue.Get<Texture>();
                    break;
                }
            }

            if (tex != nullptr)
            {
                ret->SetTexture(0, tex);
            }
        }
    }

    ret->Create();

    return ret;
}

MaterialLite::MaterialLite()
{
    mType = MaterialLite::GetStaticType();
}

MaterialLite::~MaterialLite()
{

}

void MaterialLite::LoadStream(Stream& stream, Platform platform)
{
    Material::LoadStream(stream, platform);

    mLiteParams.mShadingModel = ShadingModel(stream.ReadUint32());
    mLiteParams.mBlendMode = BlendMode(stream.ReadUint32());
    mLiteParams.mVertexColorMode = VertexColorMode(stream.ReadUint32());

    if (mVersion < ASSET_VERSION_MATERIAL_LITE_TEXTURE_COUNT)
    {
        mLiteParams.mNumTextures = MATERIAL_LITE_MAX_TEXTURES;
    }
    else
    {
        mLiteParams.mNumTextures = stream.ReadUint32();
    }

    for (uint32_t i = 0; i < MATERIAL_LITE_MAX_TEXTURES; ++i)
    {
        stream.ReadAsset(mLiteParams.mTextures[i]);
        mLiteParams.mUvMaps[i] = stream.ReadUint8();
        mLiteParams.mTevModes[i] = (TevMode)stream.ReadUint8();
    }

    for (uint32_t i = 0; i < MAX_UV_MAPS; ++i)
    {
        mLiteParams.mUvOffsets[i] = stream.ReadVec2();
        mLiteParams.mUvScales[i] = stream.ReadVec2();
    }

    mLiteParams.mColor = stream.ReadVec4();
    mLiteParams.mFresnelColor = stream.ReadVec4();
    mLiteParams.mFresnelPower = stream.ReadFloat();
    mLiteParams.mEmission = stream.ReadFloat();
    mLiteParams.mWrapLighting = stream.ReadFloat();
    mLiteParams.mSpecular = stream.ReadFloat();
    mLiteParams.mToonSteps = stream.ReadUint32();
    mLiteParams.mOpacity = stream.ReadFloat();
    mLiteParams.mMaskCutoff = stream.ReadFloat();
    mLiteParams.mShininess = stream.ReadFloat();
    mLiteParams.mSortPriority = stream.ReadInt32();
    mLiteParams.mDisableDepthTest = stream.ReadBool();
    mLiteParams.mFresnelEnabled = stream.ReadBool();
    mLiteParams.mApplyFog = stream.ReadBool();
    mLiteParams.mCullMode = (CullMode)stream.ReadUint8();
}

void MaterialLite::SaveStream(Stream& stream, Platform platform)
{
    Material::SaveStream(stream, platform);

    stream.WriteUint32(uint32_t(mLiteParams.mShadingModel));
    stream.WriteUint32(uint32_t(mLiteParams.mBlendMode));
    stream.WriteUint32(uint32_t(mLiteParams.mVertexColorMode));

    stream.WriteUint32(mLiteParams.mNumTextures);

    for (uint32_t i = 0; i < MATERIAL_LITE_MAX_TEXTURES; ++i)
    {
        stream.WriteAsset(mLiteParams.mTextures[i]);
        stream.WriteUint8(mLiteParams.mUvMaps[i]);
        stream.WriteUint8((uint8_t)mLiteParams.mTevModes[i]);
    }

    for (uint32_t i = 0; i < MAX_UV_MAPS; ++i)
    {
        stream.WriteVec2(mLiteParams.mUvOffsets[i]);
        stream.WriteVec2(mLiteParams.mUvScales[i]);
    }

    stream.WriteVec4(mLiteParams.mColor);
    stream.WriteVec4(mLiteParams.mFresnelColor);
    stream.WriteFloat(mLiteParams.mFresnelPower);
    stream.WriteFloat(mLiteParams.mEmission);
    stream.WriteFloat(mLiteParams.mWrapLighting);
    stream.WriteFloat(mLiteParams.mSpecular);
    stream.WriteUint32(mLiteParams.mToonSteps);
    stream.WriteFloat(mLiteParams.mOpacity);
    stream.WriteFloat(mLiteParams.mMaskCutoff);
    stream.WriteFloat(mLiteParams.mShininess);
    stream.WriteInt32(mLiteParams.mSortPriority);
    stream.WriteBool(mLiteParams.mDisableDepthTest);
    stream.WriteBool(mLiteParams.mFresnelEnabled);
    stream.WriteBool(mLiteParams.mApplyFog);
    stream.WriteUint8((uint8_t)mLiteParams.mCullMode);
}

void MaterialLite::Create()
{
    Material::Create();

    Renderer* renderer = Renderer::Get();

    for (uint32_t i = 0; i < MATERIAL_LITE_MAX_TEXTURES; ++i)
    {
        if (mLiteParams.mTextures[i].Get() == nullptr)
        {
            mLiteParams.mTextures[i] = renderer->mWhiteTexture;
        }
    }

    GFX_CreateMaterialResource(this);
}

void MaterialLite::Destroy()
{
    Material::Destroy();

    GFX_DestroyMaterialResource(this);

    for (uint32_t i = 0; i < MATERIAL_LITE_MAX_TEXTURES; ++i)
    {
        mLiteParams.mTextures[i] = nullptr;
    }
}

bool MaterialLite::Import(const std::string& path, ImportOptions* options)
{
    return Material::Import(path, options);
}

void MaterialLite::GatherProperties(std::vector<Property>& outProps)
{
    Material::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Integer, "Shading Model", this, &mLiteParams.mShadingModel, 1, HandlePropChange, NULL_DATUM, int32_t(ShadingModel::Count), gShadingModelStrings));
    outProps.push_back(Property(DatumType::Integer, "Blend Mode", this, &mLiteParams.mBlendMode, 1, HandlePropChange, NULL_DATUM, int32_t(BlendMode::Count), gBlendModeStrings));
    outProps.push_back(Property(DatumType::Integer, "Vertex Color Mode", this, &mLiteParams.mVertexColorMode, 1, HandlePropChange, NULL_DATUM, int32_t(VertexColorMode::Count), gVertexColorModeStrings));
    outProps.push_back(Property(DatumType::Integer, "Num Textures", this, &mLiteParams.mNumTextures, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Asset, "Texture 0", this, &mLiteParams.mTextures[0], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 0", this, &mLiteParams.mUvMaps[0], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "TEV Mode 0", this, &mLiteParams.mTevModes[0], 1, HandlePropChange, NULL_DATUM, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 1", this, &mLiteParams.mTextures[1], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 1", this, &mLiteParams.mUvMaps[1], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "TEV Mode 1", this, &mLiteParams.mTevModes[1], 1, HandlePropChange, NULL_DATUM, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 2", this, &mLiteParams.mTextures[2], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 2", this, &mLiteParams.mUvMaps[2], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "TEV Mode 2", this, &mLiteParams.mTevModes[2], 1, HandlePropChange, NULL_DATUM, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Asset, "Texture 3", this, &mLiteParams.mTextures[3], 1, HandlePropChange, int32_t(Texture::GetStaticType())));
    outProps.push_back(Property(DatumType::Byte, "UV Map 3", this, &mLiteParams.mUvMaps[3], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "TEV Mode 3", this, &mLiteParams.mTevModes[3], 1, HandlePropChange, NULL_DATUM, int32_t(TevMode::Count), sTevModeStrings));
    outProps.push_back(Property(DatumType::Vector2D, "UV Offset 0", this, &mLiteParams.mUvOffsets[0], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Scale 0", this, &mLiteParams.mUvScales[0], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Offset 1", this, &mLiteParams.mUvOffsets[1], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Vector2D, "UV Scale 1", this, &mLiteParams.mUvScales[1], 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Color", this, &mLiteParams.mColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Opacity", this, &mLiteParams.mOpacity, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Mask Cutoff", this, &mLiteParams.mMaskCutoff, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Sort Priority", this, &mLiteParams.mSortPriority, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Disable Depth Test", this, &mLiteParams.mDisableDepthTest, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Fresnel", this, &mLiteParams.mFresnelEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Fresnel Color", this, &mLiteParams.mFresnelColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Fresnel Power", this, &mLiteParams.mFresnelPower, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Specular", this, &mLiteParams.mSpecular, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Shininess", this, &mLiteParams.mShininess, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Integer, "Toon Steps", this, &mLiteParams.mToonSteps, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Emission", this, &mLiteParams.mEmission, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Wrap Lighting", this, &mLiteParams.mWrapLighting, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Apply Fog", this, &mLiteParams.mApplyFog, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Byte, "Cull Mode", this, &mLiteParams.mCullMode, 1, HandlePropChange, NULL_DATUM, int32_t(CullMode::Count), gCullModeStrings));
}

glm::vec4 MaterialLite::GetTypeColor()
{
    return glm::vec4(0.4f, 0.95f, 0.25f, 1.0f);
}

const char* MaterialLite::GetTypeName()
{
    return "MaterialLite";
}

bool MaterialLite::DrawCustomProperty(Property& prop)
{
#if EDITOR
    if (prop.mName == "Num Textures")
    {
        ImGui::Text("Num Textures");
        ImGui::SliderInt("##NumTexSlide", (int*)&mLiteParams.mNumTextures, 0, 4);
        return true;
    }
    else if (prop.mName.find("Texture") != std::string::npos)
    {
        uint32_t texIdx = 0;
        bool hideProp = false;
        if (prop.mName == "Texture 0")
        {
            hideProp = (mLiteParams.mNumTextures < 1);
            texIdx = 0;
        }
        else if (prop.mName == "Texture 1")
        {
            hideProp = (mLiteParams.mNumTextures < 2);
            texIdx = 1;
        }
        else if (prop.mName == "Texture 2")
        {
            hideProp = (mLiteParams.mNumTextures < 3);
            texIdx = 2;
        }
        else if (prop.mName == "Texture 3")
        {
            hideProp = (mLiteParams.mNumTextures < 4);
            texIdx = 3;
        }

        if (!hideProp)
        {
            // Draw a 
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();

            float red = glm::mix(75.0f, 135.0f, texIdx / 3.0f);

            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(cursorPos.x - 50.0f, cursorPos.y),
                ImVec2(cursorPos.x + 210.0f + 50.0f, cursorPos.y + 120.0f),
                //(texIdx % 2 == 0) ? IM_COL32(25, 128, 120, 255) : IM_COL32(12, 64, 60, 255)
                IM_COL32(int32_t(red + 0.5f), 32, 32, 255)
            );
        }

        return hideProp;
    }
    else if (prop.mName.find("UV Map") != std::string::npos)
    {
        if (prop.mName == "UV Map 0")
            return (mLiteParams.mNumTextures < 1);
        else if (prop.mName == "UV Map 1")
            return (mLiteParams.mNumTextures < 2);
        else if (prop.mName == "UV Map 2")
            return (mLiteParams.mNumTextures < 3);
        else if (prop.mName == "UV Map 3")
            return (mLiteParams.mNumTextures < 4);

        return true;
    }
    else if (prop.mName.find("TEV Mode") != std::string::npos)
    {
        if (prop.mName == "TEV Mode 0")
            return (mLiteParams.mNumTextures < 1);
        else if (prop.mName == "TEV Mode 1")
            return (mLiteParams.mNumTextures < 2);
        else if (prop.mName == "TEV Mode 2")
            return (mLiteParams.mNumTextures < 3);
        else if (prop.mName == "TEV Mode 3")
            return (mLiteParams.mNumTextures < 4);

        return true;
    }
#endif

    return false;
}

bool MaterialLite::IsLite() const
{
    return true;
}

const MaterialLiteParams& MaterialLite::GetLiteParams() const
{
    return mLiteParams;
}

void MaterialLite::SetLiteParams(const MaterialLiteParams& params)
{
    mLiteParams = params;
}

void MaterialLite::SetTexture(uint32_t slot, Texture* texture)
{
    if (slot >= MATERIAL_LITE_MAX_TEXTURES)
    {
        LogWarning("Invalid texture slot in MaterialLite::SetTexture()");
        return;
    }

    if (mLiteParams.mTextures[slot].Get<Texture>() != texture)
    {
        mLiteParams.mTextures[slot] = texture;
    }
}

Texture* MaterialLite::GetTexture(uint32_t slot)
{
    if (slot >= uint32_t(mLiteParams.mNumTextures) || slot >= MATERIAL_LITE_MAX_TEXTURES)
        return Renderer::Get()->mWhiteTexture.Get<Texture>();

    return mLiteParams.mTextures[slot].Get<Texture>();
}

ShadingModel MaterialLite::GetShadingModel() const
{
    return mLiteParams.mShadingModel;
}

void MaterialLite::SetShadingModel(ShadingModel shadingModel)
{
    mLiteParams.mShadingModel = shadingModel;
}

BlendMode MaterialLite::GetBlendMode() const
{
    return mLiteParams.mBlendMode;
}

void MaterialLite::SetBlendMode(BlendMode blendMode)
{
    mLiteParams.mBlendMode = blendMode;
}

VertexColorMode MaterialLite::GetVertexColorMode() const
{
    return mLiteParams.mVertexColorMode;
}

void MaterialLite::SetVertexColorMode(VertexColorMode mode)
{
    mLiteParams.mVertexColorMode = mode;
}

glm::vec2 MaterialLite::GetUvOffset(int32_t uvIndex) const
{
    if (uvIndex < 0 || uvIndex >= MAX_UV_MAPS)
    {
        //LogWarning("Out of range UV index");
        uvIndex = glm::clamp<int32_t>(uvIndex, 0, MAX_UV_MAPS - 1);
    }

    return mLiteParams.mUvOffsets[uvIndex];
}

void MaterialLite::SetUvOffset(glm::vec2 offset, int32_t uvIndex)
{
    if (uvIndex < 0 || uvIndex >= MAX_UV_MAPS)
    {
        //LogWarning("Out of range UV index");
        uvIndex = glm::clamp<int32_t>(uvIndex, 0, MAX_UV_MAPS - 1);
    }

    mLiteParams.mUvOffsets[uvIndex] = offset;
}

glm::vec2 MaterialLite::GetUvScale(int32_t uvIndex) const
{
    if (uvIndex < 0 || uvIndex >= MAX_UV_MAPS)
    {
        //LogWarning("Out of range UV index");
        uvIndex = glm::clamp<int32_t>(uvIndex, 0, MAX_UV_MAPS - 1);
    }

    return mLiteParams.mUvScales[uvIndex];
}

void MaterialLite::SetUvScale(glm::vec2 scale, int32_t uvIndex)
{
    if (uvIndex < 0 || uvIndex >= MAX_UV_MAPS)
    {
        //LogWarning("Out of range UV index");
        uvIndex = glm::clamp<int32_t>(uvIndex, 0, MAX_UV_MAPS - 1);
    }

    mLiteParams.mUvScales[uvIndex] = scale;
}

glm::vec4 MaterialLite::GetColor() const
{
    return mLiteParams.mColor;
}

void MaterialLite::SetColor(const glm::vec4& color)
{
    mLiteParams.mColor = color;
}

glm::vec4 MaterialLite::GetFresnelColor() const
{
    return mLiteParams.mFresnelColor;
}

void MaterialLite::SetFresnelColor(const glm::vec4& color)
{
    mLiteParams.mFresnelColor = color;
}

float MaterialLite::GetEmission() const
{
    return mLiteParams.mEmission;
}

void MaterialLite::SetEmission(float emission)
{
    mLiteParams.mEmission = emission;
}

float MaterialLite::GetWrapLighting() const
{
    return mLiteParams.mWrapLighting;
}

void MaterialLite::SetWrapLighting(float wrapLighting)
{
    mLiteParams.mWrapLighting = wrapLighting;
}

float MaterialLite::GetFresnelPower() const
{
    return mLiteParams.mFresnelPower;
}

void MaterialLite::SetFresnelPower(float power)
{
    mLiteParams.mFresnelPower = power;
}

float MaterialLite::GetSpecular() const
{
    return mLiteParams.mSpecular;
}

void MaterialLite::SetSpecular(float specular)
{
    mLiteParams.mSpecular = specular;
}

uint32_t MaterialLite::GetToonSteps() const
{
    return mLiteParams.mToonSteps;
}

void MaterialLite::SetToonSteps(uint32_t steps)
{
    mLiteParams.mToonSteps = steps;
}

float MaterialLite::GetOpacity() const
{
    return mLiteParams.mOpacity;
}

void MaterialLite::SetOpacity(float opacity)
{
    mLiteParams.mOpacity = opacity;
}

float MaterialLite::GetMaskCutoff() const
{
    return mLiteParams.mMaskCutoff;
}

void MaterialLite::SetMaskCutoff(float cutoff)
{
    mLiteParams.mMaskCutoff = cutoff;
}

float MaterialLite::GetShininess() const
{
    return mLiteParams.mShininess;
}

void MaterialLite::SetShininess(float shininess)
{
    mLiteParams.mShininess = shininess;
}

int32_t MaterialLite::GetSortPriority() const
{
    return mLiteParams.mSortPriority;
}

void MaterialLite::SetSortPriority(int32_t priority)
{
    mLiteParams.mSortPriority = priority;
}

bool MaterialLite::IsDepthTestDisabled() const
{
    return mLiteParams.mDisableDepthTest;
}

void MaterialLite::SetDepthTestDisabled(bool depthTest)
{
    mLiteParams.mDisableDepthTest = depthTest;
}

bool MaterialLite::IsFresnelEnabled() const
{
    return mLiteParams.mFresnelEnabled;
}

void MaterialLite::SetFresnelEnabled(bool enable)
{
    mLiteParams.mFresnelEnabled = enable;
}

bool MaterialLite::ShouldApplyFog() const
{
    return mLiteParams.mApplyFog;
}

void MaterialLite::SetApplyFog(bool applyFog)
{
    mLiteParams.mApplyFog = applyFog;
}

CullMode MaterialLite::GetCullMode() const
{
    return mLiteParams.mCullMode;
}

void MaterialLite::SetCullMode(CullMode cullMode)
{
    mLiteParams.mCullMode = cullMode;
}

uint32_t MaterialLite::GetUvMap(uint32_t textureSlot)
{
    OCT_ASSERT(textureSlot < MATERIAL_LITE_MAX_TEXTURES);
    if (textureSlot < uint32_t(mLiteParams.mNumTextures) && textureSlot < MATERIAL_LITE_MAX_TEXTURES)
    {
        return mLiteParams.mUvMaps[textureSlot];
    }

    return 0;
}

void MaterialLite::SetUvMap(uint32_t textureSlot, uint32_t uvMapIndex)
{
    OCT_ASSERT(textureSlot < MATERIAL_LITE_MAX_TEXTURES);
    OCT_ASSERT(uvMapIndex < MAX_UV_MAPS);
    if (textureSlot < MATERIAL_LITE_MAX_TEXTURES &&
        uvMapIndex < MAX_UV_MAPS)
    {
        mLiteParams.mUvMaps[textureSlot] = uvMapIndex;
    }
}

TevMode MaterialLite::GetTevMode(uint32_t textureSlot)
{
    OCT_ASSERT(textureSlot < MATERIAL_LITE_MAX_TEXTURES);
    if (textureSlot < uint32_t(mLiteParams.mNumTextures) && textureSlot < MATERIAL_LITE_MAX_TEXTURES)
    {
        return mLiteParams.mTevModes[textureSlot];
    }

    return TevMode::Pass;
}

void MaterialLite::SetTevMode(uint32_t textureSlot, TevMode mode)
{
    OCT_ASSERT(textureSlot < MATERIAL_LITE_MAX_TEXTURES);
    OCT_ASSERT(mode != TevMode::Count);

    if (textureSlot < MATERIAL_LITE_MAX_TEXTURES &&
        mode != TevMode::Count)
    {
        mLiteParams.mTevModes[textureSlot] = mode;
    }
}
