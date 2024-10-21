#include "Nodes/Node.h"
#include "Log.h"
#include "World.h"
#include "Renderer.h"
#include "Clock.h"
#include "Enums.h"
#include "Maths.h"
#include "Utilities.h"
#include "Engine.h"
#include "Script.h"
#include "ObjectRef.h"
#include "NetworkManager.h"
#include "Assets/Scene.h"

#include "Nodes/3D/Node3d.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/Sphere3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/Audio3d.h"
#include "Nodes/3D/Primitive3d.h"
#include "Nodes/3D/Light3d.h"

#include "Graphics/Graphics.h"
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Node_Lua.h"

#if EDITOR
#include "EditorState.h"
#endif

#include <functional>
#include <algorithm>

#define INVOKE_NET_FUNC_BODY(P) \
    bool isScript = mScript && mScript->InvokeNetFunc(name, P, params); \
    bool shouldExecute = false; \
    NetFunc* netFunc = nullptr; \
    if (!isScript) \
    { \
        netFunc = FindNetFunc(name); \
        if (netFunc != nullptr) \
        { \
            OCT_ASSERT(netFunc && netFunc->mNumParams == P); \
            shouldExecute = ShouldExecuteNetFunc(netFunc->mType, this); \
            SendNetFunc(netFunc, P, params); \
        } \
        else \
        { \
            LogError("Can't find NetFunc %s on node %s", name, mName.c_str()); \
        } \
    }

std::unordered_map<TypeId, NetFuncMap> Node::sTypeNetFuncMap;

#define ENABLE_SCRIPT_FUNCS 1

DEFINE_SCRIPT_LINK_BASE(Node);

FORCE_LINK_DEF(Node);
DEFINE_FACTORY_MANAGER(Node);
DEFINE_FACTORY(Node, Node);
DEFINE_RTTI(Node);

bool Node::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);

    OCT_ASSERT(prop != nullptr);
    Node* node = static_cast<Node*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Script")
    {
        const std::string& newFileName = *((const std::string*)newValue);
        node->SetScriptFile(newFileName);

        success = true;
    }
    if (prop->mName == "Name")
    {
        const std::string& newName = *((const std::string*)newValue);
        node->SetName(newName);

        success = true;
    }
#if EDITOR
    if (prop->mName == "Restart Script")
    {
        bool restart = *((const bool*)newValue);
        if (restart && node->GetScript() != nullptr)
        {
            node->GetScript()->RestartScript();
        }
        success = true;
    }
    else if (prop->mName == "Reload Script File")
    {
        bool reload = *((const bool*)newValue);
        if (reload && node->GetScript() != nullptr)
        {
            node->GetScript()->ReloadScriptFile(node->GetScript()->GetFile(), true);
        }
        success = true;
    }
#endif

    return success;
}

bool Node::OnRep_OwningHost(Datum* datum, uint32_t index, const void* newValue)
{
    Node* node = (Node*)datum->mOwner;
    OCT_ASSERT(node != nullptr);

    NetHostId newHost = (NetHostId) *((uint8_t*)newValue);
    node->SetOwningHost(newHost);

    return true;
}


Node* Node::Construct(const std::string& name)
{
    Node* newNode = Node::CreateInstance(name.c_str());
    newNode->Create();
    return newNode;
}

Node* Node::Construct(TypeId typeId)
{
    Node* newNode = Node::CreateInstance(typeId);
    newNode->Create();
    return newNode;
}

void Node::Destruct(Node* node)
{
    if (node != nullptr)
    {
        // Recursively stop children first before calling Destroy
        auto stopNodeFunc = [](Node* node) -> bool
            {
                if (node->HasStarted())
                {
                    node->Stop();
                }

                return true;
            };

        node->Traverse(stopNodeFunc, true);

        node->Destroy();
        delete node;
    }
}

Node::Node()
{
    mName = "Node";
}

Node::~Node()
{

}

void Node::Create()
{
    REGISTER_SCRIPT_FUNCS();
}

void Node::Destroy()
{
    bool isWorldRoot = (mParent == nullptr && mWorld != nullptr);

    for (int32_t i = int32_t(GetNumChildren()) - 1; i >= 0; --i)
    {
        Node* child = GetChild(i);
        Node::Destruct(child);
    }

    if (IsPrimitive3D() && GetWorld())
    {
        GetWorld()->PurgeOverlaps(static_cast<Primitive3D*>(this));
    }

    if (mParent != nullptr)
    {
        Attach(nullptr);
    }

    if (mScript != nullptr)
    {
        delete mScript;
        mScript = nullptr;
    }

    // Unref userdata is it was created by Node_Lua::Create().
    if (mUserdataRef != LUA_REFNIL)
    {
        lua_State* L = GetLua();

        // Clear the userdata's mNode member.
        lua_rawgeti(L, LUA_REGISTRYINDEX, mUserdataRef);
        Node_Lua* nodeLua = (Node_Lua*) lua_touserdata(L, -1);
        nodeLua->mNode = nullptr;
        lua_pop(L, 1);

        luaL_unref(L, LUA_REGISTRYINDEX, mUserdataRef);
        mUserdataRef = LUA_REFNIL;
    }

    NodeRef::EraseReferencesToObject(this);

#if EDITOR
    GetEditorState()->HandleNodeDestroy(this);
#endif

    if (isWorldRoot)
    {
        Node* worldRoot = GetWorld()->GetRootNode();
        OCT_ASSERT(worldRoot == this);
        if (worldRoot == this)
        {
            GetWorld()->SetRootNode(nullptr);
        }
    }
}

