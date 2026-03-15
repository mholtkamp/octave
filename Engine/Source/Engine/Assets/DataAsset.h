#pragma once

#include "Asset.h"
#include "Property.h"
#include <vector>
#include <string>

class OCTAVE_API DataAsset : public Asset
{
public:

    DECLARE_ASSET(DataAsset, Asset);

    DataAsset();
    virtual ~DataAsset();

    virtual void Create() override;
    virtual void Destroy() override;

    // Asset overrides
    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    // Lua script file that defines properties
    void SetScriptFile(const std::string& path);
    const std::string& GetScriptFile() const { return mScriptFile; }

    // Get property value by name (for Lua access)
    Datum* GetPropertyValue(const std::string& name);
    bool SetPropertyValue(const std::string& name, const Datum& value);

    // Get all dynamic properties
    const std::vector<Property>& GetDataProperties() const { return mProperties; }
    std::vector<Property>& GetDataProperties() { return mProperties; }

    // Reload properties from Lua script
    void ReloadPropertiesFromScript();

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    std::string mScriptFile;                    // Path to .lua defining properties
    std::vector<Property> mProperties;          // Dynamic properties from Lua
    bool mPropertiesLoaded = false;

    void LoadPropertiesFromLua();
    void SyncPropertyStorage();
};
