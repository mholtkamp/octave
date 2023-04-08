#pragma once

#include "ScriptFunc.h"

#include "NetMsg.h"

struct AnimEvent;
class Widget;

struct ScriptEvent
{
    // SkeletalMeshComponent
    static void Animation(std::string& tableName, std::string& funcName, const AnimEvent& animEvent);

    // Widget (Button, Selector, TextField events)
    static void WidgetState(std::string& tableName, std::string& funcName, Widget* widget);

    // NetworkManager
    static void NetConnect(std::string& tableName, std::string& funcName, const NetClient& client);
    static void NetAccept(std::string& tableName, std::string& funcName);
    static void NetReject(std::string& tableName, std::string& funcName, NetMsgReject::Reason reason);
    static void NetDisconnect(std::string& tableName, std::string& funcName, const NetClient& client);
    static void NetKick(std::string& tableName, std::string& funcName, NetMsgKick::Reason reason);
};
