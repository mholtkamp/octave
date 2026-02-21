#include "NodeGraph/GraphProcessor.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/GraphLink.h"
#include "NodeGraph/GraphPin.h"
#include "Log.h"

#include <unordered_map>
#include <queue>

void GraphProcessor::Evaluate(NodeGraph* graph)
{
    if (graph == nullptr)
    {
        return;
    }

    TopologicalSort(graph);

    if (mHasCycles)
    {
        LogWarning("GraphProcessor::Evaluate - Graph has cycles, cannot evaluate");
        return;
    }

    PropagateValues(graph);

    // Evaluate nodes in topological order
    for (uint32_t i = 0; i < mEvaluationOrder.size(); ++i)
    {
        GraphNode* node = graph->FindNode(mEvaluationOrder[i]);
        if (node != nullptr)
        {
            node->Evaluate();
        }
    }
}

void GraphProcessor::TopologicalSort(NodeGraph* graph)
{
    mEvaluationOrder.clear();
    mHasCycles = false;

    const std::vector<GraphNode*>& nodes = graph->GetNodes();
    const std::vector<GraphLink>& links = graph->GetLinks();

    if (nodes.empty())
    {
        return;
    }

    // Build in-degree map
    std::unordered_map<GraphNodeId, uint32_t> inDegree;
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        inDegree[nodes[i]->GetId()] = 0;
    }

    for (uint32_t i = 0; i < links.size(); ++i)
    {
        inDegree[links[i].mInputNodeId]++;
    }

    // Start with nodes that have no incoming edges
    std::queue<GraphNodeId> queue;
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        if (inDegree[nodes[i]->GetId()] == 0)
        {
            queue.push(nodes[i]->GetId());
        }
    }

    while (!queue.empty())
    {
        GraphNodeId current = queue.front();
        queue.pop();
        mEvaluationOrder.push_back(current);

        // Reduce in-degree of downstream nodes
        for (uint32_t i = 0; i < links.size(); ++i)
        {
            if (links[i].mOutputNodeId == current)
            {
                GraphNodeId downstream = links[i].mInputNodeId;
                inDegree[downstream]--;
                if (inDegree[downstream] == 0)
                {
                    queue.push(downstream);
                }
            }
        }
    }

    if (mEvaluationOrder.size() != nodes.size())
    {
        mHasCycles = true;
        mEvaluationOrder.clear();
    }
}

void GraphProcessor::PropagateValues(NodeGraph* graph)
{
    const std::vector<GraphLink>& links = graph->GetLinks();

    // Reset all input pin values to their defaults
    const std::vector<GraphNode*>& nodes = graph->GetNodes();
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        for (uint32_t j = 0; j < nodes[i]->GetNumInputPins(); ++j)
        {
            GraphPin* pin = nodes[i]->GetInputPin(j);
            if (pin != nullptr)
            {
                pin->mValue = pin->mDefaultValue;
            }
        }
    }

    // For each node in evaluation order, after evaluation, propagate outputs through links
    for (uint32_t orderIdx = 0; orderIdx < mEvaluationOrder.size(); ++orderIdx)
    {
        GraphNode* node = graph->FindNode(mEvaluationOrder[orderIdx]);
        if (node == nullptr)
        {
            continue;
        }

        // Propagate this node's output values to connected input pins
        for (uint32_t i = 0; i < links.size(); ++i)
        {
            if (links[i].mOutputNodeId == node->GetId())
            {
                GraphPin* outputPin = graph->FindPin(links[i].mOutputPinId);
                GraphPin* inputPin = graph->FindPin(links[i].mInputPinId);

                if (outputPin != nullptr && inputPin != nullptr)
                {
                    inputPin->mValue = outputPin->mValue;
                }
            }
        }
    }
}
