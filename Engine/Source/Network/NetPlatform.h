#pragma once

#include "NetworkTypes.h"
#include "NetworkConstants.h"
#include "EngineTypes.h"
#include "NetSession.h"

enum class NetPlatformType
{
    LAN,
    Epic,
    // Steam,

    Count
};

class NetPlatform
{
public:

    // Lifecycle
    virtual void Create();
    virtual void Destroy();
    virtual void Update();

    // Login
    virtual void Login();
    virtual void Logout();
    virtual bool IsLoggedIn() const;

    // Matchmaking
    virtual void OpenSession();
    virtual void CloseSession();
    virtual void JoinSession(uint64_t sessionId);
    virtual void BeginSessionSearch();
    virtual void EndSessionSearch();
    virtual void UpdateSearch();
    virtual bool IsSearching() const;

    const std::vector<NetSession>& GetSessions() const;

protected:

    std::vector<NetSession> mSessions;
};