void Node::SaveStream(Stream& stream, Platform platorm)
{
    // For serializing extra data besides properties
}

void Node::LoadStream(Stream& stream, Platform platorm, uint32_t version)
{
    // For serializing extra data besides properties
}

void Node::Copy(Node* srcNode, bool recurse)
{
    OCT_ASSERT(srcNode);
    OCT_ASSERT(srcNode->GetType() == GetType());

    if (srcNode == nullptr ||
        srcNode->GetType() != GetType())
    {
        LogError("Failed to copy node");
        return;
    }

    // I'm not using CopyPropertyValues() here because we need to handle the special
    // case where "Script" is copied. Should refactor this code so we can use the CopyPropertyValues() func.
    // Possibly, just copy over ScriptProperties separately after initial pass.
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
                break;
            }
        }

        if (dstProp != nullptr)
        {
            if (dstProp->IsVector())
            {
                dstProp->ResizeVector(srcProp->GetCount());
            }
            else
            {
                OCT_ASSERT(dstProp->mCount == srcProp->mCount);
            }

            dstProp->SetValue(srcProp->mData.vp, 0, srcProp->mCount);
        }

        // TODO-NODE: Gather properties if this uses a script.
        // For script components... if we first copy over the Filename property,
        // that will change the number of properties on the script so we need to regather them.
        // Script component is really the only component that can dynamically change its properties,
        // so I'm adding a hack now just for script component.
        if (srcProp->mName == "Script")
        {
            dstProps.clear();
            GatherProperties(dstProps);
        }
    }

    mScene = srcNode->GetScene();

    // Copy extra data
    Stream extraDataStream;
    srcNode->SaveStream(extraDataStream, Platform::Count);
    if (extraDataStream.GetSize() > 0)
    {
        extraDataStream.SetPos(0);
        LoadStream(extraDataStream, Platform::Count, ASSET_VERSION_CURRENT);
    }

    if (recurse)
    {

        for (uint32_t i = 0; i < srcNode->GetNumChildren(); ++i)
        {
            Node* srcChild = srcNode->GetChild(i);
            Node* dstChild = nullptr;

            if (i >= GetNumChildren())
            {
                LogError("Aborting recursive node copy. Mismatched number of children.");
                break;
                //dstChild = CreateChild(srcChild->GetType());
            }
            else
            {
                dstChild = GetChild(i);
            }

            OCT_ASSERT(dstChild);
            if (dstChild != nullptr)
            {
                dstChild->Copy(srcChild, recurse);
            }
        }
    }
}

void Node::Render(PipelineConfig pipelineConfig)
{
    // TODO-NODE: This function is used when rendering hit check and selected geometry I believe.
    // Could probably adjust Render() function in Primitive3D + Widget so that it can take a pipeline.
    // Or just manually bind the pipeline from the callers.
    if (IsPrimitive3D() && IsVisible())
    {
        Primitive3D* primComp = static_cast<Primitive3D*>(this);
        GFX_SetPipelineState(pipelineConfig);
        primComp->Render();
    }
}

void Node::Start()
{
    if (!mHasStarted && GetWorld() != nullptr)
    {
        mHasStarted = true;

        if (mReplicate)
        {
            // We still need to register net funcs even if Local so that the functions can execute locally.
            Node::RegisterNetFuncs(this);

            if (NetIsServer())
            {
                // The server will add the net node as soon as Start() is called.
                // On the client, the net node will be added once the NetMsgSpawnNode is executed.
                NetworkManager::Get()->AddNetNode(this, INVALID_NET_ID);

                // Send a reliable forced replication message to ensure the initial state
                // is received by the clients.
                ForceReplication();
            }
        }

        // TODO-NODE: Start children first? We could add a bool mLateStart.
        for (uint32_t i = 0; i < GetNumChildren(); ++i)
        {
            Node* child = GetChild(i);
            if (!child->HasStarted())
            {
                GetChild(i)->Start();
            }
        }

        if (mScript != nullptr)
        {
            mScript->CallFunction("Start");
        }
    }
}

void Node::Stop()
{
    if (mScript != nullptr)
    {
        mScript->CallFunction("Stop");
    }

    if (mNetId != INVALID_NET_ID)
    {
        // RemoveNetNode should be called whether or not it's the server.
        NetworkManager::Get()->RemoveNetNode(this);
    }

    mHasStarted = false;
}

void Node::RecursiveTick(float deltaTime, bool game)
{
    // TODO-NODE: Add a bool on Node for mLateTick.
    // If late tick is set, then tick after children tick.

    if (game && !mHasStarted)
    {
        Start();
    }

    if (IsTickEnabled() && IsActive() && !mPendingDestroy)
    {
        if (!mLateTick)
        {
            if (game)
            {
                Tick(deltaTime);
            }
            else
            {
                EditorTick(deltaTime);
            }
        }

        for (int32_t i = 0; i < (int32_t)GetNumChildren(); ++i)
        {
            Node* child = GetChild(i);
            child->RecursiveTick(deltaTime, game);

            if (child->IsPendingDestroy())
            {
                Node::Destruct(child);
                child = nullptr;
                --i;
            }
        }

        if (mLateTick)
        {
            if (game)
            {
                Tick(deltaTime);
            }
            else
            {
                EditorTick(deltaTime);
            }
        }

        if (mPendingDestroy && mParent == nullptr)
        {
            Destroy();
        }
    }
}

