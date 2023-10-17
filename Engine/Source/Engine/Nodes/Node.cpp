#include "Nodes/Node.h"
#include "Actor.h"
#include "Log.h"
#include "World.h"
#include "ObjectRef.h"

#include "Nodes/3D/TransformComponent.h"
#include "Nodes/3D/StaticMeshComponent.h"
#include "Nodes/3D/SkeletalMeshComponent.h"
#include "Nodes/3D/PointLightComponent.h"
#include "Nodes/3D/DirectionalLightComponent.h"
#include "Nodes/3D/CameraComponent.h"
#include "Nodes/3D/BoxComponent.h"
#include "Nodes/3D/SphereComponent.h"
#include "Nodes/3D/ParticleComponent.h"
#include "Nodes/3D/AudioComponent.h"

FORCE_LINK_DEF(Node);
DEFINE_FACTORY_MANAGER(Node);
DEFINE_FACTORY(Node, Node);
DEFINE_RTTI(Node);

Node::Node()
{
    mName = "Node";
}

Node::~Node()
{

}

void Node::Create()
{

}

void Node::Destroy()
{
    NodeRef::EraseReferencesToObject(this);

    if (IsPrimitiveNode() && GetWorld())
    {
        GetWorld()->PurgeOverlaps(static_cast<PrimitiveComponent*>(this));
    }

#if EDITOR
    GetWorld()->DeselectComponent(this);
#endif
}

void Node::SaveStream(Stream& stream)
{
    // TODO-NODE: Can we just entirely remove Save/LoadStream from Nodes 
    // and just serialize the properties? Could simplify things. Or instead of
    // totally removing Save/LoadStream(), still allow nodes to override it
    // but just delete all of the stuff that could be serialized by properties.

    stream.WriteString(mName);
    stream.WriteBool(mActive);
    stream.WriteBool(mVisible);

    // Tags
    OCT_ASSERT(mTags.size() <= 255);
    uint32_t numTags = glm::min((uint32_t)mTags.size(), 255u);
    stream.WriteUint8(numTags);

    for (uint32_t i = 0; i < numTags; ++i)
    {
        stream.WriteString(mTags[i]);
    }

    stream.WriteBool(mReplicate);
    stream.WriteBool(mReplicateTransform);

    // TODO-NODE: Script serailization? Possibly just serialize-by-properties.
}

void Node::LoadStream(Stream& stream)
{
    // TODO-NODE: Remove old data loading after serializing everything.
#if 1
    // Load old data
    stream.ReadString(mName);
    mActive = stream.ReadBool();
    mVisible = stream.ReadBool();
#else
    // Load new data
    stream.ReadString(mName);
    mActive = stream.ReadBool();
    mVisible = stream.ReadBool();


    uint32_t numTags = (uint32_t)stream.ReadUint8();
    mTags.resize(numTags);
    for (uint32_t i = 0; i < numTags; ++i)
    {
        stream.ReadString(mTags[i]);
    }

    mReplicate = stream.ReadBool();
    mReplicateTransform = stream.ReadBool();

    // TODO-NODE: Script serailization? Possibly just serialize-by-properties.
#endif
}

void Node::Copy(Node* srcNode)
{
    OCT_ASSERT(srcNode);
    OCT_ASSERT(srcNode->GetType() == GetType());

    // Copy actor properties
    std::vector<Property> srcProps;
    srcNode->GatherProperties(srcProps);

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


        // TODO-NODE: Gather properties if this uses a script.
        // For script components... if we first copy over the Filename property,
        // that will change the number of properties on the script so we need to regather them.
        // Script component is really the only component that can dynamically change its properties,
        // so I'm adding a hack now just for script component.
        if (srcProp->mName == "Filename")
        {
            dstProps.clear();
            GatherProperties(dstProps);
        }
    }
}

void Node::Start()
{

}

void Node::Stop()
{

}

void Node::Tick(float deltaTime)
{
    
}

void Node::EditorTick(float deltaTime)
{
    Tick(deltaTime);
}

void Node::GatherProperties(std::vector<Property>& outProps)
{
    outProps.push_back({DatumType::String, "Name", this, &mName});
    outProps.push_back({DatumType::Bool, "Active", this, &mActive});
    outProps.push_back({DatumType::Bool, "Visible", this, &mVisible});

    outProps.push_back(Property(DatumType::Bool, "Persistent", this, &mPersistent));
    outProps.push_back(Property(DatumType::Bool, "Replicate", this, &mReplicate));
    outProps.push_back(Property(DatumType::Bool, "Replicate Transform", this, &mReplicateTransform));
    outProps.push_back(Property(DatumType::String, "Tags", this, &mTags).MakeVector());
}

// TODO-NODE: Register / unregister should happen on AddChild() / SetParent()
// If the prev world != newWorld, set its world and also call register/unregister.
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

void Node::SetName(const std::string& newName)
{
    mName = newName;
}

const std::string& Node::GetName() const
{
    return mName;
}

void Node::SetActive(bool active)
{
    mActive = active;
}

bool Node::IsActive() const
{
    return mActive;
}

void Node::SetVisible(bool visible)
{
    mVisible = visible;
}

bool Node::IsVisible() const
{
    return mVisible;
}

void Node::SetTransient(bool transient)
{
    mTransient = transient;
}

bool Node::IsTransient() const
{
    return mTransient;
}

void Node::SetDefault(bool isDefault)
{
    mDefault = isDefault;
}

bool Node::IsDefault() const
{
    return mDefault;
}

World* Node::GetWorld()
{
    return mWorld;
}

const char* Node::GetTypeName() const
{
    return "Node";
}

DrawData Node::GetDrawData()
{
    DrawData ret = {};
    ret.mComponent = nullptr;
    ret.mMaterial = nullptr;
    return ret;
}

bool Node::IsTransformNode() const
{
    return false;
}

bool Node::IsPrimitiveNode() const
{
    return false;
}

bool Node::IsLightNode() const
{
    return false;
}
