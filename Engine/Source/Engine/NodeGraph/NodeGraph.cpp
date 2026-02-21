#include "NodeGraph/NodeGraph.h"
#include "Stream.h"
#include "Log.h"

#include <queue>
#include <unordered_map>
#include <unordered_set>

NodeGraph::NodeGraph()
{
}

NodeGraph::~NodeGraph()
{
    Clear();
}

GraphNode* NodeGraph::AddNode(TypeId nodeType)
{
    GraphNode* node = GraphNode::CreateInstance(nodeType);
    if (node != nullptr)
    {
        node->SetId(mNextNodeId++);
        node->SetupPins();

        // Reassign pin IDs with graph-global uniqueness
        for (uint32_t i = 0; i < node->GetNumInputPins(); ++i)
        {
            node->GetInputPins()[i].mId = mNextPinId++;
            node->GetInputPins()[i].mOwnerNodeId = node->GetId();
        }
        for (uint32_t i = 0; i < node->GetNumOutputPins(); ++i)
        {
            node->GetOutputPins()[i].mId = mNextPinId++;
            node->GetOutputPins()[i].mOwnerNodeId = node->GetId();
        }

        mNodes.push_back(node);
    }
    return node;
}

void NodeGraph::RemoveNode(GraphNodeId id)
{
    // Remove all links connected to this node
    for (int32_t i = (int32_t)mLinks.size() - 1; i >= 0; --i)
    {
        if (mLinks[i].mOutputNodeId == id || mLinks[i].mInputNodeId == id)
        {
            mLinks.erase(mLinks.begin() + i);
        }
    }

    // Remove the node
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        if (mNodes[i]->GetId() == id)
        {
            delete mNodes[i];
            mNodes.erase(mNodes.begin() + i);
            break;
        }
    }
}

GraphNode* NodeGraph::FindNode(GraphNodeId id) const
{
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        if (mNodes[i]->GetId() == id)
        {
            return mNodes[i];
        }
    }
    return nullptr;
}

GraphLink* NodeGraph::AddLink(GraphPinId outputPinId, GraphPinId inputPinId)
{
    GraphPin* outputPin = FindPin(outputPinId);
    GraphPin* inputPin = FindPin(inputPinId);

    if (outputPin == nullptr || inputPin == nullptr)
    {
        LogWarning("NodeGraph::AddLink - Invalid pin IDs");
        return nullptr;
    }

    if (outputPin->mDirection != GraphPinDirection::Output ||
        inputPin->mDirection != GraphPinDirection::Input)
    {
        LogWarning("NodeGraph::AddLink - Wrong pin directions");
        return nullptr;
    }

    if (!AreGraphPinTypesCompatible(outputPin->mDataType, inputPin->mDataType))
    {
        LogWarning("NodeGraph::AddLink - Incompatible pin types");
        return nullptr;
    }

    // Check for cycles
    GraphNode* outputNode = FindPinOwner(outputPinId);
    GraphNode* inputNode = FindPinOwner(inputPinId);
    if (outputNode == nullptr || inputNode == nullptr)
    {
        return nullptr;
    }

    if (outputNode->GetId() == inputNode->GetId())
    {
        LogWarning("NodeGraph::AddLink - Cannot link node to itself");
        return nullptr;
    }

    if (WouldCreateCycle(outputNode->GetId(), inputNode->GetId()))
    {
        LogWarning("NodeGraph::AddLink - Would create cycle");
        return nullptr;
    }

    // Remove existing link to this input pin (one link per input)
    for (int32_t i = (int32_t)mLinks.size() - 1; i >= 0; --i)
    {
        if (mLinks[i].mInputPinId == inputPinId)
        {
            mLinks.erase(mLinks.begin() + i);
            break;
        }
    }

    GraphLink link;
    link.mId = mNextLinkId++;
    link.mOutputPinId = outputPinId;
    link.mInputPinId = inputPinId;
    link.mOutputNodeId = outputNode->GetId();
    link.mInputNodeId = inputNode->GetId();

    mLinks.push_back(link);
    return &mLinks.back();
}

void NodeGraph::RemoveLink(GraphLinkId id)
{
    for (uint32_t i = 0; i < mLinks.size(); ++i)
    {
        if (mLinks[i].mId == id)
        {
            mLinks.erase(mLinks.begin() + i);
            break;
        }
    }
}

GraphLink* NodeGraph::FindLink(GraphLinkId id) const
{
    for (uint32_t i = 0; i < mLinks.size(); ++i)
    {
        if (mLinks[i].mId == id)
        {
            return const_cast<GraphLink*>(&mLinks[i]);
        }
    }
    return nullptr;
}

GraphLink* NodeGraph::FindLinkByInputPin(GraphPinId inputPinId) const
{
    for (uint32_t i = 0; i < mLinks.size(); ++i)
    {
        if (mLinks[i].mInputPinId == inputPinId)
        {
            return const_cast<GraphLink*>(&mLinks[i]);
        }
    }
    return nullptr;
}

