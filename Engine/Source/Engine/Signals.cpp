#include "Signals.h"
#include "Script.h"
#include "Nodes/Node.h"

void Signal::Emit(const std::vector<Datum>& args)
{
    mEmitting = true;

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

    mEmitting = false;

    // Handle any pending connects/disconnects
    for (uint32_t i = 0; i < mPendingDisconnects.size(); ++i)
    {
        mConnectionMap.erase(mPendingDisconnects[i]);
    }
    for (uint32_t i = 0; i < mPendingConnects.size(); ++i)
    {
        mConnectionMap.insert({ mPendingConnects[i] });
    }
}

void Signal::Connect(Node* node, SignalHandlerFP func)
{
    if (node != nullptr)
    {
        CleanupDeadConnections();
        NodePtrWeak nodePtrWeak = Node::ResolveWeakPtr(node);

        if (mEmitting)
        {
            SignalHandlerFunc handler;
            handler.mFuncPointer = func;
            mPendingConnects.push_back({ nodePtrWeak, handler });
        }
        else
        {
            mConnectionMap[nodePtrWeak].mFuncPointer = func;
        }
    }
}

void Signal::Connect(Node* node, const ScriptFunc& func)
{
    if (node != nullptr)
    {
        CleanupDeadConnections();
        NodePtrWeak nodePtrWeak = Node::ResolveWeakPtr(node);

        if (mEmitting)
        {
            SignalHandlerFunc handler;
            handler.mScriptFunc = func;
            mPendingConnects.push_back({ nodePtrWeak, handler });
        }
        else
        {
            mConnectionMap[nodePtrWeak].mScriptFunc = func;
        }
    }
}

void Signal::Disconnect(Node* node)
{
    if (node != nullptr)
    {
        NodePtrWeak nodePtrWeak = Node::ResolveWeakPtr(node);

        if (mEmitting)
        {
            mPendingDisconnects.push_back(nodePtrWeak);
        }
        else
        {
            mConnectionMap.erase(nodePtrWeak);
        }
    }
}

void Signal::CleanupDeadConnections()
{
    if (mEmitting)
        return;

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
