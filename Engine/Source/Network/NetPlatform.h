#pragma once

#include "NetworkTypes.h"
#include "NetworkConstants.h"
#include "EngineTypes.h"
#include "NetSession.h"

#ifdef SendMessage
#undef SendMessage
#endif

struct NetMsg;

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
    virtual void JoinSession(const NetSession& session);
    virtual void BeginSessionSearch();
    virtual void EndSessionSearch();
    virtual void UpdateSearch();
    virtual bool IsSearching() const;

    // Messaging
    virtual void SendMessage(const NetHost& host, const char* buffer, uint32_t size);
    virtual int32_t RecvMessage(char* recvBuffer, int32_t bufferSize, NetHost& outHost);

    const std::vector<NetSession>& GetSessions() const;

protected:

    std::vector<NetSession> mSessions;
};
