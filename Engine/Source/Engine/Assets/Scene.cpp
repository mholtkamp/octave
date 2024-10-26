#include "Assets/Scene.h"
#include "World.h"
#include "Log.h"
#include "Engine.h"
#include "Script.h"
#include "NetworkManager.h"
#include "Nodes/Node.h"
#include "Nodes/3D/SkeletalMesh3d.h"

FORCE_LINK_DEF(Scene);
DEFINE_ASSET(Scene);

static const char* sFogDensityStrings[] =
{
    "Linear",
    "Exponential",
};
static_assert(int32_t(FogDensityFunc::Count) == 2, "Need to update string conversion table");


bool Scene::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    bool success = false;
#if EDITOR
    Property* prop = static_cast<Property*>(datum);
    Scene* scene = (Scene*)prop->mOwner;

    // Just set value anyway
    datum->SetValueRaw(newValue, index);
    World* world = GetWorld(0);
    if (world != nullptr &&
        world->GetRootNode() != nullptr &&
        world->GetRootNode()->GetScene() == scene)
    {
        world->UpdateRenderSettings();
    }

#endif
    return success;
}


Scene::Scene()
{
    mType = Scene::GetStaticType();
}

Scene::~Scene()
{

}

void Scene::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    uint32_t numNodeDefs = stream.ReadUint32();
    OCT_ASSERT(numNodeDefs < 65535); // Something reasonable?
    mNodeDefs.resize(numNodeDefs);

    for (uint32_t i = 0; i < numNodeDefs; ++i)
    {
        SceneNodeDef& def = mNodeDefs[i];

        def.mType = (TypeId)stream.ReadUint32();
        def.mParentIndex = stream.ReadInt32();

        stream.ReadAsset(def.mScene);
        stream.ReadString(def.mName);
        def.mExposeVariable = stream.ReadBool();
        def.mParentBone = stream.ReadInt8();

        uint32_t numProps = stream.ReadUint32();
        def.mProperties.resize(numProps);
        for (uint32_t p = 0; p < numProps; ++p)
        {
            def.mProperties[p].ReadStream(stream, false);
        }

        if (mVersion >= ASSET_VERSION_SCENE_EXTRA_DATA)
        {
            uint32_t extraDataSize = stream.ReadUint32();
            if (extraDataSize > 0)
            {
                def.mExtraData.resize(extraDataSize);
                stream.ReadBytes(def.mExtraData.data(), extraDataSize);
            }
        }
    }

    // World render properties
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
}

void Scene::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    Scene* srcScene = this;

    if (platform != Platform::Count)
    {
        // Capture the scene for the given platform. This was originally
        // added so we can unroll InstancedMesh3D nodes on platforms that
        // don't suppose instanced mesh rendering.
        srcScene = NewTransientAsset<Scene>();
        Node* tempRoot = Instantiate();
        srcScene->Capture(tempRoot, platform);
        Node::Destruct(tempRoot);

        // These transient scene that was created should get garbage collected since 
        // no AssetRefs are pointing to it.
    }

    stream.WriteUint32((uint32_t)srcScene->mNodeDefs.size());

    for (uint32_t i = 0; i < srcScene->mNodeDefs.size(); ++i)
    {
        const SceneNodeDef& def = srcScene->mNodeDefs[i];
        stream.WriteUint32((uint32_t)def.mType);
        stream.WriteInt32(def.mParentIndex);

        stream.WriteAsset(def.mScene);
        stream.WriteString(def.mName);
        stream.WriteBool(def.mExposeVariable);
        stream.WriteInt8(def.mParentBone);

        stream.WriteUint32((uint32_t)def.mProperties.size());
        for (uint32_t p = 0; p < def.mProperties.size(); ++p)
        {
            def.mProperties[p].WriteStream(stream);
        }

        stream.WriteUint32((uint32_t)def.mExtraData.size());
        stream.WriteBytes(def.mExtraData.data(), (uint32_t)def.mExtraData.size());
    }

    // Now that we've written out the platform-cooked scene data, write out the rest of the data for this scene
    // from this instance itself. This data is platform independent.
    srcScene = nullptr;

    // World render properties
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
}

void Scene::Create()
{
    Asset::Create();
}

void Scene::Destroy()
{
    Asset::Destroy();
}

void Scene::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

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

glm::vec4 Scene::GetTypeColor()
{
    return glm::vec4(0.9f, 0.8f, 0.2f, 1.0f);
    //return glm::vec4(0.4f, 0.4f, 1.0f, 1.0f);
}

const char* Scene::GetTypeName()
{
    return "Scene";
}

void Scene::Capture(Node* root, Platform platform)
{
    mNodeDefs.clear();

    if (root == nullptr)
        return;

    std::vector<Node*> nodeList;
    AddNodeDef(root, platform, nodeList);
}