GraphPin* NodeGraph::FindPin(GraphPinId id) const
{
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        for (uint32_t j = 0; j < mNodes[i]->GetNumInputPins(); ++j)
        {
            if (mNodes[i]->GetInputPins()[j].mId == id)
            {
                return const_cast<GraphPin*>(&mNodes[i]->GetInputPins()[j]);
            }
        }
        for (uint32_t j = 0; j < mNodes[i]->GetNumOutputPins(); ++j)
        {
            if (mNodes[i]->GetOutputPins()[j].mId == id)
            {
                return const_cast<GraphPin*>(&mNodes[i]->GetOutputPins()[j]);
            }
        }
    }
    return nullptr;
}

GraphNode* NodeGraph::FindPinOwner(GraphPinId id) const
{
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        for (uint32_t j = 0; j < mNodes[i]->GetNumInputPins(); ++j)
        {
            if (mNodes[i]->GetInputPins()[j].mId == id)
            {
                return mNodes[i];
            }
        }
        for (uint32_t j = 0; j < mNodes[i]->GetNumOutputPins(); ++j)
        {
            if (mNodes[i]->GetOutputPins()[j].mId == id)
            {
                return mNodes[i];
            }
        }
    }
    return nullptr;
}

bool NodeGraph::WouldCreateCycle(GraphNodeId fromNodeId, GraphNodeId toNodeId) const
{
    // BFS from toNode to see if we can reach fromNode through existing links
    std::queue<GraphNodeId> queue;
    std::unordered_set<GraphNodeId> visited;

    queue.push(toNodeId);
    visited.insert(toNodeId);

    while (!queue.empty())
    {
        GraphNodeId current = queue.front();
        queue.pop();

        if (current == fromNodeId)
        {
            return true;
        }

        // Follow outgoing links from current node
        for (uint32_t i = 0; i < mLinks.size(); ++i)
        {
            if (mLinks[i].mOutputNodeId == current)
            {
                GraphNodeId nextId = mLinks[i].mInputNodeId;
                if (visited.find(nextId) == visited.end())
                {
                    visited.insert(nextId);
                    queue.push(nextId);
                }
            }
        }
    }

    return false;
}

void NodeGraph::SaveStream(Stream& stream)
{
    stream.WriteString(mDomainName);

    // Save nodes
    stream.WriteUint32((uint32_t)mNodes.size());
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        stream.WriteUint32((uint32_t)mNodes[i]->GetType());
        mNodes[i]->SaveStream(stream);
    }

    // Save links
    stream.WriteUint32((uint32_t)mLinks.size());
    for (uint32_t i = 0; i < mLinks.size(); ++i)
    {
        stream.WriteUint32(mLinks[i].mId);
        stream.WriteUint32(mLinks[i].mOutputPinId);
        stream.WriteUint32(mLinks[i].mInputPinId);
        stream.WriteUint32(mLinks[i].mOutputNodeId);
        stream.WriteUint32(mLinks[i].mInputNodeId);
    }

    // Save ID counters
    stream.WriteUint32(mNextNodeId);
    stream.WriteUint32(mNextLinkId);
}

void NodeGraph::LoadStream(Stream& stream, uint32_t version)
{
    Clear();

    stream.ReadString(mDomainName);

    // Load nodes
    uint32_t numNodes = stream.ReadUint32();
    for (uint32_t i = 0; i < numNodes; ++i)
    {
        TypeId nodeType = (TypeId)stream.ReadUint32();
        GraphNode* node = GraphNode::CreateInstance(nodeType);

        if (node != nullptr)
        {
            node->SetupPins();
            node->LoadStream(stream, version);
            mNodes.push_back(node);
        }
        else
        {
            LogWarning("NodeGraph::LoadStream - Failed to create node of type %u", (uint32_t)nodeType);
        }
    }

    // Load links
    uint32_t numLinks = stream.ReadUint32();
    for (uint32_t i = 0; i < numLinks; ++i)
    {
        GraphLink link;
        link.mId = stream.ReadUint32();
        link.mOutputPinId = stream.ReadUint32();
        link.mInputPinId = stream.ReadUint32();
        link.mOutputNodeId = stream.ReadUint32();
        link.mInputNodeId = stream.ReadUint32();
        mLinks.push_back(link);
    }

    // Load ID counters
    mNextNodeId = stream.ReadUint32();
    mNextLinkId = stream.ReadUint32();

    // Compute mNextPinId from loaded pin data
    mNextPinId = 1;
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        for (uint32_t j = 0; j < mNodes[i]->GetNumInputPins(); ++j)
        {
            GraphPinId id = mNodes[i]->GetInputPins()[j].mId;
            if (id >= mNextPinId)
                mNextPinId = id + 1;
        }
        for (uint32_t j = 0; j < mNodes[i]->GetNumOutputPins(); ++j)
        {
            GraphPinId id = mNodes[i]->GetOutputPins()[j].mId;
            if (id >= mNextPinId)
                mNextPinId = id + 1;
        }
    }

    // Clean up duplicate output nodes (keep the first one found)
    bool foundOutputNode = false;
    for (int32_t i = (int32_t)mNodes.size() - 1; i >= 0; --i)
    {
        GraphNode* node = mNodes[i];
        if (node->GetNumOutputPins() == 0 && node->GetNumInputPins() > 0 && !node->IsInputNode())
        {
            if (foundOutputNode)
            {
                // Duplicate — remove links and delete
                GraphNodeId nodeId = node->GetId();
                for (int32_t j = (int32_t)mLinks.size() - 1; j >= 0; --j)
                {
                    if (mLinks[j].mOutputNodeId == nodeId || mLinks[j].mInputNodeId == nodeId)
                    {
                        mLinks.erase(mLinks.begin() + j);
                    }
                }
                delete node;
                mNodes.erase(mNodes.begin() + i);
                LogWarning("NodeGraph::LoadStream - Removed duplicate output node");
            }
            else
            {
                foundOutputNode = true;
            }
        }
    }
}

