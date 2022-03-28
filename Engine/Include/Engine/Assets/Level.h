#pragma once

#include "Asset.h"

class World;
class Actor;

class Level : public Asset
{
public:

    DECLARE_ASSET(Level, Asset);

    Level();
    ~Level();

    virtual void LoadStream(Stream& stream, Platform platform) override;
	virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Import(const std::string& path) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    void CaptureWorld(World* world);
    void LoadIntoWorld(World* world);
    void UnloadFromWorld(World* world);

    bool GetNetLoad() const;

protected:

    bool ShouldSaveActor(Actor* actor) const;

    bool mNetLoad = true;

    std::vector<uint8_t> mData;
};