Node* Scene::Instantiate()
{
    Node* rootNode = nullptr;

    if (mNodeDefs.size() > 0)
    {
        std::vector<Node*> nodeList;

        // The nativeChildren vector holds a list of all children by created in C++ for the nodes in this scene.
        // If there is no SceneNodeDef for the nativeChild, then we must destroy it. This will happen
        // if the user renames a native child, then we will have a duplicate so we need to destroy the native one.
        std::vector<Node*> nativeChildren;

        for (uint32_t i = 0; i < mNodeDefs.size(); ++i)
        {
            Node* node = nullptr;
            Node* parent = (i > 0) ? nodeList[mNodeDefs[i].mParentIndex] : nullptr;

            if (parent != nullptr)
            {
                // See if the node already exists. This can happen if lets say,
                // the root node spawned other nodes on Create() in C++.
                Node* existingChild = parent->FindChild(mNodeDefs[i].mName, false);

                if (existingChild != nullptr &&
                    existingChild->GetType() == mNodeDefs[i].mType &&
                    existingChild->GetScene() == mNodeDefs[i].mScene.Get())
                {
                    node = existingChild;
                }

                if (node != nullptr)
                {
                    // Double check that we found a natively spawned child.
                    // Otherwise do we have conflicting SceneNodeDefs with same name?!
                    bool isNativeChild = false;
                    for (uint32_t n = 0; n < nativeChildren.size(); ++n)
                    {
                        if (nativeChildren[n] == node)
                        {
                            nativeChildren.erase(nativeChildren.begin() + n);
                            isNativeChild = true;

                            // Add this node's children as new native child nodes.
                            for (uint32_t c = 0; c < node->GetNumChildren(); ++c)
                            {
                                nativeChildren.push_back(node->GetChild(c));
                            }

                            break;
                        }
                    }

                    OCT_ASSERT(isNativeChild);
                }
            }

            // We aren't overriding a native child, so we need to create a new node.
            if (node == nullptr)
            {
                if (mNodeDefs[i].mScene != nullptr)
                {
                    Scene* scene = mNodeDefs[i].mScene.Get<Scene>();
                    node = scene->Instantiate();

#if EDITOR
                    node->SetExposeVariable(mNodeDefs[i].mExposeVariable);
#endif
                }
                else
                {
                    node = Node::Construct(mNodeDefs[i].mType);

                    for (uint32_t c = 0; c < node->GetNumChildren(); ++c)
                    {
                        nativeChildren.push_back(node->GetChild(c));
                    }
                }
            }

            OCT_ASSERT(node);

            std::vector<Property> dstProps;
            node->GatherProperties(dstProps);
            CopyPropertyValues(dstProps, mNodeDefs[i].mProperties);

            if (mNodeDefs[i].mExtraData.size() > 0)
            {
                Stream extraStream((char*)mNodeDefs[i].mExtraData.data(), (uint32_t)mNodeDefs[i].mExtraData.size());
                node->LoadStream(extraStream, GetPlatform(), mVersion);
            }

            // If this node has a script, then it might have script properties, and those
            // won't exist in the properties until the "Script File" property was assigned during the
            // copy we just did. So to copy all of the script properties we need to gather + copy them a second time.
            // During the second gather, node->mScript will be non-null and thus we can get the default script values that 
            // we will now override during the second copy.
            if (node->GetScript() != nullptr)
            {
                dstProps.clear();
                node->GatherProperties(dstProps);
                CopyPropertyValues(dstProps, mNodeDefs[i].mProperties);
            }

            if (i > 0)
            {
                OCT_ASSERT(parent != nullptr);

                // Note: We call AddChild even if the node already existed natively to ensure the order matches scene order.
                if (mNodeDefs[i].mParentBone >= 0)
                {
                    SkeletalMesh3D* parentSk = parent->As<SkeletalMesh3D>();

                    OCT_ASSERT(node->IsNode3D());
                    OCT_ASSERT(parentSk != nullptr);
                    if (node->IsNode3D())
                    {
                        Node3D* node3d = static_cast<Node3D*>(node);
                        node3d->AttachToBone(parentSk, mNodeDefs[i].mParentBone, false);
                    }
                }
                else
                {
                    parent->AddChild(node);
                }

                if (mNodeDefs[i].mExposeVariable)
                {
                    Script* rootScript = nodeList[0]->GetScript();
                    if (rootScript != nullptr)
                    {
                        rootScript->SetField(node->GetName().c_str(), node);
                    }
                }
            }

            // Call update transform on the root node, mainly to update the cached euler rotation.
            Node3D* node3d = node->As<Node3D>();
            if (node3d != nullptr)
            {
                node3d->UpdateTransform(true);
            }

            nodeList.push_back(node);
        }

        rootNode = nodeList[0];
        OCT_ASSERT(rootNode);

        rootNode->SetScene(this);

        // Do we want this behavior for native nodes? I ran into a problem where...
        // I changed my scene for Buggy (in Navi) so that the root component was no longer a Sphere
        // and instead made the Buggy node inherit from Node3D. I added a Sphere3D collider node as a childe.
        // And when I loaded the Scene, it was removing the new sphere? I think the native nodes shouldn't be 
        // deletable (like in Unreal) but... I feel like I added this loop to destroy unused nodes for a reason...
        // Update: I think it might be if you rename a native node then you would get duplicates next time you open.
        // So maybe, we should disable renaming native nodes (force them to be named in C++)
#if 0
        // Destruct any native nodes that weren't matched by a SceneNodeDef
        for (uint32_t n = 0; n < nativeChildren.size(); ++n)
        {
            Node::Destruct(nativeChildren[n]);
            nativeChildren[n] = nullptr;
        }
#endif
    }

    // Destruct any replicated non-root nodes. The server will need to send the NetMsgSpawn for those.
    if (!NetIsAuthority())
    {
        std::vector<Node*> repNodesToDelete;

        auto pruneReplicated = [&](Node* node) -> bool
        {
            if (node != rootNode && node->IsReplicated())
            {
                repNodesToDelete.push_back(node);

                // We are going to destroy this node, so no need to traverse children
                return false;
            }

            return true;
        };
        rootNode->Traverse(pruneReplicated, true);

        for (uint32_t i = 0; i < repNodesToDelete.size(); ++i)
        {
            Node::Destruct(repNodesToDelete[i]);
            repNodesToDelete[i] = nullptr;
        }
    }

    return rootNode;
}

