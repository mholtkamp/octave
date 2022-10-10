#include "Actor.h"
#include "Renderer.h"
#include "Clock.h"
#include "World.h"
#include "Components/CameraComponent.h"
#include "Enums.h"
#include "Log.h"
#include "Utilities.h"
#include "Engine.h"
#include "ObjectRef.h"
#include "NetworkManager.h"
#include "Assets/Blueprint.h"

#include "Components/PrimitiveComponent.h"
#include "Components/LightComponent.h"
#include "Components/ScriptComponent.h"

#include "Graphics/Graphics.h"

#if LUA_ENABLED
#include "LuaBindings/LuaUtils.h"
#endif

#include <glm/glm.hpp>
#include <functional>
#include <algorithm>

using namespace std;

#define INVOKE_NET_FUNC_BODY(P) \
    NetFunc* netFunc = FindNetFunc(name); \
    assert(netFunc->mNumParams == P); \
    bool shouldExecute = ShouldExecuteNetFunc(netFunc->mType, this); \
    SendNetFunc(netFunc, P, params);

std::unordered_map<TypeId, NetFuncMap> Actor::sTypeNetFuncMap;
std::unordered_set<TypeId> Actor::sScriptRegisteredSet;

#define ENABLE_SCRIPT_FUNCS 1

bool Actor::OnRep_RootPosition(Datum* datum, const void* newValue)
{
    Actor* actor = (Actor*) datum->mOwner;
    assert(actor != nullptr);
    glm::vec3* newPos = (glm::vec3*) newValue;
    actor->GetRootComponent()->SetPosition(*newPos);
    return true;
}

bool Actor::OnRep_RootRotation(Datum* datum, const void* newValue)
{
    Actor* actor = (Actor*) datum->mOwner;
    assert(actor != nullptr);
        
    glm::vec3* newRot = (glm::vec3*) newValue;
    actor->GetRootComponent()->SetRotation(*newRot);

    return true;
}

bool Actor::OnRep_RootScale(Datum* datum, const void* newValue)
{
    Actor* actor = (Actor*) datum->mOwner;
    assert(actor != nullptr);
    glm::vec3* newScale = (glm::vec3*) newValue;
    actor->GetRootComponent()->SetScale(*newScale);
    return true;
}

Actor::Actor() :
    mRootComponent(nullptr),
    mName("Actor"),
    mWorld(nullptr),
    mLevel(nullptr),
    mNetId(INVALID_NET_ID),
    mHitCheckId(0),
    mOwningHost(INVALID_HOST_ID),
    mReplicate(false),
    mReplicateTransform(false),
    mForceReplicate(false),
    mBegunPlay(false),
    mPendingDestroy(false),
    mTickEnabled(true),
    mTransient(false),
    mReplicationRate(ReplicationRate::High),
    mNumScriptComps(0)
{

}

Actor::~Actor()
{

}

void Actor::Create()
{
#if LUA_ENABLED
    lua_State* L = GetLua();
    if (L != nullptr)
    {
        if (!AreScriptFuncsRegistered(GetType()))
        {
            RegisterScriptFuncs(L);
        }
    }
#endif
}

void Actor::Destroy()
{
    ActorRef::EraseReferencesToObject(this);
    DestroyAllComponents();
}

void Actor::Tick(float deltaTime)
{
    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i]->IsActive())
        {
            mComponents[i]->Tick(deltaTime);
        }
    }

    UpdateComponentTransforms();
}

void Actor::EditorTick(float deltaTime)
{
    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i]->IsActive())
        {
            mComponents[i]->EditorTick(deltaTime);
        }
    }

    UpdateComponentTransforms();
}

void Actor::BeginPlay()
{
    mBegunPlay = true;

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        mComponents[i]->BeginPlay();
    }
}

void Actor::EndPlay()
{
    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        mComponents[i]->EndPlay();
    }
}