void NodeGraph::Clear()
{
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        delete mNodes[i];
    }
    mNodes.clear();
    mLinks.clear();
    mNextNodeId = 1;
    mNextLinkId = 1;
    mNextPinId = 1;
}

void NodeGraph::CopyFrom(const NodeGraph& other)
{
    Clear();

    mDomainName = other.mDomainName;
    mNextNodeId = other.mNextNodeId;
    mNextLinkId = other.mNextLinkId;
    mNextPinId = other.mNextPinId;

    // Map old node IDs to new nodes
    std::unordered_map<GraphNodeId, GraphNode*> nodeMap;

    for (uint32_t i = 0; i < other.mNodes.size(); ++i)
    {
        GraphNode* srcNode = other.mNodes[i];
        TypeId type = srcNode->GetType();

        GraphNode* newNode = GraphNode::CreateInstance(type);
        if (newNode == nullptr)
            continue;

        newNode->SetupPins();
        newNode->SetId(srcNode->GetId());
        newNode->SetEditorPosition(srcNode->GetEditorPosition());

        // Copy input pin IDs and values
        for (uint32_t j = 0; j < srcNode->GetNumInputPins() && j < newNode->GetNumInputPins(); ++j)
        {
            newNode->GetInputPins()[j].mId = srcNode->GetInputPins()[j].mId;
            newNode->GetInputPins()[j].mOwnerNodeId = srcNode->GetId();
            newNode->GetInputPins()[j].mDefaultValue = srcNode->GetInputPins()[j].mDefaultValue;
            newNode->GetInputPins()[j].mValue = srcNode->GetInputPins()[j].mValue;
        }

        // Copy output pin IDs
        for (uint32_t j = 0; j < srcNode->GetNumOutputPins() && j < newNode->GetNumOutputPins(); ++j)
        {
            newNode->GetOutputPins()[j].mId = srcNode->GetOutputPins()[j].mId;
            newNode->GetOutputPins()[j].mOwnerNodeId = srcNode->GetId();
        }

        // Copy input name for input nodes
        if (srcNode->IsInputNode())
        {
            newNode->SetInputName(srcNode->GetInputName());
        }

        nodeMap[srcNode->GetId()] = newNode;
        mNodes.push_back(newNode);
    }

    // Copy links
    mLinks = other.mLinks;
}

GraphNode* NodeGraph::FindInputNode(const char* inputName) const
{
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        if (mNodes[i]->IsInputNode() &&
            mNodes[i]->GetInputName() == inputName)
        {
            return mNodes[i];
        }
    }
    return nullptr;
}

bool NodeGraph::SetInputFloat(const char* inputName, float value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

bool NodeGraph::SetInputInt(const char* inputName, int32_t value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

bool NodeGraph::SetInputBool(const char* inputName, bool value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

bool NodeGraph::SetInputString(const char* inputName, const char* value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(std::string(value));
        node->GetInputPins()[0].mValue = Datum(std::string(value));
        return true;
    }
    return false;
}

bool NodeGraph::SetInputVector(const char* inputName, const glm::vec3& value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

bool NodeGraph::SetInputColor(const char* inputName, const glm::vec4& value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

bool NodeGraph::SetInputByte(const char* inputName, uint8_t value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

bool NodeGraph::SetInputAsset(const char* inputName, Asset* value)
{
    GraphNode* node = FindInputNode(inputName);
    if (node != nullptr && node->GetNumInputPins() > 0)
    {
        node->GetInputPins()[0].mDefaultValue = Datum(value);
        node->GetInputPins()[0].mValue = Datum(value);
        return true;
    }
    return false;
}

GraphNode* NodeGraph::FindOutputNode() const
{
    // Output nodes are sink nodes with no output pins.
    // Typically there is only one per graph (e.g. MaterialOutputNode).
    for (uint32_t i = 0; i < mNodes.size(); ++i)
    {
        if (mNodes[i]->GetNumOutputPins() == 0 && mNodes[i]->GetNumInputPins() > 0 &&
            !mNodes[i]->IsInputNode())
        {
            return mNodes[i];
        }
    }
    return nullptr;
}

const Datum& NodeGraph::GetOutputValue(uint32_t pinIndex) const
{
    GraphNode* output = FindOutputNode();
    if (output != nullptr)
    {
        return output->GetInputValue(pinIndex);
    }

    static Datum sNullDatum;
    return sNullDatum;
}
