#pragma once

#include "Asset.h"
#include "Property.h"
#include "Factory.h"

class World;
class Node;

struct SceneNodeDef
{
    TypeId mType = INVALID_TYPE_ID;
    int32_t mParentIndex = -1;
    int32_t mChildSlot = -1;
    SceneRef mScene;
    std::vector<Property> mProperties;
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

    void Capture(Node* root);
    Node* Instantiate();

protected:

    void AddNodeDef(Node* node, std::vector<Node*>& nodeList);
    int32_t FindNodeIndex(Node* node, const std::vector<Node*>& nodeList);

    std::vector<SceneNodeDef> mNodeDefs;
};
