#include "Assets/Blueprint.h"
#include "World.h"
#include "Actor.h"
#include "Log.h"
#include "Engine.h"
#include "NetworkManager.h"

FORCE_LINK_DEF(Blueprint);
DEFINE_ASSET(Blueprint);

Blueprint::Blueprint()
{
    mType = Blueprint::GetStaticType();
}

Blueprint::~Blueprint()
{

}

void Blueprint::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mActorType = (TypeId)stream.ReadUint32();
    mActorProps.resize(stream.ReadUint32());

    for (uint32_t i = 0; i < mActorProps.size(); ++i)
    {
        mActorProps[i].ReadStream(stream, false);
    }

    uint32_t numComps = stream.ReadUint32();
    mComponents.resize(numComps);

    for (uint32_t c = 0; c < numComps; ++c)
    {
        mComponents[c].mType = (TypeId)stream.ReadUint32();
        mComponents[c].mParent = stream.ReadInt32();
        mComponents[c].mProperties.resize(stream.ReadUint32());

        for (uint32_t p = 0; p < mComponents[c].mProperties.size(); ++p)
        {
            mComponents[c].mProperties[p].ReadStream(stream, false);
        }
    }

    mRootComponentIndex = stream.ReadInt32();
}

void Blueprint::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteUint32((uint32_t)mActorType);
    stream.WriteUint32((uint32_t)mActorProps.size());

    for (uint32_t i = 0; i < mActorProps.size(); ++i)
    {
        mActorProps[i].WriteStream(stream);
    }

    stream.WriteUint32((uint32_t)mComponents.size());

    for (uint32_t c = 0; c < mComponents.size(); ++c)
    {
        stream.WriteUint32((uint32_t)mComponents[c].mType);
        stream.WriteInt32(mComponents[c].mParent);
        stream.WriteUint32((uint32_t)mComponents[c].mProperties.size());

        for (uint32_t p = 0; p < mComponents[c].mProperties.size(); ++p)
        {
            mComponents[c].mProperties[p].WriteStream(stream);
        }
    }

    stream.WriteInt32(mRootComponentIndex);
}

void Blueprint::Create()
{
    Asset::Create();
}

void Blueprint::Destroy()
{
    Asset::Destroy();
}

void Blueprint::Import(const std::string& path)
{
    Asset::Import(path);
}

void Blueprint::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
}

glm::vec4 Blueprint::GetTypeColor()
{
    return glm::vec4(0.2f, 0.2f, 1.0f, 1.0f);
}

const char* Blueprint::GetTypeName()
{
    return "Blueprint";
}

void Blueprint::Create(Actor* srcActor)
{
    mActorType = INVALID_TYPE_ID;
    mActorProps.clear();
    mComponents.clear();

    if (srcActor == nullptr)
        return;

    assert(srcActor->DoComponentsHaveUniqueNames());

    mActorType = srcActor->GetType();

    std::vector<Property> actorProps;
    srcActor->GatherProperties(actorProps);

    mActorProps.resize(actorProps.size());
    for (uint32_t i = 0; i < actorProps.size(); ++i)
    {
        mActorProps[i].DeepCopy(actorProps[i], true);
    }

    const std::vector<Component*>& comps = srcActor->GetComponents();

    mComponents.resize(comps.size());

    for (uint32_t i = 0; i < comps.size(); ++i)
    {
        mComponents[i].mType = comps[i]->GetType();

        std::vector<Property> compProps;
        comps[i]->GatherProperties(compProps);

        mComponents[i].mProperties.resize(compProps.size());
        for (uint32_t p = 0; p < compProps.size(); ++p)
        {
            mComponents[i].mProperties[p].DeepCopy(compProps[p], true);
        }

        if (comps[i]->IsTransformComponent())
        {
            mComponents[i].mParent = static_cast<TransformComponent*>(comps[i])->FindParentComponentIndex();
        }
        else
        {
            mComponents[i].mParent = -1;
        }

        if (comps[i]->GetOwner()->GetRootComponent() == comps[i])
        {
            mRootComponentIndex = (int32_t)i;
        }
    }

    srcActor->SetBlueprintSource(this);
}

Actor* Blueprint::Instantiate(World* world)
{
    Actor* retActor = nullptr;

    if (mActorType != INVALID_TYPE_ID)
    {
        retActor = world->SpawnActor(mActorType, false);
        retActor->SetBlueprintSource(this);

        std::vector<Property> dstProps;
        retActor->GatherProperties(dstProps);

        CopyPropertyValues(dstProps, mActorProps);

        // Now we need to add components to the new actor. It is assumed that the native components
        // created by an actor's class are the first components in the Actor::mComponent array.
        // So add any extra components that aren't "native" to the actor's class type
        uint32_t numNativeComps = (uint32_t)retActor->GetComponents().size();
        for (uint32_t i = numNativeComps; i < mComponents.size(); ++i)
        {
            retActor->CreateComponent(mComponents[i].mType);
        }

        // Now copy over all of the default components values saved in this Blueprint
        const std::vector<Component*>& dstComps = retActor->GetComponents();
        assert(mComponents.size() == dstComps.size());

        uint32_t i = 0;
        for (i = 0; i < dstComps.size(); ++i)
        {
            assert(mComponents[i].mType == dstComps[i]->GetType());
            std::vector<Property> dstProps;
            dstComps[i]->GatherProperties(dstProps);

            CopyPropertyValues(dstProps, mComponents[i].mProperties);
        }

        // Setup transform hierarchy.
        for (uint32_t i = 0; i < dstComps.size(); ++i)
        {
            if (dstComps[i]->IsTransformComponent() &&
                mComponents[i].mParent != -1)
            {
                int32_t parentIndex = mComponents[i].mParent;
                assert(dstComps[parentIndex]->IsTransformComponent());

                TransformComponent* comp = static_cast<TransformComponent*>(dstComps[i]);
                TransformComponent* parentComp = static_cast<TransformComponent*>(dstComps[parentIndex]);

                comp->Attach(parentComp);
            }
        }

        // Establish the root component
        assert(mRootComponentIndex >= 0);
        assert(dstComps[mRootComponentIndex]->IsTransformComponent());
        retActor->SetRootComponent((TransformComponent*)dstComps[mRootComponentIndex]);

        // We had deferred the adding of the actor to the network.
        // Usually this happens on SpawnActor(), but we needed to load all of the saved properties
        // first before we could tell if the mReplicate flag was set.
        GetWorld()->AddNetActor(retActor, INVALID_NET_ID);
    }
    else
    {
        LogError("Failed to instantiate blueprint. Invalid Type.")
    }

    return retActor;
}

const Property* Blueprint::GetActorProperty(const char* name)
{
    const Property* ret = nullptr;

    for (uint32_t i = 0; i < mActorProps.size(); ++i)
    {
        if (mActorProps[i].mName == name)
        {
            ret = &mActorProps[i];
            break;
        }
    }

    return ret;
}

const Property* Blueprint::GetComponentProperty(int32_t index, const char* name)
{
    const Property* ret = nullptr;

    if (index >= 0 && index < (int32_t)mComponents.size())
    {
        const std::vector<Property>& props = mComponents[index].mProperties;

        for (uint32_t i = 0; i < props.size(); ++i)
        {
            if (props[i].mName == name)
            {
                ret = &props[i];
                break;
            }
        }
    }

    return ret;
}

