#pragma once

#include <string>
#include <vector>

#include "Maths.h"
#include "NodeGraph/GraphTypes.h"

class NodeGraph;
class NodeGraphAsset;
class Stream;

namespace GraphClipboard
{
    // Serialize selected nodes + internal links to a stream
    void SerializeSelection(NodeGraph& graph, const std::vector<GraphNodeId>& selectedNodeIds, Stream& outStream);

    // Deserialize nodes/links from a stream into the target graph, remapping IDs
    void DeserializeIntoGraph(NodeGraph& targetGraph, Stream& inStream, const glm::vec2& pastePosition, NodeGraphAsset* ownerAsset);

    // Copy selected nodes to system clipboard (base64 encoded)
    void CopyToClipboard(NodeGraph& graph, const std::vector<GraphNodeId>& selectedNodeIds);

    // Paste from system clipboard into graph
    void PasteFromClipboard(NodeGraph& targetGraph, const glm::vec2& pastePosition, NodeGraphAsset* ownerAsset);

    // Export selected nodes to a file
    void ExportToFile(NodeGraph& graph, const std::vector<GraphNodeId>& selectedNodeIds, const std::string& filePath);

    // Import nodes from a file into the graph
    void ImportFromFile(NodeGraph& targetGraph, const std::string& filePath, const glm::vec2& pastePosition, NodeGraphAsset* ownerAsset);
}
