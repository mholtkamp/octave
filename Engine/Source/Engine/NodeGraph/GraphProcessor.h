#pragma once

#include "NodeGraph/GraphTypes.h"

#include <vector>

class NodeGraph;

class GraphProcessor
{
public:

    void Evaluate(NodeGraph* graph);

    const std::vector<GraphNodeId>& GetEvaluationOrder() const { return mEvaluationOrder; }
    bool HasCycles() const { return mHasCycles; }

private:

    void TopologicalSort(NodeGraph* graph);
    void PropagateValues(NodeGraph* graph);

    std::vector<GraphNodeId> mEvaluationOrder;
    bool mHasCycles = false;
};
