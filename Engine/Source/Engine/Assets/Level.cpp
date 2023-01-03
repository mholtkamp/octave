#include "Assets/Level.h"
#include "Stream.h"
#include "Actor.h"
#include "World.h"
#include "Engine.h"
#include "Log.h"
#include "NetworkManager.h"
#include "Assets/Blueprint.h"

#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

FORCE_LINK_DEF(Level);
DEFINE_ASSET(Level);

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

void Level::Import(const std::string& path)
{
    LogWarning("Level::Import() not implemented.")
}

void Level::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::Bool, "Net Load", this, &mNetLoad));
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
            }
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
    }
}

bool Level::GetNetLoad() const
{
    return mNetLoad;
}

bool Level::ShouldSaveActor(Actor* actor) const
{
    if (actor->GetName() == "Default Camera" ||
        actor->IsTransient())
        return false;

    return true;
}