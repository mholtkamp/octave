#include "NodeGraph/GraphClipboard.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/GraphLink.h"
#include "NodeGraph/GraphPin.h"
#include "NodeGraph/Nodes/FunctionNodes.h"
#include "NodeGraph/Nodes/VariableNodes.h"
#include "Assets/NodeGraphAsset.h"
#include "Stream.h"
#include "Log.h"
#include "System/System.h"

#include <unordered_map>
#include <unordered_set>

static const uint32_t kClipboardMagic = 0x4F435447;  // "OCTG"
static const uint32_t kClipboardVersion = 1;

// =============================================================================
// Base64 encode/decode (inline, no external dependency)
// =============================================================================
static const char kBase64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const uint8_t* data, uint32_t length)
{
    std::string result;
    result.reserve(((length + 2) / 3) * 4);

    for (uint32_t i = 0; i < length; i += 3)
    {
        uint32_t b = (data[i] << 16);
        if (i + 1 < length) b |= (data[i + 1] << 8);
        if (i + 2 < length) b |= data[i + 2];

        result.push_back(kBase64Chars[(b >> 18) & 0x3F]);
        result.push_back(kBase64Chars[(b >> 12) & 0x3F]);
        result.push_back((i + 1 < length) ? kBase64Chars[(b >> 6) & 0x3F] : '=');
        result.push_back((i + 2 < length) ? kBase64Chars[b & 0x3F] : '=');
    }

    return result;
}

