#pragma once

#include <string>
#include <vector>
#include "Assertion.h"

#include "RTTI.h"
#include "EngineTypes.h"
#include "Property.h"
#include "Stream.h"
#include "Factory.h"

class Actor;
class World;

#define DECLARE_NODE(Base, Parent) DECLARE_FACTORY(Base, Node); DECLARE_RTTI(Base, Parent);
#define DEFINE_NODE(Base) DEFINE_FACTORY(Base, Node); DEFINE_RTTI(Base);

class Node : public RTTI
{
public:

    DECLARE_FACTORY_MANAGER(Node);
    DECLARE_FACTORY(Node, Node);
    DECLARE_RTTI(Node, RTTI);

    Node();
    virtual ~Node();

    virtual void Create();
    virtual void Destroy();
    
    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream);

    virtual void Copy(Node* srcNode);

    virtual void Start();
    virtual void Stop();
    virtual void Tick(float deltaTime);
    virtual void EditorTick(float deltaTime);
    virtual void GatherProperties(std::vector<Property>& outProps);

    virtual void SetOwner(Actor* owner);
    Actor* GetOwner();
    void SetName(const std::string& newName);
    const std::string& GetName() const;
    void SetActive(bool active);
    bool IsActive() const;
    void SetVisible(bool visible);
    bool IsVisible() const;
    void SetTransient(bool transient);
    virtual bool IsTransient() const;

    void SetDefault(bool isDefault);
    bool IsDefault() const;

    World* GetWorld();

    virtual const char* GetTypeName() const;
    virtual DrawData GetDrawData();

    virtual bool IsTransformNode() const;
    virtual bool IsPrimitiveNode() const;
    virtual bool IsLightNode() const;

protected:

    Node* mParent = nullptr;
    std::vector<Node*> mChildren;

    std::string mName;
    bool mActive = true;
    bool mVisible = true;
    bool mTransient = false;
    bool mDefault = false;
};