void Node::Tick(float deltaTime)
{
    TickCommon(deltaTime);
}

void Node::EditorTick(float deltaTime)
{
    TickCommon(deltaTime);
}

void Node::TickCommon(float deltaTime)
{
    if (mScript != nullptr)
    {
        mScript->Tick(deltaTime);
    }
}

void Node::Render()
{

}

VertexType Node::GetVertexType() const
{
    return VertexType::Max;
}

void Node::GatherProperties(std::vector<Property>& outProps)
{
    {
        SCOPED_CATEGORY("Node");

        outProps.push_back({ DatumType::String, "Name", this, &mName, 1, HandlePropChange });
#if EDITOR
        outProps.push_back(Property(DatumType::Bool, "Expose Variable", this, &mExposeVariable));
#endif
        outProps.push_back({ DatumType::Bool, "Active", this, &mActive });
        outProps.push_back({ DatumType::Bool, "Visible", this, &mVisible });
        outProps.push_back({ DatumType::Bool, "Late Tick", this, &mLateTick });

        outProps.push_back(Property(DatumType::Bool, "Replicate", this, &mReplicate));
        outProps.push_back(Property(DatumType::Bool, "Replicate Transform", this, &mReplicateTransform));
        outProps.push_back(Property(DatumType::String, "Tags", this, &mTags).MakeVector());
    }

    {
        SCOPED_CATEGORY("Script");

        outProps.push_back(Property(DatumType::String, "Script", this, &mScriptFile, 1, HandlePropChange));

#if EDITOR
        static bool sFakeBool = false;
        outProps.push_back(Property(DatumType::Bool, "Restart Script", this, &sFakeBool, 1, HandlePropChange));
        outProps.push_back(Property(DatumType::Bool, "Reload Script File", this, &sFakeBool, 1, HandlePropChange));
#endif
    }

    if (mScript != nullptr)
    {
        mScript->AppendScriptProperties(outProps);
    }
}

void Node::GatherReplicatedData(std::vector<NetDatum>& outData)
{
    outData.push_back(NetDatum(DatumType::Byte, this, &mOwningHost, 1, OnRep_OwningHost));
}

void Node::GatherNetFuncs(std::vector<NetFunc>& outFuncs)
{

}

void Node::GatherPropertyOverrides(std::vector<Property>& outOverrides)
{
    // TODO-NODE: Property overrides are only needed if we want to allow the user to change
    // child scene node properties while keeping the Scene connection. This could be useful so we 
    // might want to allow this, but to keep things simple for now, only the properties on the root
    // child scene node can be editted.
#if 0
    if (mScene != nullptr)
    {
        Scene* scene = mScene.Get<Scene>();

        std::vector<Property> nodeProps;
        GatherProperties(nodeProps);

        Node* defaultNode = Node::Construct(GetType());
        std::vector<Property> defaultProps;
        defaultNode->GatherProperties(defaultProps);

        for (uint32_t i = 0; i < nodeProps.size(); ++i)
        {
            const Property* scProp = scene->GetProperty(nodeProps[i].mName.c_str());
            const Property* defaultProp = FindProperty(defaultProps, nodeProps[i].mName);

            if ((scProp == nullptr && defaultProp == nullptr) ||           // Prop doesnt exist in BP or default
                (scProp != nullptr && nodeProps[i] != *scProp) ||         // Prop exists on bp but it's different
                (scProp == nullptr && defaultProp != nullptr && nodeProps[i] != *defaultProp)) // Prop exists on the default actor but it's different
            {
                outOverrides.push_back(nodeProps[i]);
            }
        }

        delete defaultNode;
        defaultNode = nullptr;
    }
#endif
}

void Node::ApplyPropertyOverrides(const std::vector<Property>& overs)
{
#if 1
    std::vector<Property> props;
    GatherProperties(props);
    CopyPropertyValues(props, overs);
#else
    std::vector<Property> props;
    GatherProperties(props);

    for (uint32_t i = 0; i < overs.size(); ++i)
    {
        for (uint32_t d = 0; d < props.size(); ++d)
        {
            if (props[d].mName == overs[i].mName &&
                props[d].mType == overs[i].mType &&
                props[d].mCount == overs[i].mCount)
            {
                props[d].SetValue(overs[i].mData.vp, 0, props[d].mCount);
                break;
            }
        }
    }
#endif
}

void Node::BeginOverlap(Primitive3D* thisNode, Primitive3D* otherNode)
{
    //LogDebug("Begin Overlap <%s> with <%s>", this->GetName().c_str(), other->GetName().c_str());

    // TODO-NODE: Get this working with scripts.
    if (mScript != nullptr)
    {
        mScript->BeginOverlap(thisNode, otherNode);
    }

    // Until "Signals" are implemented, the current method of handling collision/overlaps will be to bubble up the events.
    if (mParent != nullptr)
    {
        mParent->BeginOverlap(thisNode, otherNode);
    }
}
void Node::EndOverlap(Primitive3D* thisNode, Primitive3D* otherNode)
{
    //LogDebug("End Overlap <%s> with <%s>", this->GetName().c_str(), other->GetName().c_str());

    // TODO-NODE: Get this working with scripts.
    if (mScript != nullptr)
    {
        mScript->EndOverlap(thisNode, otherNode);
    }

    // Until "Signals" are implemented, the current method of handling collision/overlaps will be to bubble up the events.
    if (mParent != nullptr)
    {
        mParent->EndOverlap(thisNode, otherNode);
    }
}

