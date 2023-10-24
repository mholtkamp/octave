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

    if (GetWorld() != nullptr &&
        GetWorld()->GetRootNode() != nullptr &&
        GetWorld()->GetRootNode()->GetScene() == scene)
    {
        GetWorld()->UpdateRenderSettings();
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

    stream.WriteUint32((uint32_t)mNodeDefs.size());

    for (uint32_t i = 0; i < mNodeDefs.size(); ++i)
    {
        SceneNodeDef& def = mNodeDefs[i];
        stream.WriteUint32((uint32_t)def.mType);
        stream.WriteInt32(def.mParentIndex);

        stream.WriteAsset(def.mScene);
        stream.WriteString(def.mName);
        stream.WriteBool(def.mExposeVariable);
        stream.WriteBool(def.mParentBone);

        stream.WriteUint32((uint32_t)def.mProperties.size());
        for (uint32_t p = 0; p < def.mProperties.size(); ++p)
        {
            def.mProperties[p].WriteStream(stream);
        }
    }

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
    //return glm::vec4(0.9f, 0.8f, 0.2f, 1.0f);
    return glm::vec4(0.4f, 0.4f, 1.0f, 1.0f);
}

const char* Scene::GetTypeName()
{
    return "Scene";
}

void Scene::Capture(Node* root)
{
    mNodeDefs.clear();

    if (root == nullptr)
        return;

    std::vector<Node*> nodeList;
    AddNodeDef(root, nodeList);
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
                        nativeChildren.push_back(node->GetChild(i));
                    }
                }
            }

            OCT_ASSERT(node);

            std::vector<Property> dstProps;
            node->GatherProperties(dstProps);
            CopyPropertyValues(dstProps, mNodeDefs[i].mProperties);

            // If this node has a script, then it might have script properties, and thosse
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

            nodeList.push_back(node);
        }

        rootNode = nodeList[0];
        OCT_ASSERT(rootNode);

        rootNode->SetScene(this);

        // Destruct any native nodes that weren't matched by a SceneNodeDef
        for (uint32_t n = 0; n < nativeChildren.size(); ++n)
        {
            Node::Destruct(nativeChildren[n]);
            nativeChildren[n] = nullptr;
        }
    }

    // Destruct any replicated non-root nodes. The server will need to send the NetMsgSpawn for those.
    if (!NetIsAuthority())
    {
        auto pruneReplicated = [&](Node* node) -> bool
        {
            if (node != rootNode && node->IsReplicated())
            {
                Node::Destruct(node);
                node = nullptr;
            }

            return true;
        };
        rootNode->Traverse(pruneReplicated, true);
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
    GetWorld()->SetAmbientLightColor(ambientLight);

    glm::vec4 shadowColor = DEFAULT_SHADOW_COLOR;
    if (mSetShadowColor)
    {
        shadowColor = mShadowColor;
    }
    GetWorld()->SetShadowColor(shadowColor);

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

//const Property* Scene::GetProperty(const std::string& widgetName, const std::string& propName)
//{
//
//}
//
//NodeDef* Scene::FindWidgetDef(const std::string& name, int32_t* outIndex)
//{
//
//}

void Scene::AddNodeDef(Node* node, std::vector<Node*>& nodeList)
{
    OCT_ASSERT(node != nullptr);

#if EDITOR
    // TODO-NODE: Do we want every node that is saved/loaded from a Scene to hold the mScene ref?
    //   Or just the root node? If only the root node should hold the scene ref (which I think is what we want)
    //   then keep this check below. When a child scene is expanded/unrolled then we remove the root's scene ref.
    if (node &&
        node->GetScene() == this)
    {
        LogWarning("Recursive Scene chain found.");
        return;
    }
#endif

    if (node->IsTransient())
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
        scene = node->GetScene();
#endif
        nodeDef.mScene = scene;

        std::vector<Property> extProps;
        node->GatherProperties(extProps);

        if (scene != nullptr)
        {
            // For nodes with scene sources, just copy all of the properties.
            for (uint32_t i = 0; i < extProps.size(); ++i)
            {
                nodeDef.mProperties.push_back(Property());
                nodeDef.mProperties.back().DeepCopy(extProps[i], true);
            }
        }
        else
        {
            // For native nodes, determine which properties are different than the defaults
            // and only save those to reduce storage/memory of the scene.
            Node* defaultNode = Node::Construct(node->GetType());
            std::vector<Property> defaultProps;
            defaultNode->GatherProperties(defaultProps);

            nodeDef.mProperties.reserve(extProps.size());
            for (uint32_t i = 0; i < extProps.size(); ++i)
            {
                Property* defaultProp = FindProperty(defaultProps, extProps[i].mName);

                if (defaultProp == nullptr ||
                    extProps[i].mType == DatumType::Asset ||
                    extProps[i] != *defaultProp)
                {
                    nodeDef.mProperties.push_back(Property());
                    nodeDef.mProperties.back().DeepCopy(extProps[i], true);
                }
            }

            delete defaultNode;
            defaultNode = nullptr;
        }

        // Recursively add children. Do not add children of widgets spawned via maps.
        if (scene == nullptr)
        {
            for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
            {
                AddNodeDef(node->GetChild(i), nodeList);
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