void Actor::SaveStream(Stream& stream)
{
#if EDITOR
    stream.WriteString(mName);
    stream.WriteBool(mReplicate);
    stream.WriteBool(mReplicateTransform);

    // Tags
    assert(mTags.size() <= 255);
    uint32_t numTags = glm::min((uint32_t)mTags.size(), 255u);
    stream.WriteUint8(numTags);

    for (uint32_t i = 0; i < numTags; ++i)
    {
        stream.WriteString(mTags[i]);
    }

    // Components
    std::vector<Component*> compsToSave;
    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (!mComponents[i]->IsTransient())
        {
            compsToSave.push_back(mComponents[i]);
        }
    }

    uint32_t numComponents = uint32_t(compsToSave.size());
    stream.WriteUint32(numComponents);
    for (uint32_t i = 0; i < numComponents; ++i)
    {
        stream.WriteUint32(uint32_t(compsToSave[i]->GetType()));
        compsToSave[i]->SaveStream(stream);
    }

    // Component Hierarchy
    // Pairs of <index, parentIndex>
    for (int32_t i = 0; i < int32_t(numComponents); ++i)
    {
        if (!compsToSave[i]->IsTransformComponent())
        {
            continue;
        }
        TransformComponent* transComp = static_cast<TransformComponent*>(compsToSave[i]);

        stream.WriteInt32(i);

        int32_t parent = -2;

        if (transComp == GetRootComponent())
        {
            parent = -1;
        }
        else
        {
            for (int32_t p = 0; p < int32_t(numComponents); ++p)
            {
                if (compsToSave[p] == transComp->GetParent())
                {
                    parent = p;
                    break;
                }
            }
        }


        // Could not find a parent for this transform comp.
        assert(parent >= -1);
        stream.WriteInt32(parent);
    }
#endif
}

void Actor::LoadStream(Stream& stream)
{
    stream.ReadString(mName);
    mReplicate = stream.ReadBool();
    mReplicateTransform = stream.ReadBool();

    // Tags
    uint32_t numTags = (uint32_t)stream.ReadUint8();
    mTags.resize(numTags);
    for (uint32_t i = 0; i < numTags; ++i)
    {
        stream.ReadString(mTags[i]);
    }

    // Components
    static std::vector<Component*> compsToLoad;
    compsToLoad.clear();
    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (!mComponents[i]->IsTransient())
        {
            compsToLoad.push_back(mComponents[i]);
        }
    }

    uint32_t numComponents = stream.ReadUint32();
    LogDebug("[Actor::Load] numComponents = %d", numComponents);
    
    for (uint32_t i = 0; i < numComponents; ++i)
    {
        TypeId type = TypeId(stream.ReadUint32());
        
        Component* comp = nullptr;
        if (i < compsToLoad.size())
        {
            // This actor has a default component created already.
            comp = compsToLoad[i];

            // The component type should match. Default components should
            // always come first in the mComponents vector.
            assert(type == comp->GetType());
        }
        else
        {
            comp = CreateComponent(type);
            compsToLoad.push_back(comp);
        }

        comp->LoadStream(stream);
    }

    // Component Hierarchy
    for (int32_t i = 0; i < int32_t(numComponents); ++i)
    {
        if (!compsToLoad[i]->IsTransformComponent())
        {
            continue;
        }

        int32_t compIndex = stream.ReadInt32();
        int32_t parentIndex = stream.ReadInt32();

        assert(compsToLoad[compIndex]->IsTransformComponent());
        assert(parentIndex == -1 || compsToLoad[parentIndex]->IsTransformComponent());

        TransformComponent* transComp = static_cast<TransformComponent*>(compsToLoad[compIndex]);

        if (parentIndex == -1)
        {
            SetRootComponent(transComp);
        }
        else
        {
            TransformComponent* parentComp = static_cast<TransformComponent*>(compsToLoad[parentIndex]);
            transComp->Attach(parentComp);
        }
    }

    if (GetRootComponent() != nullptr)
    {
        glm::vec3 pos = GetRootComponent()->GetPosition();
        LogDebug("[Actor::Load] Pos = <%.2f, %.2f, %.2f>", pos.x, pos.y, pos.z);

        // TODO: Figure out better solution.
        // Needed to add this for actors with default components.
        // Their transforms weren't correct.
        GetRootComponent()->MarkTransformDirty();
        UpdateComponentTransforms();
    }
}

void Actor::Copy(Actor* srcActor)
{
    if (srcActor == nullptr ||
        srcActor->GetType() != GetType())
    {
        LogError("Failed to copy actor");
        return;
    }

    // Copy actor properties
    std::vector<Property> srcProps;
    srcActor->GatherProperties(srcProps);

    std::vector<Property> dstProps;
    GatherProperties(dstProps);

    CopyPropertyValues(dstProps, srcProps);
    CopyComponents(srcActor->GetComponents());

    mBlueprintSource = srcActor->GetBlueprintSource();

    // TODO
    // Increment actor name.
}

void Actor::Render(PipelineId pipelineId)
{
    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i]->IsPrimitiveComponent() && mComponents[i]->IsVisible())
        {
            PrimitiveComponent* primComp = static_cast<PrimitiveComponent*>(mComponents[i]);
            GFX_BindPipeline(pipelineId, primComp->GetVertexType());
            primComp->Render();
        }
    }
}