void Node::OnCollision(
    Primitive3D* thisNode,
    Primitive3D* otherNode,
    glm::vec3 impactPoint,
    glm::vec3 impactNormal,
    btPersistentManifold* manifold)
{
    //LogDebug("Collisions [%d] <%s> with <%s>", manifold->getNumContacts(), this->GetName().c_str(), other->GetName().c_str());

    // TODO-NODE: Get this working with scripts.
    if (mScript != nullptr)
    {
        mScript->OnCollision(thisNode, otherNode, impactPoint, impactNormal, manifold);
    }

    // Until "Signals" are implemented, the current method of handling collision/overlaps will be to bubble up the events.
    if (mParent != nullptr)
    {
        mParent->OnCollision(thisNode, otherNode, impactPoint, impactNormal, manifold);
    }
}

void Node::RenderShadow()
{
#if 0
    Pipeline* shadowPipeline = Renderer::Get()->GetShadowCaster()->GetPipeline();

    for (uint32_t i = 0; i < mComponents.size(); ++i)
    {
        if (IsPrimitive3D() && IsVisible())
        {
            static_cast<Primitive3D*>(this)->RenderWithPipeline(shadowPipeline);
        }
    }
#endif
}

void Node::RenderSelected(bool renderChildren)
{
#if EDITOR
    if (!IsVisible())
        return;

    const bool proxyEnabled = Renderer::Get()->IsProxyRenderingEnabled();

    if (IsPrimitive3D())
    {
        Primitive3D* primComp = static_cast<Primitive3D*>(this);
        GFX_SetPipelineState(PipelineConfig::Selected);
        primComp->Render();
    }

    if (proxyEnabled && IsNode3D())
    {
        Node3D* transComp = static_cast<Node3D*>(this);

        std::vector<DebugDraw> proxyDraws;
        transComp->GatherProxyDraws(proxyDraws);

        for (DebugDraw& draw : proxyDraws)
        {
            GFX_SetPipelineState(PipelineConfig::Selected);
            GFX_DrawStaticMesh(draw.mMesh, nullptr, draw.mTransform, draw.mColor);
        }
    }

    if (renderChildren)
    {
        for (uint32_t i = 0; i < GetNumChildren(); ++i)
        {
            GetChild(i)->RenderSelected(true);
        }
    }
#endif
}

Node* Node::CreateChild(TypeId nodeType)
{
    Node* subNode = Node::Construct(nodeType);

    if (subNode != nullptr)
    {
        AddChild(subNode);

        if (HasStarted())
        {
            subNode->Start();
        }
    }
    else
    {
        LogError("Failed to create child node.");
    }

    return subNode;
}

Node* Node::CreateChild(const char* typeName)
{
    Node* subNode = nullptr;
    const std::vector<Factory*>& factories = Node::GetFactoryList();
    for (uint32_t i = 0; i < factories.size(); ++i)
    {
        if (strncmp(typeName, factories[i]->GetClassName(), MAX_PATH_SIZE) == 0)
        {
            subNode = CreateChild(factories[i]->GetType());
            break;
        }
    }

    return subNode;
}

Node* Node::CreateChildClone(Node* srcNode, bool recurse)
{
    Node* subNode = srcNode->Clone(recurse);
    AddChild(subNode);

    if (HasStarted())
    {
        subNode->Start();
    }

    return subNode;
}

Node* Node::Clone(bool recurse, bool instantiateLinkedScene)
{
    Node* clonedNode = nullptr;
    Scene* srcScene = instantiateLinkedScene ? GetScene() : nullptr;
    bool hasNativeChildren = false;

    if (srcScene != nullptr)
    {
        clonedNode = srcScene->Instantiate();
    }
    else
    {
        clonedNode = Node::Construct(GetType());

        if (clonedNode->GetNumChildren() > 0)
        {
            hasNativeChildren = true;
        }
    }

    if (clonedNode != nullptr)
    {
        // Might need to move copy after recurse block if properties aren't getting copied correctly.
        clonedNode->Copy(this, hasNativeChildren);

        if (recurse && !srcScene && !hasNativeChildren)
        {
            // Clone children
            for (uint32_t i = 0; i < GetNumChildren(); ++i)
            {
                // Note: instantiateLinkedScene is always defaulted to true here.
                // That option was added just for PIE when cloning the EditScene. Because
                // in that case we don't want to instantiate from the Scene. It may not be saved 
                // and we want to carry over the current state instead of what was last saved.
                Node* childClone = GetChild(i)->Clone(recurse);
                clonedNode->AddChild(childClone);
            }
        }

        if (HasStarted() && clonedNode->GetParent() != nullptr)
        {
            clonedNode->Start();
        }
    }

    return clonedNode;
}

void Node::DestroyChild(Node* childNode)
{
    // TODO-NODE: This working right? Destroy() should detach the node. And 
    // I think we want to still have the parent set while Stop() / Destroy() is called.
    Node::Destruct(childNode);
}

void Node::DestroyAllChildren()
{
    for (int32_t i = int32_t(GetNumChildren()) - 1; i >= 0; --i)
    {
        Node* child = GetChild(i);
        Node::Destruct(child);
    }
}

Node* Node::GetRoot()
{
    Node* root = this;

    if (GetParent() != nullptr)
    {
        root = GetParent()->GetRoot();
    }

    return root;
}

bool Node::IsWorldRoot() const
{
    bool isWorldRoot = mWorld && (mWorld->GetRootNode() == this);

    if (isWorldRoot)
    {
        OCT_ASSERT(mParent == nullptr);
    }

    return isWorldRoot;
}

