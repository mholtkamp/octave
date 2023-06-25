#include "Assets/Level.h"
#include "Stream.h"
#include "Actor.h"
#include "World.h"
#include "Engine.h"
#include "Log.h"
#include "Maths.h"
#include "NetworkManager.h"
#include "Assets/Blueprint.h"
#include "Components/LightComponent.h"

#if EDITOR
#include "EditorState.h"
#endif

#include <algorithm>

FORCE_LINK_DEF(Level);
DEFINE_ASSET(Level);

static const char* sFogDensityStrings[] =
{
    "Linear",
    "Exponential",
};
static_assert(int32_t(FogDensityFunc::Count) == 2, "Need to update string conversion table");

bool Level::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    bool success = false;
#if EDITOR
    Property* prop = static_cast<Property*>(datum);
    Level* level = (Level*)prop->mOwner;

    // Just set value anyway
    datum->SetValueRaw(newValue, index);

    if (GetEditorState()->mActiveLevel == level)
    {
        level->ApplySettings(true);
    }

#endif
    return success;
}

Level::Level()
{
    mType = Level::GetStaticType();
    mEnableRefCount = false;
}

Level::~Level()
{

}

void Level::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mNetLoad = stream.ReadBool();

    mSetAmbientLightColor = stream.ReadBool();
    mSetShadowColor = stream.ReadBool();
    mSetFog = stream.ReadBool();
    mAmbientLightColor = stream.ReadVec4();
    mShadowColor = stream.ReadVec4();
    mFogEnabled = stream.ReadBool();
    mFogColor = stream.ReadVec4();
    mFogDensityFunc = (FogDensityFunc)stream.ReadUint8();
    mFogNear = stream.ReadFloat();
    mFogFar = stream.ReadFloat();

    uint32_t dataSize = stream.ReadUint32();
    mData.resize(dataSize);
    stream.ReadBytes(mData.data(), dataSize);
}

// Save entire world as a level.
void Level::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

#if EDITOR
    stream.WriteBool(mNetLoad);

    stream.WriteBool(mSetAmbientLightColor);
    stream.WriteBool(mSetShadowColor);
    stream.WriteBool(mSetFog);
    stream.WriteVec4(mAmbientLightColor);
    stream.WriteVec4(mShadowColor);
    stream.WriteBool(mFogEnabled);
    stream.WriteVec4(mFogColor);
    stream.WriteUint8(uint8_t(mFogDensityFunc));
    stream.WriteFloat(mFogNear);
    stream.WriteFloat(mFogFar);

    stream.WriteUint32((uint32_t)mData.size());
    stream.WriteBytes(mData.data(), (uint32_t)mData.size());
#endif
}

void Level::Create()
{
    Asset::Create();
}

void Level::Destroy()
{
    Asset::Destroy();
}

void Level::Import(const std::string& path, ImportOptions* options)
{
    LogWarning("Level::Import() not implemented.");
}

void Level::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Bool, "Net Load", this, &mNetLoad));

    // Settings
    outProps.push_back(Property(DatumType::Bool, "Set Ambient Light Color", this, &mSetAmbientLightColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Ambient Light Color", this, &mAmbientLightColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Set Shadow Color", this, &mSetShadowColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Shadow Color", this, &mShadowColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Set Fog", this, &mSetFog, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Bool, "Fog Enabled", this, &mFogEnabled, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Color, "Fog Color", this, &mFogColor, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Byte, "Fog Density", this, &mFogDensityFunc, 1, HandlePropChange, 0, int32_t(FogDensityFunc::Count), sFogDensityStrings));
    outProps.push_back(Property(DatumType::Float, "Fog Near", this, &mFogNear, 1, HandlePropChange));
    outProps.push_back(Property(DatumType::Float, "Fog Far", this, &mFogFar, 1, HandlePropChange));
}

glm::vec4 Level::GetTypeColor()
{
    return glm::vec4(0.9f, 0.8f, 0.2f, 1.0f);
}

const char* Level::GetTypeName()
{
    return "Level";
}

void Level::CaptureWorld(World* world)
{
#if EDITOR
    Stream captureStream;

    const std::vector<Actor*>& actors = world->GetActors();
    uint32_t numActors = 0;

    for (uint32_t i = 0; i < actors.size(); ++i)
    {
        if (ShouldSaveActor(actors[i]))
        {
            ++numActors;
        }
    }
    captureStream.WriteUint32(numActors);

    for (uint32_t i = 0; i < actors.size(); ++i)
    {
        if (ShouldSaveActor(actors[i]))
        {
            bool bp = (actors[i]->GetBlueprintSource() != nullptr);
            captureStream.WriteBool(bp);

            if (bp)
            {
                // Blueprints need to serialize their asset name + override properties
                captureStream.WriteAsset(actors[i]->GetBlueprintSource());

                std::vector<PropertyOverride> propOverrides;
                actors[i]->GatherPropertyOverrides(propOverrides);

                captureStream.WriteUint32((uint32_t)propOverrides.size());
                for (uint32_t p = 0; p < propOverrides.size(); ++p)
                {
                    captureStream.WriteInt32(propOverrides[p].mIndex);
                    propOverrides[p].mProperty.WriteStream(captureStream);
                }
            }
            else
            {
                captureStream.WriteUint32(actors[i]->GetType());
                actors[i]->SaveStream(captureStream);
            }
        }
    }

    mData.resize(captureStream.GetSize());
    memcpy(mData.data(), captureStream.GetData(), captureStream.GetSize());
#endif
}