void Actor::GatherProperties(std::vector<Property>& outProps)
{
    outProps.push_back(Property(DatumType::String, "Name", this, &mName));
    outProps.push_back(Property(DatumType::Bool, "Replicate", this, &mReplicate));
    outProps.push_back(Property(DatumType::Bool, "Replicate Transform", this, &mReplicateTransform));
    outProps.push_back(Property(DatumType::String, "Tags", this, &mTags).MakeVector());
}

void Actor::GatherReplicatedData(std::vector<NetDatum>& outData)
{
    outData.push_back(NetDatum(DatumType::Byte, this, &mOwningHost));

    if (mReplicateTransform)
    {
        outData.push_back(NetDatum(DatumType::Vector, this, &mRootComponent->GetPositionRef(), 1, OnRep_RootPosition));
        outData.push_back(NetDatum(DatumType::Vector, this, &mRootComponent->GetRotationEulerRef(), 1, OnRep_RootRotation));
        outData.push_back(NetDatum(DatumType::Vector, this, &mRootComponent->GetScaleRef(), 1, OnRep_RootScale));
    }
}

void Actor::GatherNetFuncs(std::vector<NetFunc>& outFuncs)
{
    
}

void Actor::GatherPropertyOverrides(std::vector<PropertyOverride>& outOverrides)
{
    if (mBlueprintSource != nullptr)
    {
        Blueprint* bp = mBlueprintSource.Get<Blueprint>();

        std::vector<Property> actorProps;
        GatherProperties(actorProps);

        for (uint32_t i = 0; i < actorProps.size(); ++i)
        {
            const Property* bpProp = bp->GetActorProperty(actorProps[i].mName.c_str());
            if (bpProp == nullptr ||
                actorProps[i] != *bpProp)
            {
                PropertyOverride over;
                over.mIndex = -1; // Actor
                over.mProperty = actorProps[i];
                outOverrides.push_back(over);
            }
        }

        // Next, iterate over component properties.
        for (int32_t c = 0; c < int32_t(mComponents.size()); ++c)
        {
            std::vector<Property> compProps;
            mComponents[c]->GatherProperties(compProps);

            for (uint32_t i = 0; i < compProps.size(); ++i)
            {
                const Property* bpProp = bp->GetComponentProperty(c, compProps[i].mName.c_str());

                bool rootPos = (mComponents[c] == GetRootComponent()) && (compProps[i].mName == "Position");

                if (bpProp == nullptr ||
                    compProps[i] != *bpProp ||
                    rootPos)
                {
                    PropertyOverride over;
                    over.mIndex = c; // Component index
                    over.mProperty = compProps[i];
                    outOverrides.push_back(over);
                }
            }
        }
    }
}

void Actor::ApplyPropertyOverrides(const std::vector<PropertyOverride>& overs)
{
    // So... we are going to assume that the property override list is in order of index.
    // Actor overrides come first (index -1), then comp0, then comp1, etc...
    // This way, we only need to gather one set of dst props at a time.
    // We could possibly sort the vector first to ensure this.
    int32_t curIndex = -2;
    std::vector<Property> props;

    for (uint32_t i = 0; i < overs.size(); ++i)
    {
        if (overs[i].mIndex > curIndex)
        {
            curIndex = overs[i].mIndex;
            props.clear();

            if (curIndex == -1)
            {
                GatherProperties(props);
            }
            else if (curIndex >= 0 &&
                curIndex < int32_t(mComponents.size()))
            {
                Component* comp = mComponents[curIndex];
                comp->GatherProperties(props);
            }
        }
        else if (overs[i].mIndex < curIndex)
        {
            // Can't iterate backwards, just skip and maybe give a warning.
            LogWarning("Out of order property override. Ignoring");
            continue;
        }

        for (uint32_t d = 0; d < props.size(); ++d)
        {
            if (props[d].mName == overs[i].mProperty.mName &&
                props[d].mType == overs[i].mProperty.mType &&
                props[d].mCount == overs[i].mProperty.mCount)
            {
                props[d].SetValue(overs[i].mProperty.mData.vp, 0, props[d].mCount);
                break;
            }
        }
    }
}

int Actor::CreateActorMetatable(lua_State* L, const char* className, const char* parentName)
{
    //TypeId typeId = GetType();
    //if (sScriptActorSet.find(typeId) == sScriptActorSet.end())
    {
        char classFlag[128];
        snprintf(classFlag, 128, "cf%s", className);

        int mtIndex = CreateClassMetatable(
            className,
            classFlag,
            parentName);

        return mtIndex;

     //   sScriptActorSet.insert(typeId);

        // Pop the metatable
        //lua_pop(L, 1);
    }
}

