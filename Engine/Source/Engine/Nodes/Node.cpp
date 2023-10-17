#include "Components/Component.h"
#include "Actor.h"
#include "Log.h"
#include "World.h"
#include "ObjectRef.h"

#include "Components/TransformComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ParticleComponent.h"
#include "Components/AudioComponent.h"

FORCE_LINK_DEF(Component);
DEFINE_FACTORY_MANAGER(Component);
DEFINE_FACTORY(Component, Component);
DEFINE_RTTI(Component);

Component::Component()
{
    mName = "Node";
}

Component::~Component()
{

}

void Component::Create()
{

}

void Component::Destroy()
{
    ComponentRef::EraseReferencesToObject(this);

    if (IsPrimitiveComponent() && GetWorld())
    {
        GetWorld()->PurgeOverlaps(static_cast<PrimitiveComponent*>(this));
    }

#if EDITOR
    GetWorld()->DeselectComponent(this);
#endif
}

void Component::SaveStream(Stream& stream)
{
    stream.WriteString(mName);
    stream.WriteBool(mActive);
    stream.WriteBool(mVisible);
}

void Component::LoadStream(Stream& stream)
{
    stream.ReadString(mName);
    mActive = stream.ReadBool();
    mVisible = stream.ReadBool();
}

void Component::Copy(Component* srcComp)
{
    OCT_ASSERT(srcComp);
    OCT_ASSERT(srcComp->GetType() == GetType());

    // Copy actor properties
    std::vector<Property> srcProps;
    srcComp->GatherProperties(srcProps);

    std::vector<Property> dstProps;
    GatherProperties(dstProps);

    for (uint32_t i = 0; i < srcProps.size(); ++i)
    {
        Property* srcProp = &srcProps[i];
        Property* dstProp = nullptr;

        for (uint32_t j = 0; j < dstProps.size(); ++j)
        {
            if (dstProps[j].mName == srcProp->mName &&
                dstProps[j].mType == srcProp->mType)
            {
                dstProp = &dstProps[j];
            }
        }

        if (dstProp != nullptr)
        {
            OCT_ASSERT(dstProp->mCount == srcProp->mCount);
            dstProp->SetValue(srcProp->mData.vp, 0, srcProp->mCount);
        }

        // For script components... if we first copy over the Filename property,
        // that will change the number of properties on the script so we need to regather them.
        // Script component is really the only component that can dynamically change its properties,
        // so I'm adding a hack now just for script component.
        if (srcComp->Is(ScriptComponent::ClassRuntimeId()) &&
            srcProp->mName == "Filename")
        {
            dstProps.clear();
            GatherProperties(dstProps);
        }
    }
}

void Component::BeginPlay()
{

}

void Component::EndPlay()
{

}

void Component::Tick(float deltaTime)
{
    
}

void Component::EditorTick(float deltaTime)
{
    Tick(deltaTime);
}

void Component::GatherProperties(std::vector<Property>& outProps)
{
    outProps.push_back({DatumType::String, "Name", this, &mName});
    outProps.push_back({DatumType::Bool, "Active", this, &mActive});
    outProps.push_back({DatumType::Bool, "Visible", this, &mVisible});
}

void Component::SetOwner(Actor* owner)
{
    World* prevWorld = mOwner ? mOwner->GetWorld() : nullptr;
    mOwner = owner;
    World* newWorld = mOwner ? mOwner->GetWorld() : nullptr;

    if (prevWorld != newWorld)
    {
        if (prevWorld != nullptr)
        {
            prevWorld->UnregisterComponent(this);
        }

        if (newWorld != nullptr)
        {
            newWorld->RegisterComponent(this);
        }
    }
}

Actor* Component::GetOwner()
{
    return mOwner;
}

void Component::SetName(const std::string& newName)
{
    mName = newName;
}

const std::string& Component::GetName() const
{
    return mName;
}

void Component::SetActive(bool active)
{
    mActive = active;
}

bool Component::IsActive() const
{
    return mActive;
}

void Component::SetVisible(bool visible)
{
    mVisible = visible;
}

bool Component::IsVisible() const
{
    return mVisible;
}

void Component::SetTransient(bool transient)
{
    mTransient = transient;
}

bool Component::IsTransient() const
{
    return mTransient;
}

void Component::SetDefault(bool isDefault)
{
    mDefault = isDefault;
}

bool Component::IsDefault() const
{
    return mDefault;
}

World* Component::GetWorld()
{
    return mOwner ? mOwner->GetWorld() : nullptr;
}

const char* Component::GetTypeName() const
{
    return "Component";
}

DrawData Component::GetDrawData()
{
    DrawData ret = {};
    ret.mComponent = nullptr;
    ret.mMaterial = nullptr;
    return ret;
}

bool Component::IsTransformComponent() const
{
    return false;
}

bool Component::IsPrimitiveComponent() const
{
    return false;
}

bool Component::IsLightComponent() const
{
    return false;
}
