#pragma once

#include "NodeGraph/GraphTypes.h"

#include <vector>

class GraphNode;
class NodeGraph;

class GraphProcessor
{
public:

    void Evaluate(NodeGraph* graph, const char* eventName = nullptr);

    const std::vector<GraphNodeId>& GetEvaluationOrder() const { return mEvaluationOrder; }
    bool HasCycles() const { return mHasCycles; }

private:

    void TopologicalSort(NodeGraph* graph);
    void PropagateValues(NodeGraph* graph);
    void ResetExecutionFlags(NodeGraph* graph);
    bool HasExecutionInputPin(GraphNode* node) const;
    bool HasAnyExecutionTriggered(GraphNode* node) const;
    void PropagateExecutionOutputs(NodeGraph* graph, GraphNode* node);

    std::vector<GraphNodeId> mEvaluationOrder;
    bool mHasCycles = false;
};