void Actor::RegisterScriptFuncs(lua_State* L)
{
    // Base class Actor functions are implemented in Actor_Lua.cpp
    // For user-created actor classes, this function is overridden automatically by DECLARE_ACTOR()
    // Add user script funcs by using the SCRIPT_FUNC macros in ScriptUtils.h
}

bool Actor::AreScriptFuncsRegistered(TypeId type)
{
    assert(type != INVALID_TYPE_ID);
    bool registered = (sScriptRegisteredSet.find(type) != sScriptRegisteredSet.end());
    return registered;
}

void Actor::SetScriptFuncsRegistered(TypeId type)
{
    assert(sScriptRegisteredSet.find(type) == sScriptRegisteredSet.end());
    sScriptRegisteredSet.insert(type);
}

void Actor::BeginOverlap(PrimitiveComponent* thisComp, PrimitiveComponent* other)
{
    for (uint32_t i = 0; i < mEventHandlerScripts.size(); ++i)
    {
        mEventHandlerScripts[i]->BeginOverlap(thisComp, other);
    }

    //LogDebug("Begin Overlap <%s> with <%s>", this->GetName().c_str(), other->GetName().c_str());
}
void Actor::EndOverlap(PrimitiveComponent* thisComp, PrimitiveComponent* other)
{
    for (uint32_t i = 0; i < mEventHandlerScripts.size(); ++i)
    {
        mEventHandlerScripts[i]->EndOverlap(thisComp, other);
    }

    //LogDebug("End Overlap <%s> with <%s>", this->GetName().c_str(), other->GetName().c_str());
}
void Actor::OnCollision(
    PrimitiveComponent* thisComp,
    PrimitiveComponent* other,
    glm::vec3 impactPoint,
    glm::vec3 impactNormal,
    btPersistentManifold* manifold)
{
    //LogDebug("Collisions [%d] <%s> with <%s>", manifold->getNumContacts(), this->GetName().c_str(), other->GetName().c_str());


    for (uint32_t i = 0; i < mEventHandlerScripts.size(); ++i)
    {
        mEventHandlerScripts[i]->OnCollision(thisComp, other, impactPoint, impactNormal, manifold);
    }
}

void Actor::RenderShadow()
{
#if 0
    Pipeline* shadowPipeline = Renderer::Get()->GetShadowCaster()->GetPipeline();

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i]->IsPrimitiveComponent() && mComponents[i]->IsVisible())
        {
            static_cast<PrimitiveComponent*>(mComponents[i])->RenderWithPipeline(shadowPipeline);
        }
    }
#endif
}

const std::string& Actor::GetName() const
{
    return mName;
}

void Actor::SetName(const std::string& name)
{
    mName = name;
}

void Actor::Attach(Actor* actor)
{
    if (actor != nullptr)
    {
        GetRootComponent()->Attach(actor->GetRootComponent());
    }
    else
    {
        GetRootComponent()->Attach(nullptr);
    }
}

void Actor::Attach(TransformComponent* comp)
{
    GetRootComponent()->Attach(comp);
}

void Actor::SetPendingDestroy(bool pendingDestroy)
{
    mPendingDestroy = pendingDestroy;
}

bool Actor::IsPendingDestroy() const
{
    return mPendingDestroy;
}

bool Actor::HasBegunPlay() const
{
    return mBegunPlay;
}

void Actor::EnableTick(bool enable)
{
    mTickEnabled = enable;
}

bool Actor::IsTickEnabled() const
{
    return mTickEnabled;
}

void Actor::SetTransient(bool transient)
{
    mTransient = transient;
}

bool Actor::IsTransient() const
{
    return mTransient;
}

glm::vec3 Actor::GetPosition() const
{
    glm::vec3 position(0);

    if (mRootComponent != nullptr)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            position = mRootComponent->GetAbsolutePosition();
        }
        else
        {
            position = mRootComponent->GetPosition();
        }
    }

    return position;
}

glm::quat Actor::GetRotationQuat() const
{
    glm::quat rotation = glm::quat();

    if (mRootComponent != nullptr)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            rotation = mRootComponent->GetAbsoluteRotationQuat();
        }
        else
        {
            rotation = mRootComponent->GetRotationQuat();
        }
    }

    return rotation;
}

glm::vec3 Actor::GetRotationEuler() const
{
    glm::vec3 rotation = {};

    if (mRootComponent != nullptr)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            rotation = mRootComponent->GetAbsoluteRotationEuler();
        }
        else
        {
            rotation = mRootComponent->GetRotationEuler();
        }
    }

    return rotation;
}

