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
        stream.ReadString(mComponents[c].mName);
        stream.ReadString(mComponents[c].mParentName);
        mComponents[c].mType = (TypeId)stream.ReadUint32();
        mComponents[c].mDefault = stream.ReadBool();

        mComponents[c].mProperties.resize(stream.ReadUint32());

        for (uint32_t p = 0; p < mComponents[c].mProperties.size(); ++p)
        {
            mComponents[c].mProperties[p].ReadStream(stream, false);
        }
    }

    stream.ReadString(mRootComponentName);
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
        stream.WriteString(mComponents[c].mName);
        stream.WriteString(mComponents[c].mParentName);
        stream.WriteUint32((uint32_t)mComponents[c].mType);
        stream.WriteBool(mComponents[c].mDefault);

        stream.WriteUint32((uint32_t)mComponents[c].mProperties.size());

        for (uint32_t p = 0; p < mComponents[c].mProperties.size(); ++p)
        {
            mComponents[c].mProperties[p].WriteStream(stream);
        }
    }

    stream.WriteString(mRootComponentName);
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
        mComponents[i].mName = comps[i]->GetName();
        mComponents[i].mType = comps[i]->GetType();
        mComponents[i].mDefault = comps[i]->IsDefault();

        std::vector<Property> compProps;
        comps[i]->GatherProperties(compProps);

        mComponents[i].mProperties.resize(compProps.size());
        for (uint32_t p = 0; p < compProps.size(); ++p)
        {
            mComponents[i].mProperties[p].DeepCopy(compProps[p], true);
        }

        if (comps[i]->IsTransformComponent())
        {
            TransformComponent* transComp = static_cast<TransformComponent*>(comps[i]);
            if (transComp->GetParent() != nullptr)
            {
                mComponents[i].mParentName = transComp->GetParent()->GetName();
            }
            else
            {
                mComponents[i].mParentName = "";
            }
        }
        else
        {
            mComponents[i].mParentName = "";
        }
    }

    assert(srcActor->GetRootComponent());
    if (srcActor->GetRootComponent())
    {
        mRootComponentName = srcActor->GetRootComponent()->GetName();
    }
    else
    {
        mRootComponentName = "";
    }

    srcActor->SetBlueprintSource(this);
}

