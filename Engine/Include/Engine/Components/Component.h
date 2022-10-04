#pragma once

#include <string>
#include <vector>
#include <assert.h>

#include "RTTI.h"
#include "EngineTypes.h"
#include "Property.h"
#include "Stream.h"
#include "Factory.h"

class Actor;
class World;

#define DECLARE_COMPONENT(Base, Parent) DECLARE_FACTORY(Base, Component); DECLARE_RTTI(Base, Parent);
#define DEFINE_COMPONENT(Base) DEFINE_FACTORY(Base, Component); DEFINE_RTTI(Base);

class Component : public RTTI
{
public:

    DECLARE_FACTORY_MANAGER(Component);
    DECLARE_FACTORY(Component, Component);
    DECLARE_RTTI(Component, RTTI);

    Component();
    virtual ~Component();

    virtual void Create();
    virtual void Destroy();
    
    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream);

    virtual void Copy(Component* srcComp);

    virtual void BeginPlay();
    virtual void EndPlay();
    virtual void Tick(float deltaTime);
    virtual void EditorTick(float deltaTime);
    virtual void GatherProperties(std::vector<Property>& outProps);

    virtual void SetOwner(Actor* owner);
    Actor* GetOwner();
    void SetName(const std::string& newName);
    const std::string& GetName() const;
    uint32_t GetId() const;
    void SetActive(bool active);
    bool IsActive() const;
    void SetVisible(bool visible);
    bool IsVisible() const;
    void SetTransient(bool transient);
    virtual bool IsTransient() const;

    World* GetWorld();

    virtual const char* GetTypeName() const;
    virtual DrawData GetDrawData();

    virtual bool IsTransformComponent() const;
    virtual bool IsPrimitiveComponent() const;
    virtual bool IsLightComponent() const;

protected:

    Actor* mOwner;

    std::string mName;
    uint32_t mId;
    bool mActive;
    bool mVisible;
    bool mTransient;
};