glm::vec3 Actor::GetScale() const
{
    glm::vec3 scale = {};

    if (mRootComponent != nullptr)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            scale = mRootComponent->GetAbsoluteScale();
        }
        else
        {
            scale = mRootComponent->GetScale();
        }
    }

    return scale;
}

void Actor::SetPosition(glm::vec3 position)
{
    if (mRootComponent)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            mRootComponent->SetAbsolutePosition(position);
        }
        else
        {
            mRootComponent->SetPosition(position);
        }
    }
}

bool Actor::SweepToPosition(glm::vec3 position, SweepTestResult& outSweepResult, uint8_t mask)
{
    bool hit = false;

    if (mRootComponent)
    {
        if (mRootComponent->IsPrimitiveComponent())
        {
            PrimitiveComponent* primComp = static_cast<PrimitiveComponent*>(mRootComponent);
            hit = primComp->SweepToWorldPosition(position, outSweepResult, mask);
        }
        else
        {
            SetPosition(position);
        }
    }

    return hit;
}

void Actor::SetRotation(glm::quat rotation)
{
    if (mRootComponent)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            mRootComponent->SetAbsoluteRotation(rotation);
        }
        else
        {
            mRootComponent->SetRotation(rotation);
        }
    }
}

void Actor::SetRotation(glm::vec3 rotation)
{
    if (mRootComponent)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            mRootComponent->SetAbsoluteRotation(rotation);
        }
        else
        {
            mRootComponent->SetRotation(rotation);
        }
    }
}

void Actor::SetScale(glm::vec3 scale)
{
    if (mRootComponent)
    {
        if (mRootComponent->GetParent() != nullptr)
        {
            mRootComponent->SetAbsoluteScale(scale);
        }
        else
        {
            mRootComponent->SetScale(scale);
        }
    }
}

glm::vec3 Actor::GetForwardVector()
{
    glm::vec3 dir = glm::vec3(0.0f, 0.0f, -1.0f);
    if (mRootComponent)
    {
        dir = mRootComponent->GetForwardVector();
    }
    return dir;
}

glm::vec3 Actor::GetRightVector()
{
    glm::vec3 dir = glm::vec3(1.0f, 0.0f, 0.0f);
    if (mRootComponent)
    {
        dir = mRootComponent->GetRightVector();
    }
    return dir;
}

glm::vec3 Actor::GetUpVector()
{
    glm::vec3 dir = glm::vec3(0.0f, 1.0f, 0.0f);
    if (mRootComponent)
    {
        dir = mRootComponent->GetUpVector();
    }
    return dir;
}


void Actor::SetWorld(World * world)
{
    mWorld = world;
}

World* Actor::GetWorld()
{
    return mWorld;
}

void Actor::SetLevel(Level* level)
{
    mLevel = level;
}

Level* Actor::GetLevel()
{
    return mLevel;
}

std::vector<NetDatum>& Actor::GetReplicatedData()
{
    return mReplicatedData;
}

void Actor::SetNetId(NetId id)
{
    mNetId = id;
}

NetId Actor::GetNetId() const
{
    return mNetId;
}

NetHostId Actor::GetOwningHost() const
{
    return mOwningHost;
}

void Actor::SetOwningHost(NetHostId hostId)
{
    mOwningHost = hostId;
}

void Actor::SetReplicate(bool replicate)
{
    mReplicate = replicate;
}

bool Actor::IsReplicated() const
{
    return mReplicate;
}

void Actor::ForceReplication()
{
    mForceReplicate = true;
}

void Actor::ClearForcedReplication()
{
    mForceReplicate = false;
}

bool Actor::NeedsForcedReplication()
{
    return mForceReplicate;
}

ReplicationRate Actor::GetReplicationRate() const
{
    return mReplicationRate;
}

bool Actor::HasTag(const std::string& tag)
{
    bool hasTag = false;

    for (uint32_t i = 0; i < mTags.size(); ++i)
    {
        if (mTags[i] == tag)
        {
            hasTag = true;
            break;
        }
    }

    return hasTag;
}

void Actor::AddTag(const std::string& tag)
{
    if (!HasTag(tag))
    {
        mTags.push_back(tag);
    }
}

void Actor::RemoveTag(const std::string& tag)
{
    for (uint32_t i = 0; i < mTags.size(); ++i)
    {
        if (mTags[i] == tag)
        {
            mTags.erase(mTags.begin() + i);
            break;
        }
    }
}

const std::vector<Component*> Actor::GetComponents() const
{
    return mComponents;
}