void Node::SetPendingDestroy(bool pendingDestroy)
{
    mPendingDestroy = pendingDestroy;

    // Do we need to mark children as pending destroy? I think it could cause problems...
    // A parent node may be expecting its children to be alive during Tick(), but if they are 
    // already set to pending destroy (and the parent uses Late Tick) then the parent will tick 
    // after the children have already been destroyed.
#if 0
    for (uint32_t i = 0; i < GetNumChildren(); ++i)
    {
        GetChild(i)->SetPendingDestroy(pendingDestroy);
    }
#endif
}

bool Node::IsPendingDestroy() const
{
    return mPendingDestroy;
}

void Node::FlushPendingDestroys()
{
    for (int32_t i = 0; i < (int32_t)mChildren.size(); ++i)
    {
        Node* child = GetChild(i);
        if (child->IsPendingDestroy())
        {
            Node::Destruct(child);
            child = nullptr;
            --i;
        }
    }
}

bool Node::HasStarted() const
{
    return mHasStarted;
}

void Node::EnableTick(bool enable)
{
    mTickEnabled = enable;
}

bool Node::IsTickEnabled() const
{
    return mTickEnabled;
}

void Node::SetWorld(World * world)
{
    if (mWorld != world)
    {
        if (mWorld != nullptr)
        {
            mWorld->UnregisterNode(this);
        }

        mWorld = world;

        if (mWorld != nullptr)
        {
            mWorld->RegisterNode(this);
        }

        if (mScript != nullptr)
        {
            mScript->SetWorld(mWorld);
        }

        for (uint32_t i = 0; i < GetNumChildren(); ++i)
        {
            GetChild(i)->SetWorld(world);
        }
    }
}

World* Node::GetWorld()
{
    return mWorld;
}

void Node::SetScene(Scene* scene)
{
    mScene = scene;
}

Scene* Node::GetScene()
{
    return mScene.Get<Scene>();
}

std::vector<NetDatum>& Node::GetReplicatedData()
{
    return mReplicatedData;
}

void Node::SetNetId(NetId id)
{
    if (mNetId != id)
    {
        mNetId = id;

        if (mWorld != nullptr)
        {
            if (mNetId != INVALID_NET_ID)
            {
                // Ensure it exists in the replication vector
                mWorld->AddNodeToRepVector(this);
            }
            else
            {
                // Remove it from its world replication vector
                mWorld->RemoveNodeFromRepVector(this);
            }
        }
    }
}

NetId Node::GetNetId() const
{
    return mNetId;
}

NetHostId Node::GetOwningHost() const
{
    return mOwningHost;
}

void Node::SetOwningHost(NetHostId hostId)
{
    mOwningHost = hostId;

    if (mScript != nullptr)
    {
        mScript->CallFunction("OwnerChanged");
    }
}

void Node::SetReplicate(bool replicate)
{
    mReplicate = replicate;
}

bool Node::IsReplicated() const
{
    return mReplicate;
}

void Node::SetReplicateTransform(bool repTransform)
{
    mReplicateTransform = repTransform;
}

bool Node::IsTransformReplicated() const
{
    return mReplicateTransform;
}

void Node::ForceReplication()
{
    mForceReplicate = true;
}

void Node::ClearForcedReplication()
{
    mForceReplicate = false;
}

bool Node::NeedsForcedReplication()
{
    return mForceReplicate;
}

ReplicationRate Node::GetReplicationRate() const
{
    return mReplicationRate;
}

bool Node::HasTag(const std::string& tag)
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

void Node::AddTag(const std::string& tag)
{
    if (!HasTag(tag))
    {
        mTags.push_back(tag);
    }
}

void Node::RemoveTag(const std::string& tag)
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

void Node::SetName(const std::string& newName)
{
    if (mName != newName)
    {
        // Erase name from parent's child name map first.
        if (mParent != nullptr)
        {
            size_t elemRemoved = mParent->mChildNameMap.erase(mName);
            OCT_ASSERT(elemRemoved == 1);
        }

        mName = newName;

        if (mParent != nullptr)
        {
            mParent->ValidateUniqueChildName(this);
            mParent->mChildNameMap.insert({ mName, this });
        }
    }
}

const std::string& Node::GetName() const
{
    return mName;
}

void Node::SetActive(bool active)
{
    mActive = active;
}

bool Node::IsActive(bool recurse) const
{
    bool ret = mActive;

    if (recurse && mParent != nullptr)
    {
        ret = ret && mParent->IsActive(recurse);
    }

    return ret;
}

void Node::SetVisible(bool visible)
{
    mVisible = visible;
}

