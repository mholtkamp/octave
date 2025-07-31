#include "Assets/Scene.h"
#include "World.h"
#include "Log.h"
#include "Engine.h"
#include "Script.h"
#include "NetworkManager.h"
#include "NodePath.h"
#include "Nodes/Node.h"
#include "Nodes/3D/SkeletalMesh3d.h"

FORCE_LINK_DEF(Scene);
DEFINE_ASSET(Scene);

int32_t Scene::sInstantiationCount = 0;
std::vector<PendingNodePath> Scene::sPendingNodePaths;

#if OCT_SCENE_CONVERSION

#include "Nodes/3D/Audio3d.h"
#include "Nodes/3D/Box3d.h"
#include "Nodes/3D/Camera3d.h"
#include "Nodes/3D/Capsule3d.h"
#include "Nodes/3D/DirectionalLight3d.h"
#include "Nodes/3D/Particle3d.h"
#include "Nodes/3D/PointLight3d.h"
#include "Nodes/3D/ShadowMesh3d.h"
#include "Nodes/3D/SkeletalMesh3d.h"
#include "Nodes/3D/StaticMesh3d.h"
#include "Nodes/3D/Sphere3d.h"
#include "Nodes/3D/TextMesh3d.h"

#include "Nodes/Widgets/ArrayWidget.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Poly.h"
#include "Nodes/Widgets/PolyRect.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"

#include <unordered_set>

static bool sSceneConversionInit = false;
static std::unordered_map<TypeId, TypeId> sTypeConversionMap;

static int32_t sConvUniqueNum = 1;
static std::unordered_set<std::string> sConvNodeNames;

void SceneConversionInit()
{
    if (!sSceneConversionInit)
    {
        // Could replace hardcoded old typeids with OctHashString("OldName")...

        // Actors
        sTypeConversionMap.insert({ 69229970ul, Node3D::GetStaticType() }); // Actor
        sTypeConversionMap.insert({ 4284921748ul, Particle3D::GetStaticType() }); // ParticleActor
        sTypeConversionMap.insert({ 1065904008ul, StaticMesh3D::GetStaticType() }); // StaticMeshActor

        // Components
        sTypeConversionMap.insert({ 2798049751ul, Node::GetStaticType() });
        sTypeConversionMap.insert({ 4201802423ul, Node3D::GetStaticType() });
        sTypeConversionMap.insert({ 124085076ul, StaticMesh3D::GetStaticType() });
        sTypeConversionMap.insert({ 82174716ul, ShadowMesh3D::GetStaticType() });
        sTypeConversionMap.insert({ 3312400722ul, Audio3D::GetStaticType() });
        sTypeConversionMap.insert({ 111766969ul, Particle3D::GetStaticType() });
        sTypeConversionMap.insert({ 360869361ul, Sphere3D::GetStaticType() });
        sTypeConversionMap.insert({ 1549886543ul, SkeletalMesh3D::GetStaticType() });
        sTypeConversionMap.insert({ 1166207443ul, Camera3D::GetStaticType() });
        //sTypeConversionMap.insert({ 3012054486ul, Node::GetStaticType() }); // ScriptComponent
        sTypeConversionMap.insert({ 3929081410ul, DirectionalLight3D::GetStaticType() });
        sTypeConversionMap.insert({ 2255870423ul, Box3D::GetStaticType() });
        sTypeConversionMap.insert({ 1231172680ul, PointLight3D::GetStaticType() });
        sTypeConversionMap.insert({ 2918742384ul, Capsule3D::GetStaticType() });
        sTypeConversionMap.insert({ 1171320635ul, TextMesh3D::GetStaticType() });

        // Widgets
        sTypeConversionMap.insert({ 2158082633ul, Widget::GetStaticType() }); // ScriptWidget

        sSceneConversionInit = true;
    }
}

TypeId ConvertOldType(TypeId typeId)
{
    TypeId newType = typeId;

    auto it = sTypeConversionMap.find(typeId);
    if (it != sTypeConversionMap.end())
    {
        newType = it->second;
    }

    return newType;
}

