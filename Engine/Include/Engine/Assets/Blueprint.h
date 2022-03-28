#pragma once

#include "Asset.h"
#include "Property.h"
#include "Factory.h"

class World;
class Actor;

struct BlueprintComp
{
    TypeId mType = INVALID_TYPE_ID;
    int32_t mParent = -1;
    std::vector<Property> mProperties;
};

class Blueprint : public Asset
{
public:

    DECLARE_ASSET(Blueprint, Asset);

    Blueprint();
    ~Blueprint();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    void Create(Actor* srcActor);
    Actor* Instantiate(World* world);

    const Property* GetActorProperty(const char* name);
    const Property* GetComponentProperty(int32_t index, const char* name);

protected:

    TypeId mActorType = INVALID_TYPE_ID;
    std::vector<Property> mActorProps;
    std::vector<BlueprintComp> mComponents;
    int32_t mRootComponentIndex = -1;
};