void Actor::AddComponent(Component* component)
{
    assert(component);
    assert(component->GetOwner() == nullptr);

    mComponents.push_back(component);

    component->SetOwner(this);

    if (component->GetStaticType() == ScriptComponent::GetStaticType())
    {
        ++mNumScriptComps;
    }
}

void Actor::RemoveComponent(Component* component)
{
    int32_t index = -1;

    std::vector<Component*>& comps = mComponents;

    for (uint32_t i = 0; i < comps.size(); ++i)
    {
        if (comps[i] == component)
        {
            if (component->IsTransformComponent())
            {
                TransformComponent* transComp = (TransformComponent*)component;
                transComp->Attach(nullptr);
            }

            index = int32_t(i);
            comps.erase(comps.begin() + i);
            component->SetOwner(nullptr);
            break;
        }
    }

    if (component->GetStaticType() == ScriptComponent::GetStaticType())
    {
        assert(mNumScriptComps >= 1);
        --mNumScriptComps;
    }

    assert(index != -1); // Component not found?
}

uint32_t Actor::GetNumComponents() const
{
    return (uint32_t)mComponents.size();
}

Component* Actor::GetComponent(int32_t index)
{
    Component* ret = nullptr;

    if (index >= 0 &&
        index < (int32_t)mComponents.size())
    {
        ret = mComponents[index];
    }

    return ret;
}

Component* Actor::GetComponent(const std::string & name)
{
    Component* retComponent = nullptr;

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i]->GetName() == name)
        {
            retComponent = mComponents[i];
            break;
        }
    }

    return retComponent;
}

Component* Actor::GetComponentByType(TypeId type)
{
    Component* retComponent = nullptr;

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (mComponents[i]->GetType() == type)
        {
            retComponent = mComponents[i];
            break;
        }
    }

    return retComponent;
}

void Actor::CopyComponents(const std::vector<Component*>& srcComps)
{
    // Copy components
    for (uint32_t i = 0; i < srcComps.size(); ++i)
    {
        Component* srcComp = srcComps[i];
        Component* dstComp = nullptr;

        if (i >= mComponents.size())
        {
            dstComp = CreateComponent(srcComps[i]->GetType());
        }
        else
        {
            dstComp = mComponents[i];
        }
        
        dstComp->Copy(srcComp);
    }

    // Copy transform hierarchy
    for (uint32_t c = 0; c < srcComps.size(); ++c)
    {
        if (srcComps[c]->IsTransformComponent())
        {
            TransformComponent* srcTransComp = static_cast<TransformComponent*>(srcComps[c]);
            TransformComponent* dstTransComp = static_cast<TransformComponent*>(mComponents[c]);
            assert(mComponents[c]->IsTransformComponent());

            int32_t parentIndex = -1;

            if (srcTransComp->GetParent() != nullptr)
            {
                for (uint32_t p = 0; p < srcComps.size(); ++p)
                {
                    if (srcComps[p] == srcTransComp->GetParent())
                    {
                        parentIndex = (int32_t)p;
                        break;
                    }
                }
            }

            if (parentIndex != -1)
            {
                assert(mComponents[parentIndex]->IsTransformComponent());
                dstTransComp->Attach((TransformComponent*)mComponents[parentIndex]);
            }

            if (srcTransComp == srcTransComp->GetOwner()->GetRootComponent())
            {
                SetRootComponent(dstTransComp);
            }
        }
    }
}

void Actor::SetRootComponent(TransformComponent* component)
{
    if (component != nullptr)
    {
        if (component->GetParent() != nullptr)
        {
            component->Attach(nullptr);
        }

        if (mRootComponent != nullptr &&
            mRootComponent != component)
        {
            mRootComponent->Attach(component);
        }
    }

    mRootComponent = component;
}

TransformComponent* Actor::GetRootComponent()
{
    return mRootComponent;
}

void Actor::UpdateComponentTransforms()
{
    if (mRootComponent != nullptr)
    {
        mRootComponent->UpdateTransform(true);
    }
}

void Actor::SetHitCheckId(uint32_t id)
{
    mHitCheckId = id;
}

uint32_t Actor::GetHitCheckId() const
{
    return mHitCheckId;
}

void Actor::AddScriptEventHandler(ScriptComponent* scriptComp)
{
    assert(std::find(mEventHandlerScripts.begin(), mEventHandlerScripts.end(), scriptComp) == mEventHandlerScripts.end());
    mEventHandlerScripts.push_back(scriptComp);
}