void LoadStreamEx_Script(Stream& stream, Node* actorNode)
{
    OCT_ASSERT(actorNode != nullptr);
    OCT_ASSERT(actorNode->GetScript() == nullptr);

    //Component::LoadStream(stream);
    std::string name;
    stream.ReadString(name);
    bool active = stream.ReadBool();
    bool visible = stream.ReadBool();
    LogDebug("======= Ex Script =======");

    LogDebug("Comp: %s, %d, %d", name.c_str(), active, visible);

    std::string fileName;
    stream.ReadString(fileName);
    LogDebug("Filename: %s", fileName.c_str());


    // Load script properties
    uint32_t numProps = stream.ReadUint32();
    std::vector<Property> savedProps;
    savedProps.resize(numProps);

#if 0
    // Start the script before updating the saved props
    // (because the props won't exist until we start the script)
    StartScript();
#endif

    actorNode->SetScriptFile(fileName);
    std::vector<Property> scriptProps = actorNode->GetScript()->GetScriptProperties();

    for (uint32_t i = 0; i < numProps; ++i)
    {
        savedProps[i].ReadStream(stream, ASSET_VERSION_BASE, false, false);

        for (uint32_t j = 0; j < scriptProps.size(); ++j)
        {
            if (scriptProps[j].mName == savedProps[i].mName &&
                scriptProps[j].mType == savedProps[i].mType)
            {
                if (scriptProps[i].mCount < savedProps[i].mCount)
                {
                    scriptProps[i].SetCount(savedProps[i].mCount);
                }

                uint32_t count = savedProps[i].mCount;
                scriptProps[j].SetValue(savedProps[i].mData.vp, 0, count);
                break;
            }
        }
    }

    actorNode->GetScript()->SetScriptProperties(scriptProps);
}