void Scene::ApplyRenderSettings(World* world)
{
    glm::vec4 ambientLight = DEFAULT_AMBIENT_LIGHT_COLOR;
    if (mSetAmbientLightColor)
    {
        ambientLight = mAmbientLightColor;
    }
    world->SetAmbientLightColor(ambientLight);

    glm::vec4 shadowColor = DEFAULT_SHADOW_COLOR;
    if (mSetShadowColor)
    {
        shadowColor = mShadowColor;
    }
    world->SetShadowColor(shadowColor);

    FogSettings fogSettings;
    if (mSetFog)
    {
        fogSettings.mEnabled = mFogEnabled;
        fogSettings.mColor = mFogColor;
        fogSettings.mDensityFunc = mFogDensityFunc;
        fogSettings.mNear = mFogNear;
        fogSettings.mFar = mFogFar;
    }
    world->SetFogSettings(fogSettings);
}

//const Property* Scene::GetProperty(const std::string& widgetName, const std::string& propName)
//{
//
//}
//
//NodeDef* Scene::FindWidgetDef(const std::string& name, int32_t* outIndex)
//{
//
//}

void Scene::AddNodeDef(Node* node, Platform platform, std::vector<Node*>& nodeList)
{
    OCT_ASSERT(node != nullptr);

    bool nodeIsRoot = (nodeList.size() == 0);

#if EDITOR
    // TODO-NODE: Do we want every node that is saved/loaded from a Scene to hold the mScene ref?
    //   Or just the root node? If only the root node should hold the scene ref (which I think is what we want)
    //   then keep this check below. When a child scene is expanded/unrolled then we remove the root's scene ref.
    if (node &&
        !nodeIsRoot && 
        node->GetScene() == this)
    {
        LogWarning("Recursive Scene chain found.");
        return;
    }
#endif

    if (!nodeIsRoot && node->IsTransient())
    {
        // Transient nodes mean they are not saved to the Scene.
        return;
    }

    if (node != nullptr)
    {
        nodeList.push_back(node);

        Node* parent = node->GetParent();

        mNodeDefs.push_back(SceneNodeDef());
        SceneNodeDef& nodeDef = mNodeDefs.back();

        nodeDef.mType = node->GetType();
        nodeDef.mParentIndex = FindNodeIndex(parent, nodeList);
        nodeDef.mParentBone = node->IsNode3D() ? ((int8_t) static_cast<Node3D*>(node)->GetParentBoneIndex()) : -1;

        Scene* scene = nullptr;
#if EDITOR
        nodeDef.mExposeVariable = node->ShouldExposeVariable();
#endif
        if (!nodeIsRoot)
        {
            scene = node->GetScene();
        }

        nodeDef.mScene = scene;
        nodeDef.mName = node->GetName();

        Stream extraDataStream;
        node->SaveStream(extraDataStream, platform);
        if (extraDataStream.GetSize() > 0)
        {
            nodeDef.mExtraData.resize(extraDataStream.GetSize());
            memcpy(nodeDef.mExtraData.data(), extraDataStream.GetData(), extraDataStream.GetSize());
        }

        GatherNonDefaultProperties(node, nodeDef.mProperties);

        if (scene == nullptr)
        {
            for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
            {
                AddNodeDef(node->GetChild(i), platform, nodeList);
            }
        }
    }
}

int32_t Scene::FindNodeIndex(Node* node, const std::vector<Node*>& nodeList)
{
    int32_t index = -1;

    for (uint32_t i = 0; i < nodeList.size(); ++i)
    {
        if (nodeList[i] == node)
        {
            index = (int32_t)i;
            break;
        }
    }

    return index;
}