bool Node::IsVisible(bool recurse) const
{
    bool ret = mVisible;

    if (recurse && mParent != nullptr)
    {
        ret = ret && mParent->IsVisible(recurse);
    }

    return ret;
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

const char* Node::GetTypeName() const
{
    return "Node";
}

DrawData Node::GetDrawData()
{
    DrawData ret = {};
    ret.mNode = nullptr;
    ret.mMaterial = nullptr;
    return ret;
}

bool Node::IsNode3D() const
{
    return false;
}

bool Node::IsWidget() const
{
    return false;
}

bool Node::IsPrimitive3D() const
{
    return false;
}

bool Node::IsLight3D() const
{
    return false;
}

Node* Node::GetParent()
{
    return mParent;
}

const Node* Node::GetParent() const
{
    return mParent;
}

const std::vector<Node*>& Node::GetChildren() const
{
    return mChildren;
}

void Node::Attach(Node* parent, bool keepWorldTransform, int32_t index)
{
    // Can't attach to self.
    OCT_ASSERT(parent != this);
    if (parent == this)
    {
        return;
    }

    // Detach from parent first
    if (mParent != nullptr)
    {
        mParent->RemoveChild(this);
    }

    // Attach to new parent
    if (parent != nullptr)
    {
        parent->AddChild(this, index);
    }
}

void Node::Detach(bool keepWorldTransform)
{
    Attach(nullptr, keepWorldTransform);
}

void Node::AddChild(Node* child, int32_t index)
{
    if (child == this)
    {
        LogError("Cannot add a node as its own child.");
        return;
    }

    if (child != nullptr)
    {
        // TODO-NODE: Is this a good way to handle the world root getting attached elsewhere?
        // Another possibility is that we could make this node's new root node the world root?
        // Could cause problems anyway if the new root is already a different world's root. Idk.
        if (child->IsWorldRoot())
        {
            child->GetWorld()->SetRootNode(nullptr);
        }

        if (child->mParent != nullptr)
        {
            child->mParent->RemoveChild(child);
        }

        // Ensure unique name
        ValidateUniqueChildName(child);

        if (index >= 0 && index <= (int32_t)mChildren.size())
        {
            mChildren.insert(mChildren.begin() + index, child);
        }
        else
        {
            mChildren.push_back(child);
        }

#if EDITOR
        if (mParent != nullptr && mScene != nullptr)
        {
            BreakSceneLink();
        }
#endif

        mChildNameMap.insert({ child->GetName(), child });

        child->SetParent(this);
        child->SetWorld(mWorld);
    }
}

void Node::RemoveChild(Node* child)
{
    if (child != nullptr)
    {
        int32_t childIndex = -1;
        for (int32_t i = 0; i < int32_t(mChildren.size()); ++i)
        {
            if (mChildren[i] == child)
            {
                childIndex = i;
                break;
            }
        }

        OCT_ASSERT(childIndex != -1); // Could not find the component to remove
        if (childIndex != -1)
        {
            RemoveChild(childIndex);

#if EDITOR
            if (mParent != nullptr && mScene != nullptr)
            {
                BreakSceneLink();
            }
#endif
        }
    }
}

void Node::RemoveChild(int32_t index)
{
    OCT_ASSERT(index >= 0 && index < int32_t(mChildren.size()));
    if (index >= 0 && index < int32_t(mChildren.size()))
    {
        Node* child = mChildren[index];

        child->SetWorld(nullptr);

        child->SetParent(nullptr);
        mChildren.erase(mChildren.begin() + index);

        // This child's name should be in the map. When a node is renamed, the parent's map needs to be udpated.
        size_t elemRemoved = mChildNameMap.erase(child->GetName());
        OCT_ASSERT(elemRemoved == 1);
    }
}

int32_t Node::FindChildIndex(const std::string& name) const
{
    int32_t index = -1;
    for (int32_t i = 0; i < int32_t(mChildren.size()); ++i)
    {
        if (mChildren[i]->GetName() == name)
        {
            index = i;
            break;
        }
    }

    return index;
}

int32_t Node::FindChildIndex(Node* child) const
{
    int32_t index = -1;

    for (int32_t i = 0; i < int32_t(mChildren.size()); ++i)
    {
        if (mChildren[i] == child)
        {
            index = i;
            break;
        }
    }

    return index;
}


Node* Node::FindChild(const std::string& name, bool recurse) const
{
    Node* retNode = nullptr;

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i]->GetName() == name)
        {
            retNode = mChildren[i];
            break;
        }
    }

    if (recurse &&
        retNode == nullptr)
    {
        for (uint32_t i = 0; i < mChildren.size(); ++i)
        {
            retNode = mChildren[i]->FindChild(name, recurse);

            if (retNode != nullptr)
            {
                break;
            }
        }
    }

    return retNode;
}

Node* Node::FindChildWithTag(const std::string& tag, bool recurse) const
{
    Node* retNode = nullptr;

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i]->HasTag(tag))
        {
            retNode = mChildren[i];
            break;
        }
    }

    if (recurse &&
        retNode == nullptr)
    {
        for (uint32_t i = 0; i < mChildren.size(); ++i)
        {
            retNode = mChildren[i]->FindChildWithTag(tag, recurse);

            if (retNode != nullptr)
            {
                break;
            }
        }
    }

    return retNode;
}

Node* Node::FindDescendant(const std::string& name)
{
    return FindChild(name, true);
}

Node* Node::FindAncestor(const std::string& name)
{
    Node* ret = nullptr;

    if (mParent != nullptr)
    {
        if (mParent->GetName() == name)
        {
            ret = mParent;
        }
        else
        {
            ret = mParent->FindAncestor(name);
        }
    }

    return ret;
}

bool Node::HasAncestor(Node* node)
{
    bool hasAncestor = false;
    if (mParent != nullptr)
    {
        if (mParent == node)
        {
            hasAncestor = true;
        }
        else
        {
            hasAncestor = mParent->HasAncestor(node);
        }
    }

    return hasAncestor;
}

Node* Node::GetChild(int32_t index) const
{
    Node* retNode = nullptr;
    if (index >= 0 &&
        index < (int32_t)mChildren.size())
    {
        retNode = mChildren[index];
    }
    return retNode;
}