void Scene::LoadStreamActor(Stream& stream)
{
    TypeId actorType = (TypeId)stream.ReadUint32();
    bool basicActor = (actorType == 69229970ul) || // Actor
        (actorType == 1065904008ul) || // StaticMeshActor
        (actorType == 4284921748ul); // ParticleActor
    actorType = ConvertOldType(actorType);

    if (basicActor)
    {
        actorType = Node3D::GetStaticType();
    }

    std::string actorName;
    stream.ReadString(actorName);
    bool replicate = stream.ReadBool();
    bool replicateTransform = stream.ReadBool();

    // Tags
    std::vector<std::string> tags;
    uint32_t numTags = (uint32_t)stream.ReadUint8();
    tags.resize(numTags);
    for (uint32_t i = 0; i < numTags; ++i)
    {
        stream.ReadString(tags[i]);
    }

    std::vector<NodePtr> compsToLoad;
    std::vector<int32_t> compParents;

    // Before we read in any of the component information,
    // construct a copy of the actor to determine how many default children (components) it has.
    NodePtr actorPtr = Node::Construct(actorType);

    for (uint32_t i = 0; i < actorPtr->GetNumChildren(); ++i)
    {
        if (!actorPtr->GetChild(i)->IsTransient())
        {
            compsToLoad.push_back(ResolvePtr(actorPtr->GetChild(i)));
        }
    }

    // Components
    uint32_t numComponents = stream.ReadUint32();

    for (uint32_t i = 0; i < numComponents; ++i)
    {
        TypeId type = TypeId(stream.ReadUint32());
        type = ConvertOldType(type);

        // Script component
        if (type == 3012054486ul)
        {
            LoadStreamEx_Script(stream, actorPtr.Get());
        }
        else
        {
            NodePtr comp = nullptr;
            if (i < compsToLoad.size())
            {
                comp = compsToLoad[i];
                OCT_ASSERT(type == comp->GetType());
            }
            else
            {
                comp = Node::Construct(type);
                OCT_ASSERT(comp);
                compsToLoad.push_back(comp);
            }

            comp->LoadStreamEx(stream);

            compParents.push_back(-1);
        }
    }

    // Hierarchy
    for (int32_t i = 0; i < int32_t(compsToLoad.size()); ++i)
    {
        if (!compsToLoad[i]->IsNode3D())
        {
            // I don't think we should hit this. Assuming we
            // don't construct nodes for ScriptComponents, I don't think
            // there are any other non-transform components?
            OCT_ASSERT(false);
            continue;
        }

        int32_t compIndex = stream.ReadInt32();
        int32_t parentIndex = stream.ReadInt32();

        OCT_ASSERT(compsToLoad[compIndex]->IsNode3D());
        OCT_ASSERT(parentIndex == -1 || compsToLoad[parentIndex]->IsNode3D());

        Node3D* transComp = static_cast<Node3D*>(compsToLoad[compIndex].Get());

        if (parentIndex == -1)
        {
            //SetRootComponent(transComp);
        }
        else
        {
            //Node3D* parentComp = static_cast<Node3D*>(compsToLoad[parentIndex]);
            //transComp->Attach(parentComp);
            compParents[compIndex] = parentIndex;
        }
    }

    for (uint32_t i = 0; i < compsToLoad.size(); ++i)
    {
        Node* node = compsToLoad[i].Get();
        if (Cast<Node3D>(node))
        {
            Node3D* node3D = Cast<Node3D>(node);
            node3D->UpdateTransform(false);
        }
    }

    // Add NodeDefs
    if (basicActor && compsToLoad.size() == 1)
    {
        // Just add the component as a single node.
        Node* node = compsToLoad[0].Get();
        mNodeDefs.push_back(SceneNodeDef());
        SceneNodeDef& def = mNodeDefs.back();
        def.mType = node->GetType();
        def.mParentIndex = 0; // This should be index of the Node3D root we made for the level.
        
        const bool kUseCompName = true;
        def.mName = kUseCompName ? node->GetName() : actorName;

        std::vector<Property> extProps;
        node->GatherProperties(extProps);
        for (uint32_t p = 0; p < extProps.size(); ++p)
        {
            def.mProperties.push_back(Property());
            Property& prop = def.mProperties.back();
            prop.DeepCopy(extProps[p], true);

            if (prop.mName == "Name")
            {
                if (def.mType == StaticMesh3D::GetStaticType())
                {
                    StaticMesh* mesh = node->As<StaticMesh3D>()->GetStaticMesh();
                    if (mesh != nullptr)
                    {
                        prop.SetString(mesh->GetName());
                    }
                }
            }
        }
    }
    else
    {
        // Otherwise, we are going to save a node for the Actor and then save all of the components as children.
        int32_t actorNodeIdx = (int32_t)mNodeDefs.size();
        mNodeDefs.push_back(SceneNodeDef());
        SceneNodeDef& actorDef = mNodeDefs.back();
        actorDef.mType = actorType;
        actorDef.mParentIndex = 0;
        actorDef.mName = actorName;

        {
            NodePtr actorNode = Node::Construct(actorType);
            std::vector<Property> actorProps;
            actorNode->GatherProperties(actorProps);

            for (uint32_t p = 0; p < actorProps.size(); ++p)
            {
                actorDef.mProperties.push_back(Property());
                actorDef.mProperties.back().DeepCopy(actorProps[p], true);
            }
        }

        for (uint32_t i = 0; i < compsToLoad.size(); ++i)
        {
            mNodeDefs.push_back(SceneNodeDef());
            SceneNodeDef& compDef = mNodeDefs.back();

            Node* node = compsToLoad[i].Get();

            int32_t parentNodeDefIdx = compParents[i];
            if (parentNodeDefIdx == -1)
            {
                // This was the root, so now it's attached to the actor node.
                parentNodeDefIdx = actorNodeIdx;
            }
            else
            {
                // Need to offset it.
                parentNodeDefIdx += (actorNodeIdx + 1);
            }

            compDef.mType = node->GetType();
            compDef.mParentIndex = parentNodeDefIdx; // This should be index of the Node3D root we made for the level.
            compDef.mName = node->GetName();

            std::vector<Property> extProps;
            node->GatherProperties(extProps);
            for (uint32_t p = 0; p < extProps.size(); ++p)
            {
                compDef.mProperties.push_back(Property());
                compDef.mProperties.back().DeepCopy(extProps[p], true);
            }
        }
    }

    // Destroy temporary Nodes that were constructed
    compsToLoad.clear();

    // Hacky hand-written loading for anything that overrode Actor::LoadStream()
    // MAKE SURE TO DELETE THE CALL TO Actor::LoadStream() in these classes (since that is done by hand above)
    if (actorType == 1500601734) // Rotator
    {
        NodePtr hackNode = Node::Construct((TypeId)1500601734);
        hackNode->LoadStream(stream, Platform::Count, ASSET_VERSION_BASE);
    }
    else if (!basicActor)
    {
        actorPtr->LoadStreamEx(stream);
    }
}

