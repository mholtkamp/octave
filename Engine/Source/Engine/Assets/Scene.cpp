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
        def.mChildSlot = stream.ReadInt32();

        stream.ReadAsset(def.mScene);
        def.mExposeVariable = stream.ReadBool();

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
        stream.WriteInt32(def.mChildSlot);

        stream.WriteAsset(def.mScene);
        stream.WriteBool(def.mExposeVariable);

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

        for (uint32_t i = 0; i < mNodeDefs.size(); ++i)
        {
            Node* newNode = nullptr;

            if (mNodeDefs[i].mScene != nullptr)
            {
                Scene* scene = mNodeDefs[i].mScene.Get<Scene>();
                newNode = scene->Instantiate();

#if EDITOR
                newNode->SetExposeVariable(mNodeDefs[i].mExposeVariable);
#endif
            }
            else
            {
                newNode = Node::Construct(mNodeDefs[i].mType);
            }
            OCT_ASSERT(newNode);

            std::vector<Property> dstProps;
            newNode->GatherProperties(dstProps);
            CopyPropertyValues(dstProps, mNodeDefs[i].mProperties);

            if (i > 0)
            {
                Node* parent = nodeList[mNodeDefs[i].mParentIndex];

                parent->AddChild(newNode, mNodeDefs[i].mChildSlot);

                if (mNodeDefs[i].mExposeVariable)
                {
                    Script* rootScript = nodeList[0]->GetScript();
                    if (rootScript != nullptr)
                    {
                        rootScript->SetField(newNode->GetName().c_str(), newNode);
                    }
                }
            }

            // Start the script on the root node so we can expose child nodes are variables if set.
            if (i == 0)
            {
                newNode->StartScript();
            }

            nodeList.push_back(newNode);
        }

        rootNode = nodeList[0];
        OCT_ASSERT(rootNode);

        rootNode->SetScene(this);
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

        Scene* scene = nullptr;
#if EDITOR
        nodeDef.mExposeVariable = node->ShouldExposeVariable();
        scene = node->GetScene();
#endif
        nodeDef.mScene = scene;

        // Find child slot
        if (parent != nullptr)
        {
            for (uint32_t i = 0; i < parent->GetNumChildren(); ++i)
            {
                if (parent->GetChild(int32_t(i)) == node)
                {
                    nodeDef.mChildSlot = (int32_t)i;
                    break;
                }
            }

            OCT_ASSERT(nodeDef.mChildSlot != -1);
        }

        std::vector<Property> extProps;
        node->GatherProperties(extProps);

        {
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