Node* Node::GetChildByType(TypeId type) const
{
    Node* ret = nullptr;

    for (uint32_t i = 0; i < GetNumChildren(); ++i)
    {
        Node* child = GetChild(i);
        if (child->GetType() == type)
        {
            ret = child;
            break;
        }
    }

    return ret;
}

uint32_t Node::GetNumChildren() const
{
    return (uint32_t)mChildren.size();
}

int32_t Node::FindParentNodeIndex() const
{
    int32_t retIndex = -1;

    if (mParent != nullptr)
    {
        const std::vector<Node*>& children = mParent->GetChildren();
        for (uint32_t i = 0; i < children.size(); ++i)
        {
            if (children[i] == mParent)
            {
                retIndex = i;
                break;
            }
        }
    }

    return retIndex;
}

void Node::SetHitCheckId(uint32_t id)
{
    mHitCheckId = id;
}

uint32_t Node::GetHitCheckId() const
{
    return mHitCheckId;
}

bool Node::IsLateTickEnabled() const
{
    return mLateTick;
}

void Node::EnableLateTick(bool enable)
{
    mLateTick = enable;
}

Script* Node::GetScript()
{
    return mScript;
}

void Node::SetScriptFile(const std::string& fileName)
{
    mScriptFile = fileName;

    if (fileName != "" && mScript == nullptr)
    {
        mScript = new Script(this);
    }

    if (mScript != nullptr &&
        mScript->GetFile() != fileName)
    {
        mScript->SetFile(fileName.c_str());
        mScript->RestartScript();

        if (HasStarted())
        {
            mScript->CallFunction("Start");
        }
    }
}

bool Node::DoChildrenHaveUniqueNames() const
{
    bool unique = true;
    std::unordered_set<std::string> names;

    for (uint32_t i = 0; i < GetNumChildren(); ++i)
    {
        if (names.find(GetChild(i)->GetName()) != names.end())
        {
            unique = false;
            break;
        }
        else
        {
            names.insert(GetChild(i)->GetName());
        }
    }

    return unique;
}

void Node::BreakSceneLink()
{
    mScene = nullptr;
}

bool Node::IsSceneLinked() const
{
    return (mScene != nullptr && mParent != nullptr);
}

bool Node::IsForeign() const
{
    // A node considered "Foreign" is one that has an ancestor which is scene-linked.
    // The scene-linked ancestor itself is not considered foreign unless it is also 
    // has a different scene-linked ancestor.
    bool foreign = mParent && mParent->IsSceneLinked();

    if (!foreign && mParent != nullptr)
    {
        foreign = mParent->IsForeign();
    }

    return foreign;
}

bool Node::HasAuthority() const
{
    return NetIsAuthority();
}

int Node::GetUserdataRef() const
{
    return mUserdataRef;
}

void Node::SetUserdataRef(int ref)
{
    // This should only be called once.
    OCT_ASSERT(mUserdataRef == LUA_REFNIL);

    mUserdataRef = ref;
}

bool Node::IsOwned() const
{
    return (NetIsLocal() || mOwningHost == NetGetHostId());
}

void Node::InvokeNetFunc(const char* name)
{
    const Datum** params = nullptr;
    INVOKE_NET_FUNC_BODY(0);
    if (shouldExecute) { netFunc->mFuncPointer.p0(this); }
}

