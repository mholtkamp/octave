#pragma once

#include "SmartPointer.h"
#include "ScriptFunc.h"
#include "Datum.h"

#include <unordered_map>

class Node;

typedef void (*SignalHandlerFP)(Node*, const std::vector<Datum>&);

struct SignalHandlerFunc
{
    SignalHandlerFP mFuncPointer = nullptr;
    mutable ScriptFunc mScriptFunc;
};

class Signal
{
public:

    void Emit(const std::vector<Datum>& args);
    void Connect(Node* node, SignalHandlerFP func);
    void Connect(Node* node, const ScriptFunc& func);
    void Disconnect(Node* node);

private:

    void CleanupDeadConnections();

    std::unordered_map<NodePtrWeak, SignalHandlerFunc> mConnectionMap;
    std::vector <std::pair<NodePtrWeak, SignalHandlerFunc>> mPendingConnects;
    std::vector <NodePtrWeak> mPendingDisconnects;
    bool mEmitting = false;

};
