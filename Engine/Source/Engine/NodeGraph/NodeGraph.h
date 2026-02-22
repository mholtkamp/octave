#pragma once

#include "NodeGraph/GraphTypes.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/GraphLink.h"
#include "NodeGraph/GraphPin.h"

#include <string>
#include <unordered_set>
#include <vector>

class Asset;
class Node;
class Stream;

class NodeGraph
{
public:

    NodeGraph();
    ~NodeGraph();

    // Node CRUD
    GraphNode* AddNode(TypeId nodeType);
    void RemoveNode(GraphNodeId id);
    GraphNode* FindNode(GraphNodeId id) const;

    uint32_t GetNumNodes() const { return (uint32_t)mNodes.size(); }
    const std::vector<GraphNode*>& GetNodes() const { return mNodes; }

    // Link CRUD
    GraphLink* AddLink(GraphPinId outputPinId, GraphPinId inputPinId);
    void RemoveLink(GraphLinkId id);
    GraphLink* FindLink(GraphLinkId id) const;
    GraphLink* FindLinkByInputPin(GraphPinId inputPinId) const;

    uint32_t GetNumLinks() const { return (uint32_t)mLinks.size(); }
    const std::vector<GraphLink>& GetLinks() const { return mLinks; }

    // Pin lookup
    GraphPin* FindPin(GraphPinId id) const;
    GraphNode* FindPinOwner(GraphPinId id) const;

    // Cycle detection
    bool WouldCreateCycle(GraphNodeId fromNodeId, GraphNodeId toNodeId) const;

    // Serialization
    void SaveStream(Stream& stream);
    void LoadStream(Stream& stream, uint32_t version);

    // Domain
    const std::string& GetDomainName() const { return mDomainName; }
    void SetDomainName(const std::string& name) { mDomainName = name; }

    // Graph name (empty = main graph, non-empty = function graph)
    const std::string& GetGraphName() const { return mGraphName; }
    void SetGraphName(const std::string& name) { mGraphName = name; }

    void Clear();

    // ID allocation (for external node insertion)
    GraphNodeId AllocNodeId() { return mNextNodeId++; }
    GraphPinId AllocPinId() { return mNextPinId++; }
    GraphLinkId AllocLinkId() { return mNextLinkId++; }

    // Insert an externally-created node (takes ownership)
    void InsertNode(GraphNode* node);

    // Deep copy for runtime (independent from asset)
    void CopyFrom(const NodeGraph& other);

    // Owner node (for gameplay graphs)
    void SetOwnerNode(Node* node) { mOwnerNode = node; }
    Node* GetOwnerNode() const { return mOwnerNode; }

    // Find input nodes by name
    GraphNode* FindInputNode(const char* inputName) const;

    // Convenience setters (find input node, write to its pin)
    bool SetInputFloat(const char* inputName, float value);
    bool SetInputInt(const char* inputName, int32_t value);
    bool SetInputBool(const char* inputName, bool value);
    bool SetInputString(const char* inputName, const char* value);
    bool SetInputVector(const char* inputName, const glm::vec3& value);
    bool SetInputColor(const char* inputName, const glm::vec4& value);
    bool SetInputByte(const char* inputName, uint8_t value);
    bool SetInputAsset(const char* inputName, Asset* value);

    // Read output node's computed values after evaluation
    GraphNode* FindOutputNode() const;
    const Datum& GetOutputValue(uint32_t pinIndex) const;

    // Execution tracking — accumulated by GraphProcessor, read/cleared by editor
    std::unordered_set<GraphPinId> mExecutedPinIds;

private:

    std::vector<GraphNode*> mNodes;
    std::vector<GraphLink> mLinks;
    std::string mDomainName;
    std::string mGraphName;
    Node* mOwnerNode = nullptr;

    GraphNodeId mNextNodeId = 1;
    GraphLinkId mNextLinkId = 1;
    GraphPinId mNextPinId = 1;
};
