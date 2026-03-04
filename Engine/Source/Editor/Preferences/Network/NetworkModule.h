#pragma once

#if EDITOR

// Undefine Windows macros that collide with our method names
#ifdef SetPort
#undef SetPort
#endif

#include "../PreferencesModule.h"

class NetworkModule : public PreferencesModule
{
public:
    DECLARE_PREFERENCES_MODULE(NetworkModule)

    NetworkModule();
    virtual ~NetworkModule();

    virtual const char* GetName() const override { return GetStaticName(); }
    virtual const char* GetParentPath() const override { return GetStaticParentPath(); }
    virtual void Render() override;
    virtual void LoadSettings(const rapidjson::Document& doc) override;
    virtual void SaveSettings(rapidjson::Document& doc) override;

    bool GetControllerServerEnabled() const { return mControllerServerEnabled; }
    int GetPort() const { return mPort; }
    bool GetLogRequests() const { return mLogRequests; }

    void SetControllerServerEnabled(bool enabled);
    void SetPort(int port);
    void SetLogRequests(bool log);

    static NetworkModule* Get();

private:
    bool mControllerServerEnabled = false;
    int mPort = 7890;
    bool mLogRequests = false;

    static NetworkModule* sInstance;
};

#endif
