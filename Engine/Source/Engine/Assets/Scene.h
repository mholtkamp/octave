#pragma once

#include "Asset.h"
#include "Property.h"
#include "Factory.h"
#include "AssetRef.h"

class World;
class Node;

struct SceneNodeDef
{
    TypeId mType = INVALID_TYPE_ID;
    int32_t mParentIndex = -1;
    SceneRef mScene;
    std::string mName;
    std::vector<Property> mProperties;
    std::vector<uint8_t> mExtraData;
    int8_t mParentBone = -1;
    bool mExposeVariable = false;
};

class Scene : public Asset
{
public:

    DECLARE_ASSET(Scene, Asset);

    Scene();
    ~Scene();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    void Capture(Node* root, Platform platform = Platform::Count);
    Node* Instantiate();

    void ApplyRenderSettings(World* world);

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void AddNodeDef(Node* node, Platform platform, std::vector<Node*>& nodeList);
    int32_t FindNodeIndex(Node* node, const std::vector<Node*>& nodeList);

    std::vector<SceneNodeDef> mNodeDefs;

    // World render properties (used when this scene is the world root).
    bool mSetAmbientLightColor = false;
    bool mSetShadowColor = false;
    bool mSetFog = false;

    glm::vec4 mAmbientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    glm::vec4 mShadowColor = { 0.0f, 0.0f, 0.0f, 0.8f };

    bool mFogEnabled = false;
    glm::vec4 mFogColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    FogDensityFunc mFogDensityFunc = FogDensityFunc::Linear;
    float mFogNear = 0.0f;
    float mFogFar = 100.0f;
};