void Scene::LoadStreamLevel(Stream& stream)
{
    bool netLoad = stream.ReadBool();

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

    uint32_t dataSize = stream.ReadUint32();

    // ---------------
    // Level stream
    // ---------------

    uint32_t numActors = 0;
    numActors = stream.ReadUint32();
    LogDebug("Loading Level... %d Actors", numActors);

    // Make a Node3D for root
    mNodeDefs.push_back(SceneNodeDef());
    SceneNodeDef& rootDef = mNodeDefs.back();
    rootDef.mType = Node3D::GetStaticType();
    rootDef.mName = mName;

    for (uint32_t i = 0; i < numActors; ++i)
    {
        bool bp = stream.ReadBool();

        if (bp)
        {
            AssetRef bpRef;
            stream.ReadAsset(bpRef);

            mNodeDefs.push_back(SceneNodeDef());
            SceneNodeDef& def = mNodeDefs.back();

            uint32_t numOverrides = stream.ReadUint32();

            for (uint32_t o = 0; o < numOverrides; ++o)
            {
                int32_t overIndex = stream.ReadInt32();
                Property overProp;
                overProp.ReadStream(stream, ASSET_VERSION_BASE, false, false);

                // Only apply overrides on root of BP. Will need to handle all children if required for Yami.
                // What are the overrides that would be on the root node and not actor? Just pos/rot/scale really?
                if (overIndex == 0 || overIndex == -1)
                {
                    if (overIndex == 0 &&
                        overProp.mName != "Position" &&
                        overProp.mName != "Rotation" &&
                        overProp.mName != "Scale")
                    {
                        LogError("zzz Non-root override: %s", overProp.mName.c_str());
                    }

                    def.mProperties.push_back(overProp);
                }

                def.mScene = bpRef;
                def.mParentIndex = 0;
            }
        }
        else
        {
            LoadStreamActor(stream);
        }

        // All actors are parented to new root level node
        mNodeDefs.back().mParentIndex = 0;
    }

    ConvEnforceUniqueNames();
}

void Scene::LoadStreamBlueprint(Stream& stream)
{
    TypeId actorType = (TypeId)stream.ReadUint32();
    TypeId rootType = ConvertOldType(actorType);

    mNodeDefs.push_back(SceneNodeDef());
    SceneNodeDef& rootDef = mNodeDefs.back();
    rootDef.mType = rootType;
    rootDef.mProperties.resize(stream.ReadUint32());

    for (uint32_t i = 0; i < rootDef.mProperties.size(); ++i)
    {
        rootDef.mProperties[i].ReadStream(stream, ASSET_VERSION_BASE, false, false);

        if (rootDef.mProperties[i].mName == "Name")
        {
            rootDef.mName = rootDef.mProperties[i].GetString();
        }
    }

    uint32_t numComps = stream.ReadUint32();
    mNodeDefs.resize(numComps + 1); // components + 1 (root node / old actor)

    std::vector<std::string> parentNames;
    parentNames.resize(numComps + 1);

    int32_t scriptIdx = -1;

    for (uint32_t c = 1; c < numComps + 1; ++c)
    {
        stream.ReadString(mNodeDefs[c].mName);

        stream.ReadString(parentNames[c]);
        mNodeDefs[c].mType = ConvertOldType((TypeId)stream.ReadUint32());
        mNodeDefs[c].mParentBone = stream.ReadInt8();
        bool isDefault = stream.ReadBool();

        mNodeDefs[c].mProperties.resize(stream.ReadUint32());

        for (uint32_t p = 0; p < mNodeDefs[c].mProperties.size(); ++p)
        {
            mNodeDefs[c].mProperties[p].ReadStream(stream, ASSET_VERSION_BASE, false, false);

            if (mNodeDefs[c].mProperties[p].mName == "Name")
            {
                mNodeDefs[c].mName = mNodeDefs[c].mProperties[p].GetString();
            }
        }

        if (mNodeDefs[c].mType == 3012054486ul)
        {
            // Only expecting one script
            OCT_ASSERT(scriptIdx == -1);
            scriptIdx = int32_t(c);
        }
    }

    // Script component
    if (scriptIdx != -1)
    {
        // Move the properties to the root node.
        SceneNodeDef& scriptDef = mNodeDefs[scriptIdx];
        std::vector<Property>& props = scriptDef.mProperties;
        for (uint32_t p = 0; p < scriptDef.mProperties.size(); ++p)
        {
            // Ignore base Component properties
            if (props[p].mName != "Name" &&
                props[p].mName != "Active" &&
                props[p].mName != "Visible")
            {
                // Script component's "Filename" was renamed to "Scipt" in Node.
                if (props[p].mName == "Filename")
                {
                    props[p].mName = "Script";
                }

                mNodeDefs[0].mProperties.push_back(props[p]);
            }
        }

        // Delete the nodedef for the script comp
        --numComps;
        mNodeDefs.erase(mNodeDefs.begin() + scriptIdx);
        parentNames.erase(parentNames.begin() + scriptIdx);
    }

    std::string rootName;
    stream.ReadString(rootName);
    LogDebug("RootName = %s", rootName.c_str());

    // Find parent index for all components
    for (uint32_t c = 1; c < numComps + 1; ++c)
    {
        if (parentNames[c] != "")
        {
            int32_t parentIndex = -1;
            for (uint32_t x = 1; x < numComps + 1; ++x)
            {
                if (mNodeDefs[x].mName == parentNames[c])
                {
                    parentIndex = x;
                    mNodeDefs[c].mParentIndex = parentIndex;
                    break;
                }
            }

            // We didn't find a parent for this node??
            OCT_ASSERT(parentIndex != -1);
        }
        else
        {
            mNodeDefs[c].mParentIndex = 0;
        }

        // For the prev root node, transfer the pos/rot/scale to the new root
        if (mNodeDefs[c].mName == rootName)
        {
            for (int32_t p = int32_t(mNodeDefs[c].mProperties.size()) - 1; p >= 0; --p)
            {
                std::vector<Property>& oldProps = mNodeDefs[c].mProperties;
                std::vector<Property>& newProps = mNodeDefs[0].mProperties;

                if (oldProps[p].mName == "Position" ||
                    oldProps[p].mName == "Rotation" ||
                    oldProps[p].mName == "Scale")
                {
                    // Find existing prop?
                    Property* existingProp = nullptr;
                    for (auto& prop : newProps)
                    {
                        if (prop.mName == oldProps[p].mName)
                        {
                            existingProp = &prop;
                            break;
                        }
                    }

                    // Replace / add prop
                    if (existingProp)
                    {
                        *existingProp = oldProps[p];
                    }
                    else
                    {
                        newProps.push_back(oldProps[p]);
                    }

                    // Remove prop from old root
                    oldProps.erase(oldProps.begin() + p);
                }
            }
        }
    }

    ConvEnforceUniqueNames();
}