static int Base64CharIndex(char c)
{
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static std::vector<uint8_t> Base64Decode(const std::string& encoded)
{
    std::vector<uint8_t> result;
    result.reserve((encoded.size() / 4) * 3);

    uint32_t val = 0;
    int bits = 0;

    for (size_t i = 0; i < encoded.size(); ++i)
    {
        if (encoded[i] == '=' || encoded[i] == '\n' || encoded[i] == '\r')
            continue;

        int idx = Base64CharIndex(encoded[i]);
        if (idx < 0)
            continue;

        val = (val << 6) | idx;
        bits += 6;

        if (bits >= 8)
        {
            bits -= 8;
            result.push_back((uint8_t)((val >> bits) & 0xFF));
        }
    }

    return result;
}

// =============================================================================
// Serialization
// =============================================================================
void GraphClipboard::SerializeSelection(NodeGraph& graph, const std::vector<GraphNodeId>& selectedNodeIds, Stream& outStream)
{
    std::unordered_set<GraphNodeId> selectedSet(selectedNodeIds.begin(), selectedNodeIds.end());

    // Header
    outStream.WriteUint32(kClipboardMagic);
    outStream.WriteUint32(kClipboardVersion);
    outStream.WriteString(graph.GetDomainName());

    // Collect selected nodes
    std::vector<GraphNode*> selectedNodes;
    for (GraphNodeId id : selectedNodeIds)
    {
        GraphNode* node = graph.FindNode(id);
        if (node != nullptr)
        {
            selectedNodes.push_back(node);
        }
    }

    // Write nodes
    outStream.WriteUint32((uint32_t)selectedNodes.size());
    for (GraphNode* node : selectedNodes)
    {
        outStream.WriteUint32((uint32_t)node->GetType());
        node->SaveStream(outStream);
    }

    // Collect internal links (both endpoints in selection)
    std::vector<GraphLink> internalLinks;
    for (const GraphLink& link : graph.GetLinks())
    {
        if (selectedSet.count(link.mOutputNodeId) > 0 && selectedSet.count(link.mInputNodeId) > 0)
        {
            internalLinks.push_back(link);
        }
    }

    // Write links
    outStream.WriteUint32((uint32_t)internalLinks.size());
    for (const GraphLink& link : internalLinks)
    {
        outStream.WriteUint32(link.mId);
        outStream.WriteUint32(link.mOutputPinId);
        outStream.WriteUint32(link.mInputPinId);
        outStream.WriteUint32(link.mOutputNodeId);
        outStream.WriteUint32(link.mInputNodeId);
    }
}

void GraphClipboard::DeserializeIntoGraph(NodeGraph& targetGraph, Stream& inStream, const glm::vec2& pastePosition, NodeGraphAsset* ownerAsset)
{
    // Read header
    uint32_t magic = inStream.ReadUint32();
    if (magic != kClipboardMagic)
    {
        LogWarning("GraphClipboard::DeserializeIntoGraph - Invalid clipboard data");
        return;
    }

    uint32_t version = inStream.ReadUint32();
    (void)version;

    std::string domainName;
    inStream.ReadString(domainName);

    // Read nodes
    uint32_t numNodes = inStream.ReadUint32();

    std::unordered_map<GraphNodeId, GraphNodeId> nodeIdMap;
    std::unordered_map<GraphPinId, GraphPinId> pinIdMap;
    std::vector<GraphNode*> newNodes;

    // Calculate centroid of source positions for offset
    glm::vec2 centroid(0.0f);
    std::vector<std::pair<TypeId, std::vector<uint8_t>>> nodeData; // Store raw data for two-pass

    // First pass: read node data to compute centroid
    uint32_t startPos = inStream.GetPos();

    for (uint32_t i = 0; i < numNodes; ++i)
    {
        TypeId nodeType = (TypeId)inStream.ReadUint32();
        GraphNode* tempNode = GraphNode::CreateInstance(nodeType);
        if (tempNode != nullptr)
        {
            tempNode->SetupPins();
            tempNode->LoadStream(inStream, ASSET_VERSION_CURRENT);
            centroid += tempNode->GetEditorPosition();
            delete tempNode;
        }
    }

    // Skip links in first pass
    uint32_t numLinksFirstPass = inStream.ReadUint32();
    for (uint32_t i = 0; i < numLinksFirstPass; ++i)
    {
        inStream.ReadUint32(); // id
        inStream.ReadUint32(); // outPinId
        inStream.ReadUint32(); // inPinId
        inStream.ReadUint32(); // outNodeId
        inStream.ReadUint32(); // inNodeId
    }

    if (numNodes > 0)
    {
        centroid /= (float)numNodes;
    }

    glm::vec2 offset = pastePosition - centroid;

    // Second pass: actually create nodes with remapped IDs
    inStream.SetPos(startPos);

    for (uint32_t i = 0; i < numNodes; ++i)
    {
        TypeId nodeType = (TypeId)inStream.ReadUint32();
        GraphNode* node = GraphNode::CreateInstance(nodeType);
        if (node == nullptr)
        {
            LogWarning("GraphClipboard: Failed to create node type %u", (uint32_t)nodeType);
            continue;
        }

        node->SetupPins();
        node->LoadStream(inStream, ASSET_VERSION_CURRENT);

        // Remap node ID
        GraphNodeId oldNodeId = node->GetId();
        GraphNodeId newNodeId = targetGraph.AllocNodeId();
        nodeIdMap[oldNodeId] = newNodeId;
        node->SetId(newNodeId);

        // Offset position
        node->SetEditorPosition(node->GetEditorPosition() + offset);

        // Remap pin IDs
        for (uint32_t j = 0; j < node->GetNumInputPins(); ++j)
        {
            GraphPin& pin = node->GetInputPins()[j];
            GraphPinId oldPinId = pin.mId;
            GraphPinId newPinId = targetGraph.AllocPinId();
            pinIdMap[oldPinId] = newPinId;
            pin.mId = newPinId;
            pin.mOwnerNodeId = newNodeId;
        }
        for (uint32_t j = 0; j < node->GetNumOutputPins(); ++j)
        {
            GraphPin& pin = node->GetOutputPins()[j];
            GraphPinId oldPinId = pin.mId;
            GraphPinId newPinId = targetGraph.AllocPinId();
            pinIdMap[oldPinId] = newPinId;
            pin.mId = newPinId;
            pin.mOwnerNodeId = newNodeId;
        }

        // Wire special nodes to owner asset
        if (ownerAsset != nullptr)
        {
            if (node->GetType() == FunctionCallNode::GetStaticType())
            {
                static_cast<FunctionCallNode*>(node)->SetOwnerAsset(ownerAsset);
            }
            else if (node->GetType() == GetVariableNode::GetStaticType())
            {
                static_cast<GetVariableNode*>(node)->SetOwnerAsset(ownerAsset);
            }
            else if (node->GetType() == SetVariableNode::GetStaticType())
            {
                static_cast<SetVariableNode*>(node)->SetOwnerAsset(ownerAsset);
            }
        }

        targetGraph.InsertNode(node);
        newNodes.push_back(node);
    }

    // Read and remap links
    uint32_t numLinks = inStream.ReadUint32();
    for (uint32_t i = 0; i < numLinks; ++i)
    {
        GraphLinkId linkId = inStream.ReadUint32();
        GraphPinId outPinId = inStream.ReadUint32();
        GraphPinId inPinId = inStream.ReadUint32();
        GraphNodeId outNodeId = inStream.ReadUint32();
        GraphNodeId inNodeId = inStream.ReadUint32();
        (void)linkId;
        (void)outNodeId;
        (void)inNodeId;

        // Remap pin IDs
        auto outIt = pinIdMap.find(outPinId);
        auto inIt = pinIdMap.find(inPinId);
        if (outIt != pinIdMap.end() && inIt != pinIdMap.end())
        {
            targetGraph.AddLink(outIt->second, inIt->second);
        }
    }
}

void GraphClipboard::CopyToClipboard(NodeGraph& graph, const std::vector<GraphNodeId>& selectedNodeIds)
{
    if (selectedNodeIds.empty())
        return;

    Stream stream;
    SerializeSelection(graph, selectedNodeIds, stream);

    // Base64 encode and copy to system clipboard
    uint32_t dataSize = stream.GetPos();
    stream.SetPos(0);
    std::vector<uint8_t> data(dataSize);
    stream.ReadBytes(data.data(), dataSize);

    std::string encoded = Base64Encode(data.data(), dataSize);
    SYS_SetClipboardText(encoded);
}

void GraphClipboard::PasteFromClipboard(NodeGraph& targetGraph, const glm::vec2& pastePosition, NodeGraphAsset* ownerAsset)
{
    std::string clipText = SYS_GetClipboardText();
    if (clipText.empty())
        return;

    std::vector<uint8_t> data = Base64Decode(clipText);
    if (data.size() < 8)
        return;

    // Quick magic number check
    uint32_t magic = (data[0]) | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    if (magic != kClipboardMagic)
        return;

    Stream stream;
    stream.WriteBytes(data.data(), (uint32_t)data.size());
    stream.SetPos(0);

    DeserializeIntoGraph(targetGraph, stream, pastePosition, ownerAsset);
}

void GraphClipboard::ExportToFile(NodeGraph& graph, const std::vector<GraphNodeId>& selectedNodeIds, const std::string& filePath)
{
    if (selectedNodeIds.empty())
        return;

    Stream stream;
    SerializeSelection(graph, selectedNodeIds, stream);
    stream.WriteFile(filePath.c_str());
}

void GraphClipboard::ImportFromFile(NodeGraph& targetGraph, const std::string& filePath, const glm::vec2& pastePosition, NodeGraphAsset* ownerAsset)
{
    Stream stream;
    if (!stream.ReadFile(filePath.c_str(), false))
    {
        LogWarning("GraphClipboard::ImportFromFile - Failed to read file: %s", filePath.c_str());
        return;
    }

    DeserializeIntoGraph(targetGraph, stream, pastePosition, ownerAsset);
}
