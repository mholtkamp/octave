#include "NodeGraph/GraphProcessor.h"
#include "NodeGraph/NodeGraph.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/GraphLink.h"
#include "NodeGraph/GraphPin.h"
#include "Log.h"

#include <unordered_map>
#include <queue>
#include <cstring>

void GraphProcessor::Evaluate(NodeGraph* graph, const char* eventName)
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
    ResetExecutionFlags(graph);

    // If an event name is specified, find matching event nodes and trigger them
    if (eventName != nullptr)
    {
        for (uint32_t i = 0; i < mEvaluationOrder.size(); ++i)
        {
            GraphNode* node = graph->FindNode(mEvaluationOrder[i]);
            if (node != nullptr && node->IsEventNode() &&
                strcmp(node->GetEventName(), eventName) == 0)
            {
                node->Evaluate();
                node->TriggerExecutionPin(0);
            }
        }
    }

    // Evaluate nodes in topological order
    for (uint32_t i = 0; i < mEvaluationOrder.size(); ++i)
    {
        GraphNode* node = graph->FindNode(mEvaluationOrder[i]);
        if (node == nullptr)
        {
            continue;
        }

        // Skip event nodes - already evaluated above when triggering
        if (node->IsEventNode())
        {
            // Still propagate data outputs from event nodes (e.g. TickEventNode's DeltaTime)
            const std::vector<GraphLink>& links = graph->GetLinks();
            for (uint32_t j = 0; j < links.size(); ++j)
            {
                if (links[j].mOutputNodeId == node->GetId())
                {
                    GraphPin* outputPin = graph->FindPin(links[j].mOutputPinId);
                    GraphPin* inputPin = graph->FindPin(links[j].mInputPinId);

                    if (outputPin != nullptr && inputPin != nullptr &&
                        outputPin->mDataType != DatumType::Execution)
                    {
                        inputPin->mValue = outputPin->mValue;
                    }
                }
            }

            PropagateExecutionOutputs(graph, node);
            continue;
        }

        // Nodes with no execution input pins always evaluate (backward compatible).
        // Nodes with execution input pins only evaluate if at least one is triggered.
        if (!HasExecutionInputPin(node) || HasAnyExecutionTriggered(node))
        {
            node->Evaluate();

            // After evaluation, propagate data outputs to connected input pins
            const std::vector<GraphLink>& links = graph->GetLinks();
            for (uint32_t j = 0; j < links.size(); ++j)
            {
                if (links[j].mOutputNodeId == node->GetId())
                {
                    GraphPin* outputPin = graph->FindPin(links[j].mOutputPinId);
                    GraphPin* inputPin = graph->FindPin(links[j].mInputPinId);

                    if (outputPin != nullptr && inputPin != nullptr &&
                        outputPin->mDataType != DatumType::Execution)
                    {
                        inputPin->mValue = outputPin->mValue;
                    }
                }
            }

            // Propagate triggered exec output pins to downstream exec input pins
            PropagateExecutionOutputs(graph, node);
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
            if (pin != nullptr && pin->mDataType != DatumType::Execution)
            {
                pin->mValue = pin->mDefaultValue;
            }
        }
    }

    // Pre-propagate values from nodes that have no execution input pins
    // (data-flow nodes that always evaluate). This ensures data is available
    // before execution-aware evaluation begins.
    for (uint32_t orderIdx = 0; orderIdx < mEvaluationOrder.size(); ++orderIdx)
    {
        GraphNode* node = graph->FindNode(mEvaluationOrder[orderIdx]);
        if (node == nullptr)
        {
            continue;
        }

        // Only pre-propagate for pure data nodes (no exec inputs)
        if (HasExecutionInputPin(node))
        {
            continue;
        }

        for (uint32_t i = 0; i < links.size(); ++i)
        {
            if (links[i].mOutputNodeId == node->GetId())
            {
                GraphPin* outputPin = graph->FindPin(links[i].mOutputPinId);
                GraphPin* inputPin = graph->FindPin(links[i].mInputPinId);

                if (outputPin != nullptr && inputPin != nullptr &&
                    outputPin->mDataType != DatumType::Execution)
                {
                    inputPin->mValue = outputPin->mValue;
                }
            }
        }
    }
}

void GraphProcessor::ResetExecutionFlags(NodeGraph* graph)
{
    const std::vector<GraphNode*>& nodes = graph->GetNodes();
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        for (uint32_t j = 0; j < nodes[i]->GetNumInputPins(); ++j)
        {
            GraphPin* pin = nodes[i]->GetInputPin(j);
            if (pin != nullptr && pin->mDataType == DatumType::Execution)
            {
                pin->mExecutionTriggered = false;
            }
        }
        for (uint32_t j = 0; j < nodes[i]->GetNumOutputPins(); ++j)
        {
            GraphPin* pin = nodes[i]->GetOutputPin(j);
            if (pin != nullptr && pin->mDataType == DatumType::Execution)
            {
                pin->mExecutionTriggered = false;
            }
        }
    }
}

bool GraphProcessor::HasExecutionInputPin(GraphNode* node) const
{
    for (uint32_t i = 0; i < node->GetNumInputPins(); ++i)
    {
        if (node->GetInputPins()[i].mDataType == DatumType::Execution)
        {
            return true;
        }
    }
    return false;
}

bool GraphProcessor::HasAnyExecutionTriggered(GraphNode* node) const
{
    for (uint32_t i = 0; i < node->GetNumInputPins(); ++i)
    {
        if (node->GetInputPins()[i].mDataType == DatumType::Execution &&
            node->GetInputPins()[i].mExecutionTriggered)
        {
            return true;
        }
    }
    return false;
}

void GraphProcessor::PropagateExecutionOutputs(NodeGraph* graph, GraphNode* node)
{
    const std::vector<GraphLink>& links = graph->GetLinks();

    for (uint32_t i = 0; i < node->GetNumOutputPins(); ++i)
    {
        const GraphPin& outPin = node->GetOutputPins()[i];
        if (outPin.mDataType == DatumType::Execution && outPin.mExecutionTriggered)
        {
            for (uint32_t j = 0; j < links.size(); ++j)
            {
                if (links[j].mOutputPinId == outPin.mId)
                {
                    GraphPin* inputPin = graph->FindPin(links[j].mInputPinId);
                    if (inputPin != nullptr && inputPin->mDataType == DatumType::Execution)
                    {
                        inputPin->mExecutionTriggered = true;
                    }
                }
            }
        }
    }
}