void Scene::LoadStreamWidgetMap(Stream& stream)
{
    uint32_t numWidgetDefs = stream.ReadUint32();
    OCT_ASSERT(numWidgetDefs < 1000); // Something reasonable?
    mNodeDefs.resize(numWidgetDefs);

    for (uint32_t i = 0; i < numWidgetDefs; ++i)
    {
        SceneNodeDef& def = mNodeDefs[i];

        def.mType = (TypeId)stream.ReadUint32();
        def.mParentIndex = stream.ReadInt32();
        int32_t childSlot = stream.ReadInt32();
        int32_t nativeChildSlot = stream.ReadInt32();

        stream.ReadAsset(def.mScene);
        def.mExposeVariable = stream.ReadBool();

        uint32_t numProps = stream.ReadUint32();
        def.mProperties.resize(numProps);
        for (uint32_t p = 0; p < numProps; ++p)
        {
            def.mProperties[p].ReadStream(stream, ASSET_VERSION_BASE, false, false);

            // Name needed for Scene
            if (def.mProperties[p].mName == "Name")
            {
                def.mName = def.mProperties[p].GetString();
            }
        }
    }

    ConvEnforceUniqueNames();
}

void Scene::ConvEnforceUniqueNames()
{
    sConvNodeNames.clear();
    sConvUniqueNum = 1;

    for (uint32_t i = 0; i < mNodeDefs.size(); ++i)
    {
        auto it = sConvNodeNames.find(mNodeDefs[i].mName);

        if (it != sConvNodeNames.end())
        {
            mNodeDefs[i].mName = mNodeDefs[i].mName + "#" + std::to_string(sConvUniqueNum);
            sConvUniqueNum++;

            it = sConvNodeNames.find(mNodeDefs[i].mName);
            OCT_ASSERT(it == sConvNodeNames.end()); // Should be unique now!!
        }

        sConvNodeNames.insert(mNodeDefs[i].mName);
    }
}

#endif

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

#if OCT_SCENE_CONVERSION
    SceneConversionInit();

    if (mOldType == 83055820)
    {
        LoadStreamLevel(stream);
        return;
    }
    else if (mOldType == 2369317065)
    {
        LoadStreamBlueprint(stream);
        return;
    }
    else if (mOldType == 3362483201)
    {
        LoadStreamWidgetMap(stream);
        return;
    }
#endif

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
            def.mProperties[p].ReadStream(stream, mVersion, false, false);
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
        NodePtr tempRoot = Instantiate();

        if (tempRoot)
        {
            srcScene->Capture(tempRoot.Get(), platform);
            tempRoot->Destroy();
        }
        else
        {
            srcScene = this;
        }

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
            def.mProperties[p].WriteStream(stream, false);
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
    outProps.push_back(Property(DatumType::Byte, "Fog Density", this, &mFogDensityFunc, 1, HandlePropChange, NULL_DATUM, int32_t(FogDensityFunc::Count), sFogDensityStrings));
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

