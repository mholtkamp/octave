#pragma once

#include "Constants.h"
#include "Network/NetworkConstants.h"

struct NetSession
{
    NetHost mHost = {};
    char mName[OCT_SESSION_NAME_LEN + 1] = {};
    uint8_t mMaxPlayers = 0;
    uint8_t mNumPlayers = 0;

    uint64_t mLobbyId = 0;
    bool mLan = true;
};

struct NetSessionOpenOptions
{
    std::string mName = "Session";
    int32_t mMaxPlayers = 16;
    uint16_t mPort = OCT_DEFAULT_PORT;
    bool mLan = false;
    bool mPrivate = false;
};

struct NetSessionSearchOptions
{
    std::string mName;
};
