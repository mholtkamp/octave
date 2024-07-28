#include "NetPlatform.h"

// Lifecycle
bool NetPlatform::Create()
{
    return false;
}

void NetPlatform::Destroy()
{

}

void NetPlatform::Update()
{

}

// Login
void NetPlatform::Login()
{

}

void NetPlatform::Logout()
{

}

bool NetPlatform::IsLoggedIn() const
{
    return false;
}

// Matchmaking
void NetPlatform::OpenSession()
{

}

void NetPlatform::CloseSession()
{

}

void NetPlatform::JoinSession(const NetSession& session)
{

}

void NetPlatform::BeginSessionSearch()
{

}

void NetPlatform::EndSessionSearch()
{

}

void NetPlatform::UpdateSearch()
{

}

bool NetPlatform::IsSearching() const
{
    return false;
}

void NetPlatform::SendMessage(const NetHost& host, const char* buffer, uint32_t size)
{

}

int32_t NetPlatform::RecvMessage(char* recvBuffer, int32_t bufferSize, NetHost& outHost)
{
    return 0;
}

const std::vector<NetSession>& NetPlatform::GetSessions() const
{
    return mSessions;
}

