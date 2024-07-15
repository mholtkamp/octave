#pragma once

struct NetSession
{
    NetHost mHost = {};
    char mName[OCT_SESSION_NAME_LEN + 1] = {};
    uint8_t mMaxPlayers = 0;
    uint8_t mNumPlayers = 0;
};

struct NetSessionCreateOptions
{
    std::string mName;
    int32_t mMaxPlayers = 4;
    uint16_t mPort = 5151;
    bool mJoinInProgress = true;
};

struct NetSessionSearchOptions
{
    std::string mName;
};
