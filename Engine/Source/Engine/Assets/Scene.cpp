#include "Assets/Scene.h"
#include "World.h"
#include "Log.h"
#include "Engine.h"
#include "Nodes/Node.h"

FORCE_LINK_DEF(Scene);
DEFINE_ASSET(Scene);

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
}

glm::vec4 Scene::GetTypeColor()
{
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
                    existingChild->GetScene() == mNodeDefs[i].mScene)
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

            // Start the script on the root node so we can expose child nodes are variables if set.
            if (i == 0)
            {
                node->StartScript();
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
        }
    }

    return rootNode;
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

