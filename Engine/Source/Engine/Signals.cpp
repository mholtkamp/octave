#include "Signals.h"
#include "Script.h"
#include "Nodes/Node.h"

void Signal::Emit(const std::vector<Datum>& args)
{
    for (auto it = mConnectionMap.begin(); it != mConnectionMap.end();)
    {
        Node* node = it->first.Get<Node>();
        if (node == nullptr)
        {
            // The NodePtr is invalid (it was destroyed)
            it = mConnectionMap.erase(it);
        }
        else
        {
            if (it->second.mFuncPointer != nullptr)
            {
                SignalHandlerFP handler = it->second.mFuncPointer;
                handler(node, args);
            }

            if (it->second.mScriptFunc.IsValid())
            {
                // Call the function as a member function (prepend self)
                std::vector<Datum> selfArgs;
                selfArgs.push_back(node);

                if (args.size() > 0)
                {
                    selfArgs.insert(selfArgs.end(), args.begin(), args.end());
                }

                it->second.mScriptFunc.Call((uint32_t)selfArgs.size(), selfArgs.data());
            }

            it++;
        }
    }
}

void Signal::Connect(Node* node, SignalHandlerFP func)
{
    if (node != nullptr)
    {
        CleanupDeadConnections();
        NodePtrWeak nodePtrWeak = Node::ResolveWeakPtr(node);
        mConnectionMap[nodePtrWeak].mFuncPointer = func;
    }
}

void Signal::Connect(Node* node, const ScriptFunc& func)
{
    if (node != nullptr)
    {
        CleanupDeadConnections();
        NodePtrWeak nodePtrWeak = Node::ResolveWeakPtr(node);
        mConnectionMap[nodePtrWeak].mScriptFunc = func;
    }
}

void Signal::Disconnect(Node* node)
{
    if (node != nullptr)
    {
        NodePtrWeak nodePtrWeak = Node::ResolveWeakPtr(node);
        mConnectionMap.erase(nodePtrWeak);
    }
}

void Signal::CleanupDeadConnections()
{
    for (auto it = mConnectionMap.begin(); it != mConnectionMap.end();)
    {
        Node* node = it->first.Get<Node>();
        if (node == nullptr)
        {
            // The NodePtr is invalid (it was destroyed)
            it = mConnectionMap.erase(it);
        }
        else
        {
            it++;
        }
    }
}
