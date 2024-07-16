#include "NetPlatform.h"

// Lifecycle
void NetPlatform::Create()
{

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


const std::vector<NetSession>& NetPlatform::GetSessions() const
{
    return mSessions;
}

