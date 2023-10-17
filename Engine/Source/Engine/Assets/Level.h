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
    virtual void Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    void CaptureWorld(World* world);
    void LoadIntoWorld(
        World* world,
        bool clear = false,
        glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f));
    void UnloadFromWorld(World* world);

    bool GetNetLoad() const;

    void ApplySettings(bool force);
    void RemoveSettings(bool force);

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    bool ShouldSaveActor(Actor* actor) const;

    bool mNetLoad = true;

    bool mSetAmbientLightColor = false;
    bool mSetShadowColor = false;
    bool mSetFog = false;

    glm::vec4 mAmbientLightColor = { 0.1f, 0.1f, 0.1f, 1.0f };
    glm::vec4 mShadowColor = {0.0f, 0.0f, 0.0f, 0.8f};

    bool mFogEnabled = false;
    glm::vec4 mFogColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    FogDensityFunc mFogDensityFunc = FogDensityFunc::Linear;
    float mFogNear = 0.0f;
    float mFogFar = 100.0f;

    std::vector<uint8_t> mData;
};