void Node::InvokeNetFunc(const char* name, Datum param0)
{
    const Datum* params[] = { &param0 };
    INVOKE_NET_FUNC_BODY(1);
    if (shouldExecute) { netFunc->mFuncPointer.p1(this, param0); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1)
{
    const Datum* params[] = { &param0, &param1 };
    INVOKE_NET_FUNC_BODY(2);
    if (shouldExecute) { netFunc->mFuncPointer.p2(this, param0, param1); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2)
{
    const Datum* params[] = { &param0, &param1, &param2 };
    INVOKE_NET_FUNC_BODY(3);
    if (shouldExecute) { netFunc->mFuncPointer.p3(this, param0, param1, param2); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3 };
    INVOKE_NET_FUNC_BODY(4);
    if (shouldExecute) { netFunc->mFuncPointer.p4(this, param0, param1, param2, param3); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4 };
    INVOKE_NET_FUNC_BODY(5);
    if (shouldExecute) { netFunc->mFuncPointer.p5(this, param0, param1, param2, param3, param4); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5 };
    INVOKE_NET_FUNC_BODY(6);
    if (shouldExecute) { netFunc->mFuncPointer.p6(this, param0, param1, param2, param3, param4, param5); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6 };
    INVOKE_NET_FUNC_BODY(7);
    if (shouldExecute) { netFunc->mFuncPointer.p7(this, param0, param1, param2, param3, param4, param5, param6); }
}

void Node::InvokeNetFunc(const char* name, Datum param0, Datum param1, Datum param2, Datum param3, Datum param4, Datum param5, Datum param6, Datum param7)
{
    const Datum* params[] = { &param0, &param1, &param2, &param3, &param4, &param5, &param6, &param7 };
    INVOKE_NET_FUNC_BODY(8);
    if (shouldExecute) { netFunc->mFuncPointer.p8(this, param0, param1, param2, param3, param4, param5, param6, param7); }
}

void Node::InvokeNetFunc(const char* name, const std::vector<Datum>& params)
{
    uint32_t numParams = (uint32_t)params.size();
    if (numParams > 8)
    {
        LogError("Too many params for NetFunc, truncating to 8.");
        numParams = 8;
    }

    switch (numParams)
    {
    case 0: InvokeNetFunc(name); break;
    case 1: InvokeNetFunc(name, params[0]); break;
    case 2: InvokeNetFunc(name, params[0], params[1]); break;
    case 3: InvokeNetFunc(name, params[0], params[1], params[2]); break;
    case 4: InvokeNetFunc(name, params[0], params[1], params[2], params[3]); break;
    case 5: InvokeNetFunc(name, params[0], params[1], params[2], params[3], params[4]); break;
    case 6: InvokeNetFunc(name, params[0], params[1], params[2], params[3], params[4], params[5]); break;
    case 7: InvokeNetFunc(name, params[0], params[1], params[2], params[3], params[4], params[5], params[6]); break;
    case 8: InvokeNetFunc(name, params[0], params[1], params[2], params[3], params[4], params[5], params[6], params[7]); break;
    }
}

void Node::RegisterNetFuncs(Node* node)
{
    TypeId nodeType = node->GetType();

    if (sTypeNetFuncMap.find(nodeType) == sTypeNetFuncMap.end())
    {
        // First actor of this class spawned.
        // Need to gather up the RPCs and save them in static memory.

        sTypeNetFuncMap.insert({ nodeType, std::unordered_map<std::string, NetFunc>() });

        std::unordered_map<std::string, NetFunc>& funcMap = sTypeNetFuncMap[nodeType];

        std::vector<NetFunc> netFuncs;
        node->GatherNetFuncs(netFuncs);

        for (uint32_t i = 0; i < netFuncs.size(); ++i)
        {
            netFuncs[i].mIndex = (uint16_t)i;
            funcMap.insert({ netFuncs[i].mName, netFuncs[i] });
        }

        //LogDebug("Registered %d net functions for class %s", int32_t(netFuncs.size()), actor->GetClassName());
    }
}

NetFunc* Node::FindNetFunc(const char* name)
{
    NetFunc* retFunc = nullptr;

    TypeId actorType = GetType();
    auto mapIt = sTypeNetFuncMap.find(actorType);

    if (mapIt == sTypeNetFuncMap.end())
    {
        // We haven't registered net funcs yet, so do that now!
        Node::RegisterNetFuncs(this);

        mapIt = sTypeNetFuncMap.find(actorType);
    }

    // The map should have been added when the first instanced of this class was spawned.
    // Checkout RegisterNetFuncs()
    OCT_ASSERT(mapIt != sTypeNetFuncMap.end());

    if (mapIt != sTypeNetFuncMap.end())
    {
        auto funcIt = mapIt->second.find(name);

        // Did you remember to register the net func in GatherNetFuncs().
        // Is the function name spelled correctly?
        OCT_ASSERT(funcIt != mapIt->second.end());

        if (funcIt != mapIt->second.end())
        {
            retFunc = &funcIt->second;
        }
    }

    return retFunc;
}

NetFunc* Node::FindNetFunc(uint16_t index)
{
    NetFunc* retFunc = nullptr;

    TypeId actorType = GetType();
    auto mapIt = sTypeNetFuncMap.find(actorType);

    // The map should have been added when the first instanced of this class was spawned.
    // Checkout RegisterNetFuncs()
    OCT_ASSERT(mapIt != sTypeNetFuncMap.end());

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

void Node::SetParent(Node* parent)
{
    mParent = parent;
}

void Node::ValidateUniqueChildName(Node* newChild)
{
    static uint32_t sUniqueId = 1;

    bool validName = (mChildNameMap.find(newChild->GetName()) == mChildNameMap.end());

    if (!validName)
    {
        std::string name = newChild->GetName();

        if (IsPlaying())
        {
            // In game, just spit out a unique string.
            // For now, just use # at the end of name and increment a static counter.
            name = name + "#" + std::to_string(sUniqueId);
            sUniqueId++;
            validName = true;
        }
        else
        {
            // In editor, we want to make the name more readable.
            const int32_t kMaxRenameTries = 1000;
            int32_t renameTry = 0;

            while (!validName)
            {
                // 1 - Determine number at end of current name and remove it from name string
                size_t lastCharIndex = name.find_last_not_of("0123456789");

                uint32_t num = 1;
                if (lastCharIndex == std::string::npos)
                {
                    // The name is entirely numbers?
                    num = (uint32_t)std::stoul(name);
                    name = "";
                }
                else if (lastCharIndex + 1 < name.size())
                {
                    std::string numStr = name.substr(lastCharIndex + 1);
                    name = name.substr(0, lastCharIndex + 1);

                    num = (uint32_t)std::stoul(numStr);
                }

                num++;
                name = name + std::to_string(num);

                validName = (mChildNameMap.find(name) == mChildNameMap.end());

                renameTry++;

                // If exceeded max renames, just use sUniqueId to make unique name like in-game.
                if (renameTry > kMaxRenameTries)
                {
                    name = newChild->GetName() + "#" + std::to_string(sUniqueId);
                    sUniqueId++;

                    validName = true;
                    break;
                }
            }
        }

        OCT_ASSERT(validName);
        // Don't call SetName() here because that will trigger another ValidateUniqueChildName() call.
        newChild->mName = name;
    }
}

void Node::SendNetFunc(NetFunc* func, uint32_t numParams, const Datum** params)
{
    if (ShouldSendNetFunc(func->mType, this))
    {
        NetworkManager::Get()->SendInvokeMsg(this, func, numParams, params);
    }
}

#if EDITOR

bool Node::ShouldExposeVariable() const
{
    return mExposeVariable;
}

void Node::SetExposeVariable(bool expose)
{
    mExposeVariable = expose;
}

#endif