Actor* Blueprint::Instantiate(World* world)
{
    Actor* retActor = nullptr;

    if (mActorType != INVALID_TYPE_ID)
    {
        // Spawn actor first, wait until end of function to add to network.
        retActor = world->SpawnActor(mActorType, false);
        retActor->SetBlueprintSource(this);

        uint32_t numDefaultComps = retActor->GetNumComponents();

        std::vector<Property> dstProps;
        retActor->GatherProperties(dstProps);

        CopyPropertyValues(dstProps, mActorProps);

        // Initialize native components
        std::vector<bool> copiedComps;
        std::vector<bool> copiedBpComps;
        copiedComps.resize(numDefaultComps);
        copiedBpComps.resize(mComponents.size());

        for (uint32_t i = 0; i < retActor->GetNumComponents(); ++i)
        {
            Component* defaultComp = retActor->GetComponent((int32_t)i);
            int32_t bpCompIdx = -1;
            BlueprintComp* bpComp = FindBlueprintComp(defaultComp->GetName(), true, &bpCompIdx);

            if (bpComp != nullptr)
            {
                dstProps.clear();
                defaultComp->GatherProperties(dstProps);

                CopyPropertyValues(dstProps, bpComp->mProperties);

                copiedComps[i] = true;
                assert(bpCompIdx >= 0 && !copiedBpComps[bpCompIdx]);
                copiedBpComps[bpCompIdx] = true;
            }
        }

        // Try to fixup renamed default components and make sure to log a warning.
        for (uint32_t i = 0; i < copiedComps.size(); ++i)
        {
            if (!copiedComps[i])
            {
                Component* comp = retActor->GetComponent(i);

                for (uint32_t b = 0; b < copiedBpComps.size(); ++b)
                {
                    if (!copiedBpComps[b] &&
                        mComponents[b].mDefault &&
                        mComponents[b].mType == comp->GetType())
                    {
                        LogWarning("Linking BP Component '%s' to Default Component '%s'", mComponents[b].mName.c_str(), comp->GetName().c_str());

                        // If the root component was renamed, we need record that.
                        if (mRootComponentName == mComponents[b].mName)
                        {
                            mRootComponentName = comp->GetName();
                        }

                        mComponents[b].mName = comp->GetName();


                        dstProps.clear();
                        comp->GatherProperties(dstProps);

                        CopyPropertyValues(dstProps, mComponents[b].mProperties);

                        copiedComps[i] = true;
                        copiedBpComps[b] = true;
                    }
                }
            }
        }

#if EDITOR
        // In the editor, remove any unused default BlueprintComps
        uint32_t numTrimmedComps = 0;
        for (int32_t i = int32_t(copiedBpComps.size()) - 1; i >= 0; --i)
        {
            if (!copiedBpComps[i] &&
                mComponents[i].mDefault)
            {
                mComponents.erase(mComponents.begin() + i);
                numTrimmedComps++;
            }
        }

        if (numTrimmedComps > 0)
        {
            LogWarning("Trimmed %d unused default components saved on Blueprint: %s", numTrimmedComps, GetName().c_str());
        }
#endif

        // Create and initialize non-default components
        for (uint32_t i = 0; i < mComponents.size(); ++i)
        {
            if (!mComponents[i].mDefault)
            {
                Component* comp = retActor->CreateComponent(mComponents[i].mType);

                dstProps.clear();
                comp->GatherProperties(dstProps);

                CopyPropertyValues(dstProps, mComponents[i].mProperties);
            }
        }

        // Setup transform hierarchy for all non-default components.
        const std::vector<Component*>& comps = retActor->GetComponents();

        for (uint32_t i = 0; i < comps.size(); ++i)
        {
            if (comps[i]->IsTransformComponent() &&
                !comps[i]->IsDefault())
            {
                BlueprintComp* bpComp = FindBlueprintComp(comps[i]->GetName(), false);
                assert(bpComp);

                if (bpComp->mParentName != "")
                {
                    Component* parentComp = retActor->GetComponent(bpComp->mParentName);
                    assert(parentComp->IsTransformComponent());

                    TransformComponent* transComp = static_cast<TransformComponent*>(comps[i]);
                    TransformComponent* parentTransComp = static_cast<TransformComponent*>(parentComp);

                    transComp->Attach(parentTransComp);
                }
            }
        }

        // Establish the root component
        TransformComponent* newRoot = nullptr;
        if (mRootComponentName != "")
        {
            Component* comp = retActor->GetComponent(mRootComponentName);
            if (comp->IsTransformComponent())
            {
                newRoot = static_cast<TransformComponent*>(comp);
            }
            else
            {
                LogWarning("Blueprint Root Component name is not a TransformComponent");
            }
        }

        // No root was specified, so leave the root as is, if there is one.
        newRoot = retActor->GetRootComponent();

        if (newRoot == nullptr)
        {
            // Pick the first transform component as a last resort
            for (uint32_t i = 0; i < retActor->GetNumComponents(); ++i)
            {
                Component* comp = retActor->GetComponent((int32_t)i);
                if (comp->IsTransformComponent())
                {
                    newRoot = static_cast<TransformComponent*>(comp);
                }
            }
        }

        if (newRoot != retActor->GetRootComponent())
        {
            retActor->SetRootComponent(newRoot);
        }

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

BlueprintComp* Blueprint::FindBlueprintComp(const std::string& name, bool isDefault, int32_t* outIndex)
{
    BlueprintComp* bpComp = nullptr;
    int32_t index = -1;

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i].mDefault != isDefault)
            continue;

        if (mComponents[i].mName == name)
        {
            bpComp = &mComponents[i];
            index = (int32_t)i;
            break;
        }
    }

    if (outIndex)
    {
        *outIndex = index;
    }

    return bpComp;
}