NodePtr Scene::Instantiate()
{
    NodePtr rootNode = nullptr;

    sInstantiationCount++;

    if (mNodeDefs.size() > 0)
    {
        std::vector<NodePtr> nodeList;

        // The nativeChildren vector holds a list of all children by created in C++ for the nodes in this scene.
        // If there is no SceneNodeDef for the nativeChild, then we must destroy it. This will happen
        // if the user renames a native child, then we will have a duplicate so we need to destroy the native one.
        std::vector<Node*> nativeChildren;

        for (uint32_t i = 0; i < mNodeDefs.size(); ++i)
        {
            NodePtr nodePtr;
            NodePtr parent = (i > 0) ? nodeList[mNodeDefs[i].mParentIndex] : nullptr;

            if (parent != nullptr)
            {
                // See if the node already exists. This can happen if lets say,
                // the root node spawned other nodes on Create() in C++.
                Node* existingChild = parent->FindChild(mNodeDefs[i].mName, false);

                if (existingChild != nullptr &&
                    existingChild->GetType() == mNodeDefs[i].mType &&
                    existingChild->GetScene() == mNodeDefs[i].mScene.Get())
                {
                    nodePtr = ResolvePtr(existingChild);
                }

                if (nodePtr != nullptr)
                {
                    // Double check that we found a natively spawned child.
                    // Otherwise do we have conflicting SceneNodeDefs with same name?!
                    bool isNativeChild = false;
                    for (uint32_t n = 0; n < nativeChildren.size(); ++n)
                    {
                        if (nodePtr == nativeChildren[n])
                        {
                            nativeChildren.erase(nativeChildren.begin() + n);
                            isNativeChild = true;

                            // Add this node's children as new native child nodes.
                            for (uint32_t c = 0; c < nodePtr->GetNumChildren(); ++c)
                            {
                                nativeChildren.push_back(nodePtr->GetChild(c));
                            }

                            break;
                        }
                    }

                    OCT_ASSERT(isNativeChild);
                }
            }

            // We aren't overriding a native child, so we need to create a new node.
            if (nodePtr == nullptr)
            {
                if (mNodeDefs[i].mScene != nullptr)
                {
                    Scene* scene = mNodeDefs[i].mScene.Get<Scene>();
                    nodePtr = scene->Instantiate();

#if EDITOR
                    nodePtr->SetExposeVariable(mNodeDefs[i].mExposeVariable);
#endif
                }
                else
                {
                    nodePtr = Node::Construct(mNodeDefs[i].mType);

                    for (uint32_t c = 0; c < nodePtr->GetNumChildren(); ++c)
                    {
                        nativeChildren.push_back(nodePtr->GetChild(c));
                    }
                }
            }

            OCT_ASSERT(nodePtr);
            Node* node = nodePtr.Get();

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

            // See if there are any nodepaths that need to be resolved.
            for (auto& prop : mNodeDefs[i].mProperties)
            {
                if (prop.mType == DatumType::Node &&
                    prop.mExtra != nullptr &&
                    prop.mCount > 0)
                {
                    PendingNodePath path;
                    path.mNode = ResolvePtr<Node>(node);
                    path.mPropName = prop.mName;
                    path.mPath = *prop.mExtra;
                    sPendingNodePaths.push_back(path);
                }
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

            nodeList.push_back(nodePtr);
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
            if (rootNode != node && node->IsReplicated())
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

    sInstantiationCount--;

    // Finished loading a top-level scene, try to resolve all node paths
    if (sInstantiationCount == 0)
    {
        ResolvePendingNodePaths(sPendingNodePaths);
        sPendingNodePaths.clear();
    }

    // Negative instantiation count should never happen!
    OCT_ASSERT(sInstantiationCount >= 0);
    if (sInstantiationCount < 0)
    {
        sInstantiationCount = 0;
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
        RecordNodePaths(node, nodeDef.mProperties);

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

bool Scene::CheckForNodeProps(std::vector<Property>& props)
{
    bool hasNodeProp = false;
    for (uint32_t i = 0; i < props.size(); ++i)
    {
        if (props[i].GetType() == DatumType::Node)
        {
            hasNodeProp = true;
            break;
        }
    }

    return hasNodeProp;
}