void Level::LoadIntoWorld(World* world, bool clear, glm::vec3 offset, glm::vec3 rotation)
{
    if (clear)
    {
        GetWorld()->Clear();
    }

    ApplySettings(false);

    Stream stream((const char*)mData.data(), (uint32_t)mData.size());

    uint32_t numActors;
    numActors = stream.ReadUint32();
    LogDebug("Loading Level... %d Actors", numActors);

    glm::vec3 rotRadians = rotation * DEGREES_TO_RADIANS;
    glm::mat4 transform = glm::mat4(1);
    transform = glm::translate(transform, offset);
    transform *= glm::eulerAngleYXZ(rotRadians.y, rotRadians.x, rotRadians.z);

    bool isAuthority = NetIsAuthority();

    for (uint32_t i = 0; i < numActors; ++i)
    {
        bool bp = stream.ReadBool();

        Actor* newActor = nullptr;

        if (bp)
        {
            AssetRef bpRef;
            stream.ReadAsset(bpRef);
            Blueprint* bp = bpRef.Get<Blueprint>();
            uint32_t numOverrides = stream.ReadUint32();

            std::vector<PropertyOverride> overs;
            overs.resize(numOverrides);

            for (uint32_t o = 0; o < numOverrides; ++o)
            {
                overs[o].mIndex = stream.ReadInt32();
                overs[o].mProperty.ReadStream(stream, false);
            }

            if (bp != nullptr)
            {
                newActor = bp->Instantiate(GetWorld(), isAuthority);
                newActor->ApplyPropertyOverrides(overs);
            }
        }
        else
        {
            TypeId actorType = (TypeId)stream.ReadUint32();
            newActor = GetWorld()->SpawnActor(actorType, isAuthority);
            newActor->LoadStream(stream);
        }

        if (newActor != nullptr)
        {
            newActor->SetLevel(this);

            glm::vec3 transformedPos = transform * glm::vec4(newActor->GetPosition(), 1.0f);
            glm::vec3 transformedRot = rotation + newActor->GetRotationEuler();
            newActor->SetPosition(transformedPos);
            newActor->SetRotation(transformedRot);

            newActor->UpdateComponentTransforms();
            
            if (!isAuthority && newActor->IsReplicated())
            {
                // There should be a better way to do this, but the client should not 
                // instantiate network actors. Let the server send down the SpawnActor messages.
                GetWorld()->DestroyActor(newActor);
                newActor = nullptr;
            }

#if !EDITOR
            // Delete any actors that are solely a baked light when in Non-Editor config
            if (newActor->GetNumComponents() == 1 &&
                newActor->GetComponent(0)->Is(LightComponent::ClassRuntimeId()))
            {
                LightComponent* lightComp = newActor->GetComponent(0)->As<LightComponent>();
                if (lightComp->GetLightingDomain() == LightingDomain::Static)
                {
                    GetWorld()->DestroyActor(newActor);
                    newActor = nullptr;
                }
            }
#endif
        }
        else
        {
            LogWarning("Failed to instantiate in Level::LoadIntoWorld()");
        }
    }

    std::vector<LevelRef>& loadedLevels = world->GetLoadedLevels();
    if (std::find(loadedLevels.begin(), loadedLevels.end(), this) == loadedLevels.end())
    {
        loadedLevels.push_back(this);
    }
}

void Level::UnloadFromWorld(World* world)
{
    // Erase this level from the loaded level list.
    std::vector<LevelRef>& loadedLevels = world->GetLoadedLevels();
    for (uint32_t i = 0; i < loadedLevels.size(); ++i)
    {
        if (loadedLevels[i] == this)
        {
            loadedLevels.erase(loadedLevels.begin() + i);
        }
    }

    // World can be null when shutting down.
    if (world != nullptr)
    {
        const std::vector<Actor*>& actors = world->GetActors();
        for (int32_t i = int32_t(actors.size() - 1); i >= 0; --i)
        {
            if (actors[i]->GetLevel() == this)
            {
                world->DestroyActor(i);
            }
        }

        RemoveSettings(false);
    }
}

bool Level::GetNetLoad() const
{
    return mNetLoad;
}

void Level::ApplySettings(bool force)
{
    if (force || mSetAmbientLightColor)
    {
        glm::vec4 ambientLight = DEFAULT_AMBIENT_LIGHT_COLOR;
        if (mSetAmbientLightColor)
        {
            ambientLight = mAmbientLightColor;
        }
        GetWorld()->SetAmbientLightColor(ambientLight);
    }

    if (force || mSetShadowColor)
    {
        glm::vec4 shadowColor = DEFAULT_SHADOW_COLOR;
        if (mSetShadowColor)
        {
            shadowColor = mShadowColor;
        }
        GetWorld()->SetShadowColor(shadowColor);
    }

    if (force || mSetFog)
    {
        FogSettings fogSettings;
        if (mSetFog)
        {
            fogSettings.mEnabled = mFogEnabled;
            fogSettings.mColor = mFogColor;
            fogSettings.mDensityFunc = mFogDensityFunc;
            fogSettings.mNear = mFogNear;
            fogSettings.mFar = mFogFar;
        }
        GetWorld()->SetFogSettings(fogSettings);
    }
}

void Level::RemoveSettings(bool force)
{
    if (force || mSetAmbientLightColor)
    {
        GetWorld()->SetAmbientLightColor(DEFAULT_AMBIENT_LIGHT_COLOR);
    }

    if (force || mSetShadowColor)
    {
        GetWorld()->SetShadowColor(DEFAULT_SHADOW_COLOR);
    }

    if (force || mSetFog)
    {
        FogSettings fogSettings;
        GetWorld()->SetFogSettings(fogSettings);
    }
}

bool Level::ShouldSaveActor(Actor* actor) const
{
    if (actor->GetName() == "Default Camera" ||
        actor->IsTransient())
        return false;

    return true;
}