void Actor::RemoveScriptEventHandler(ScriptComponent* scriptComp)
{
    auto it = std::find(mEventHandlerScripts.begin(), mEventHandlerScripts.end(), scriptComp);
    assert(it != mEventHandlerScripts.end());
    mEventHandlerScripts.erase(it);
}

uint32_t Actor::GetNumScriptComponents() const
{
    return uint32_t(mNumScriptComps);
}

bool Actor::DoComponentsHaveUniqueNames() const
{
    bool unique = true;
    std::unordered_set<std::string> names;

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (names.find(mComponents[i]->GetName()) != names.end())
        {
            unique = false;
            break;
        }
        else
        {
            names.insert(mComponents[i]->GetName());
        }
    }

    return unique;
}

Blueprint* Actor::GetBlueprintSource() const
{
    return mBlueprintSource.Get<Blueprint>();
}

void Actor::SetBlueprintSource(Blueprint* bp)
{
    mBlueprintSource = bp;
}

void Actor::InvokeNetFunc(const char* name)
{
    Datum** params = nullptr;
    INVOKE_NET_FUNC_BODY(0);
    if (shouldExecute) { netFunc->mFuncPointer.p0(this);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0)
{
    Datum* params[] = { &param0 };
    INVOKE_NET_FUNC_BODY(1);
    if (shouldExecute) { netFunc->mFuncPointer.p1(this, param0);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1)
{
    Datum* params[] = { &param0, &param1 };
    INVOKE_NET_FUNC_BODY(2);
    if (shouldExecute) { netFunc->mFuncPointer.p2(this, param0, param1);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2)
{
    Datum* params[] = { &param0, &param1, &param2 };
    INVOKE_NET_FUNC_BODY(3);
    if (shouldExecute) { netFunc->mFuncPointer.p3(this, param0, param1, param2);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3)
{
    Datum* params[] = { &param0, &param1, &param2, &param3 };
    INVOKE_NET_FUNC_BODY(4);
    if (shouldExecute) { netFunc->mFuncPointer.p4(this, param0, param1, param2, param3);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4)
{
    Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    INVOKE_NET_FUNC_BODY(5);
    if (shouldExecute) { netFunc->mFuncPointer.p5(this, param0, param1, param2, param3, param4);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5)
{
    Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    INVOKE_NET_FUNC_BODY(6);
    if (shouldExecute) { netFunc->mFuncPointer.p6(this, param0, param1, param2, param3, param4, param5);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6)
{
    Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    INVOKE_NET_FUNC_BODY(7);
    if (shouldExecute) { netFunc->mFuncPointer.p7(this, param0, param1, param2, param3, param4, param5, param6);}
}

void Actor::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6, Datum param7)
{
    Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    INVOKE_NET_FUNC_BODY(8);
    if (shouldExecute) { netFunc->mFuncPointer.p8(this, param0, param1, param2, param3, param4, param5, param6, param7);}
}

void Actor::RegisterNetFuncs(Actor* actor)
{
    TypeId actorType = actor->GetType();
    
    if (sTypeNetFuncMap.find(actorType) == sTypeNetFuncMap.end())
    {
        // First actor of this class spawned.
        // Need to gather up the RPCs and save them in static memory.

        sTypeNetFuncMap.insert({actorType, std::unordered_map<std::string, NetFunc>()});

        std::unordered_map<std::string, NetFunc>& funcMap = sTypeNetFuncMap[actorType];

        std::vector<NetFunc> netFuncs;
        actor->GatherNetFuncs(netFuncs);

        for (uint32_t i = 0; i < netFuncs.size(); ++i)
        {
            netFuncs[i].mIndex = (uint16_t) i;
            funcMap.insert({netFuncs[i].mName, netFuncs[i]});
        }

        //LogDebug("Registered %d net functions for class %s", int32_t(netFuncs.size()), actor->GetClassName());
    }
}

NetFunc* Actor::FindNetFunc(const char* name)
{
    NetFunc* retFunc = nullptr;

    TypeId actorType = GetType();
    auto mapIt = sTypeNetFuncMap.find(actorType);

    // The map should have been added when the first instanced of this class was spawned.
    // Checkout RegisterNetFuncs()
    assert(mapIt != sTypeNetFuncMap.end());

    if (mapIt != sTypeNetFuncMap.end())
    {
        auto funcIt = mapIt->second.find(name);

        // Did you remember to register the net func in GatherNetFuncs().
        // Is the function name spelled correctly?
        assert(funcIt != mapIt->second.end());

        if (funcIt != mapIt->second.end())
        {
            retFunc = &funcIt->second;
        }
    }

    return retFunc;
}

NetFunc* Actor::FindNetFunc(uint16_t index)
{
    NetFunc* retFunc = nullptr;

    TypeId actorType = GetType();
    auto mapIt = sTypeNetFuncMap.find(actorType);

    // The map should have been added when the first instanced of this class was spawned.
    // Checkout RegisterNetFuncs()
    assert(mapIt != sTypeNetFuncMap.end());

    if (mapIt != sTypeNetFuncMap.end())
    {
        for (auto funcIt = mapIt->second.begin(); funcIt != mapIt->second.end(); ++funcIt)
        {
            if (funcIt->second.mIndex == index)
            {
                retFunc = &funcIt->second;
                break;
            }
        }
    }

    return retFunc;
}

void Actor::SendNetFunc(NetFunc* func, uint32_t numParams, Datum** params)
{
    if (ShouldSendNetFunc(func->mType, this))
    {
        NetworkManager::Get()->SendInvokeMsg(this, func, numParams, params);
    }
}

void Actor::DestroyAllComponents()
{
    // Destroy components first, then delete the memory, in case a component
    // references a sibling component during its destruction.
    for (int32_t i = int32_t(mComponents.size()) - 1; i >= 0; --i)
    {
        mComponents[i]->Destroy();
    }

    for (int32_t i = int32_t(mComponents.size()) - 1; i >= 0; --i)
    {
        delete mComponents[i];
    }

    mComponents.clear();
}

void Actor::RenderSelected()
{
#if EDITOR
    const bool renderAll = (mWorld->IsComponentSelected(GetRootComponent()));
    const bool proxyEnabled = Renderer::Get()->IsProxyRenderingEnabled();

    for (int32_t i = 0; i < int32_t(mComponents.size()); ++i)
    {
        if (mComponents[i]->IsPrimitiveComponent())
        {
            PrimitiveComponent* primComp = static_cast<PrimitiveComponent*>(mComponents[i]);

            if (primComp->IsVisible() &&
                (renderAll || mWorld->IsComponentSelected(primComp)))
            {
                GFX_BindPipeline(PipelineId::Selected, primComp->GetVertexType());
                primComp->Render();
            }
        }

        if (proxyEnabled &&
            mComponents[i]->IsTransformComponent())
        {
            TransformComponent* transComp = static_cast<TransformComponent*>(mComponents[i]);

            if (transComp->IsVisible() &&
                renderAll || mWorld->IsComponentSelected(transComp))
            {
                std::vector<DebugDraw> proxyDraws;
                transComp->GatherProxyDraws(proxyDraws);

                for (DebugDraw& draw : proxyDraws)
                {
                    GFX_BindPipeline(PipelineId::Selected);
                    GFX_DrawStaticMesh(draw.mMesh, nullptr, draw.mTransform, draw.mColor);
                }
            }
        }
    }
#endif
}

Component* Actor::CreateComponent(TypeId compType)
{
    Component* subComp = Component::CreateInstance(compType);

    if (subComp != nullptr)
    {
        AddComponent(subComp);
        subComp->Create();

        if (subComp->IsTransformComponent())
        {
            TransformComponent* transComp = static_cast<TransformComponent*>(subComp);

            if (mRootComponent == nullptr)
            {
                // If no root component exists yet, make this transform comp the root
                SetRootComponent(transComp);
            }
            else
            {
                // Otherwise, attach it to the root.
                transComp->Attach(mRootComponent);
            }
        }

        if (HasBegunPlay())
        {
            subComp->BeginPlay();
        }
    }
    
    return subComp;
}

Component* Actor::CreateComponent(const char* typeName)
{
    Component* subComp = nullptr;
    const std::vector<Factory*>& factories = Component::GetFactoryList();
    for (uint32_t i = 0; i < factories.size(); ++i)
    {
        if (strncmp(typeName, factories[i]->GetClassName(), MAX_PATH_SIZE) == 0)
        {
            subComp = CreateComponent(factories[i]->GetType());
            break;
        }
    }

    return subComp;
}

Component* Actor::CloneComponent(Component* srcComp)
{
    Component* subComp = Component::CreateInstance(srcComp->GetType());

    if (subComp != nullptr)
    {
        AddComponent(subComp);
        subComp->Create();
        subComp->Copy(srcComp);

        if (HasBegunPlay())
        {
            subComp->BeginPlay();
        }
    }

    return subComp;
}

void Actor::DestroyComponent(Component* comp)
{
    RemoveComponent(comp);
    comp->Destroy();
    delete comp;
}

FORCE_LINK_DEF(Actor);
DEFINE_FACTORY_MANAGER(Actor);
DEFINE_FACTORY(Actor, Actor);
DEFINE_RTTI(Actor);
