#if EDITOR

#include "NodeGraph/NodeGraphPanel.h"
#include "EditorState.h"
#include "EditorConstants.h"
#include "EditorIcons.h"
#include "Assets/NodeGraphAsset.h"
#include "Assets/MaterialLite.h"
#include "NodeGraph/GraphNode.h"
#include "NodeGraph/GraphLink.h"
#include "NodeGraph/GraphPin.h"
#include "NodeGraph/GraphTypes.h"
#include "NodeGraph/GraphDomain.h"
#include "NodeGraph/GraphDomainManager.h"
#include "NodeGraph/GraphProcessor.h"
#include "NodeGraph/Nodes/ValueNodes.h"
#include "NodeGraph/Nodes/InputNodes.h"
#include "NodeGraph/Nodes/FunctionNodes.h"
#include "NodeGraph/Nodes/VariableNodes.h"
#include "NodeGraph/GraphClipboard.h"
#include "NodeGraph/GraphVariable.h"
#include "AssetManager.h"
#include "Stream.h"
#include "Engine.h"
#include "World.h"
#include "Nodes/NodeGraphPlayer.h"
#include "Log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-node-editor/imgui_node_editor.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace ed = ax::NodeEditor;

// imgui-node-editor uses a single shared ID space for nodes, pins, and links.
// Encode type in lower 2 bits to prevent collisions between ID types.
static ed::NodeId MakeNodeId(GraphNodeId id) { return ed::NodeId(((uintptr_t)id << 2) | 1); }
static ed::PinId  MakePinId(GraphPinId id)   { return ed::PinId(((uintptr_t)id << 2) | 2); }
static ed::LinkId MakeLinkId(GraphLinkId id) { return ed::LinkId(((uintptr_t)id << 2) | 3); }
static GraphNodeId FromNodeId(ed::NodeId id) { return (GraphNodeId)(id.Get() >> 2); }
static GraphPinId  FromPinId(ed::PinId id)   { return (GraphPinId)(id.Get() >> 2); }
static GraphLinkId FromLinkId(ed::LinkId id) { return (GraphLinkId)(id.Get() >> 2); }

static ed::EditorContext* sEditorContext = nullptr;
static NodeGraph* sEditedGraph = nullptr;
static Asset* sEditedOwner = nullptr;
static NodeGraphAsset* sEditedAsset = nullptr;
static GraphProcessor sProcessor;
static bool sNeedsPositionSync = false;
static bool sPreviewEnabled = false;
static ImVec2 sCanvasCenter = ImVec2(0, 0);
static GraphNodeId sPendingNodeId = 0;
static glm::vec2 sPendingNodePos = glm::vec2(0, 0);

// Function sidebar state
static const float kFunctionSidebarWidth = 200.0f;
static const float kMiddleGap = 20.0f;  // gap between input/output columns
static int32_t sSelectedGraphIndex = -1;  // -1 = main graph (Event Graph), 0..N = function index
static std::map<std::string, ed::EditorContext*> sEditorContextMap;
static bool sRenamingFunction = false;
static int32_t sRenamingIndex = -1;
static char sRenameBuffer[128] = {};
static char sNodeSearchBuffer[128] = {};
static int sSearchSelectedIndex = 0;
static int sPopupFrameCount = 0;

// Variable sidebar state
static bool sRenamingVariable = false;
static int32_t sRenamingVarIndex = -1;
static char sVarRenameBuffer[128] = {};

// Import/Export state
static bool sShowExportPopup = false;
static bool sShowImportPopup = false;
static char sFilePathBuffer[512] = {};

// Execution flash feedback
static std::unordered_map<GraphNodeId, double> sNodeFlashTimes;
static const float kFlashDuration = 0.4f;
static NodeGraph* sRuntimeGraph = nullptr; // Points to active NodeGraphPlayer's runtime graph during PIE

static ed::EditorContext* CreateNewEditorContext()
{
    ed::Config config;
    config.SettingsFile = nullptr;
    config.NavigateButtonIndex = 1;
    return ed::CreateEditor(&config);
}

static void EnsureEditorContext()
{
    if (sEditorContext == nullptr)
    {
        sEditorContext = CreateNewEditorContext();
    }
}

static ImColor GetPinColor(DatumType type)
{
    glm::vec4 c = GetDatumTypeColor(type);
    return ImColor(c.x, c.y, c.z, c.w);
}

static bool IsNodeRefType(DatumType type)
{
    return type == DatumType::Node || type == DatumType::Node3D || type == DatumType::Widget ||
           type == DatumType::Text || type == DatumType::Quad || type == DatumType::Audio3D ||
           type == DatumType::Spline3D;
}

static void DrawPinIcon(DatumType type, bool connected)
{
    ImVec2 size(12.0f, 12.0f);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(cursorPos.x + size.x * 0.5f, cursorPos.y + size.y * 0.5f);
    float radius = size.x * 0.5f;
    ImColor color = GetPinColor(type);

    if (type == DatumType::Execution)
    {
        // Triangle/arrow shape for execution pins
        ImVec2 p1(center.x - radius * 0.6f, center.y - radius * 0.8f);
        ImVec2 p2(center.x + radius * 0.8f, center.y);
        ImVec2 p3(center.x - radius * 0.6f, center.y + radius * 0.8f);
        if (connected)
            drawList->AddTriangleFilled(p1, p2, p3, color);
        else
            drawList->AddTriangle(p1, p2, p3, color, 2.0f);
    }
    else if (IsNodeRefType(type))
    {
        // Diamond shape for node-reference pins
        ImVec2 top(center.x, center.y - radius);
        ImVec2 right(center.x + radius, center.y);
        ImVec2 bottom(center.x, center.y + radius);
        ImVec2 left(center.x - radius, center.y);
        if (connected)
            drawList->AddQuadFilled(top, right, bottom, left, color);
        else
            drawList->AddQuad(top, right, bottom, left, color, 2.0f);
    }
    else if (type == DatumType::Scene || type == DatumType::Asset)
    {
        // Square shape for asset/scene pins
        ImVec2 tl(center.x - radius * 0.7f, center.y - radius * 0.7f);
        ImVec2 br(center.x + radius * 0.7f, center.y + radius * 0.7f);
        if (connected)
            drawList->AddRectFilled(tl, br, color);
        else
            drawList->AddRect(tl, br, color, 0.0f, 0, 2.0f);
    }
    else if (type == DatumType::PointCloud)
    {
        // Dot cluster shape for point cloud pins
        float dotR = radius * 0.3f;
        if (connected)
        {
            drawList->AddCircleFilled(ImVec2(center.x - radius * 0.35f, center.y - radius * 0.35f), dotR, color);
            drawList->AddCircleFilled(ImVec2(center.x + radius * 0.35f, center.y - radius * 0.35f), dotR, color);
            drawList->AddCircleFilled(ImVec2(center.x, center.y + radius * 0.35f), dotR, color);
        }
        else
        {
            drawList->AddCircle(ImVec2(center.x - radius * 0.35f, center.y - radius * 0.35f), dotR, color, 8, 1.5f);
            drawList->AddCircle(ImVec2(center.x + radius * 0.35f, center.y - radius * 0.35f), dotR, color, 8, 1.5f);
            drawList->AddCircle(ImVec2(center.x, center.y + radius * 0.35f), dotR, color, 8, 1.5f);
        }
    }
    else
    {
        // Circle for data pins (existing behavior)
        if (connected)
            drawList->AddCircleFilled(center, radius, color);
        else
            drawList->AddCircle(center, radius, color, 12, 2.0f);
    }

    ImGui::Dummy(size);
}

static bool IsPinConnected(NodeGraph& graph, GraphPinId pinId)
{
    const std::vector<GraphLink>& links = graph.GetLinks();
    for (uint32_t i = 0; i < links.size(); ++i)
    {
        if (links[i].mOutputPinId == pinId || links[i].mInputPinId == pinId)
        {
            return true;
        }
    }
    return false;
}

// Deferred pin enum popup state
static GraphPin* sPinEnumTarget = nullptr;
static GraphNode* sPinEnumOwnerNode = nullptr;
static uint32_t sPinEnumPinIndex = 0;
static bool sPinEnumJustOpened = false;

static void DrawInputPinWidget(GraphPin& pin, GraphNode* ownerNode = nullptr, uint32_t pinIndex = 0)
{
    switch (pin.mDataType)
    {
    case DatumType::Float:
    {
        float f = pin.mDefaultValue.GetFloat();
        ImGui::SetNextItemWidth(60.0f);
        if (ImGui::DragFloat("##v", &f, 0.01f))
        {
            pin.mDefaultValue = Datum(f);
            pin.mValue = pin.mDefaultValue;
        }
        break;
    }
    case DatumType::Integer:
    {
        // Check if the owning node provides enum options for this pin
        std::vector<PinEnumOption> enumOptions;
        if (ownerNode != nullptr && ownerNode->GetPinEnumOptions(pinIndex, enumOptions) && !enumOptions.empty())
        {
            int32_t currentVal = pin.mDefaultValue.GetInteger();

            // Find the current selection's display name
            const char* currentName = "?";
            for (int i = 0; i < (int)enumOptions.size(); ++i)
            {
                if (enumOptions[i].mValue == currentVal)
                {
                    currentName = enumOptions[i].mName;
                    break;
                }
            }

            // Render a button inside the node; the actual popup is deferred
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::Button(currentName, ImVec2(100.0f, 0.0f)))
            {
                sPinEnumTarget = &pin;
                sPinEnumOwnerNode = ownerNode;
                sPinEnumPinIndex = pinIndex;
                sPinEnumJustOpened = true;
            }
        }
        else
        {
            int32_t v = pin.mDefaultValue.GetInteger();
            ImGui::SetNextItemWidth(60.0f);
            if (ImGui::DragInt("##v", &v))
            {
                pin.mDefaultValue = Datum(v);
                pin.mValue = pin.mDefaultValue;
            }
        }
        break;
    }
    case DatumType::Bool:
    {
        bool b = pin.mDefaultValue.GetBool();
        if (ImGui::Checkbox("##v", &b))
        {
            pin.mDefaultValue = Datum(b);
            pin.mValue = pin.mDefaultValue;
        }
        break;
    }
    case DatumType::Vector2D:
    {
        glm::vec2 v = pin.mDefaultValue.GetVector2D();
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::DragFloat2("##v", &v.x, 0.01f))
        {
            pin.mDefaultValue = Datum(v);
            pin.mValue = pin.mDefaultValue;
        }
        break;
    }
    case DatumType::Vector:
    {
        glm::vec3 v = pin.mDefaultValue.GetVector();
        ImGui::SetNextItemWidth(150.0f);
        if (ImGui::DragFloat3("##v", &v.x, 0.01f))
        {
            pin.mDefaultValue = Datum(v);
            pin.mValue = pin.mDefaultValue;
        }
        break;
    }
    case DatumType::Color:
    {
        glm::vec4 c = pin.mDefaultValue.GetColor();
        ImGui::ColorEdit4("##v", &c.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        if (memcmp(&c, &pin.mDefaultValue.GetColor(), sizeof(glm::vec4)) != 0)
        {
            pin.mDefaultValue = Datum(c);
            pin.mValue = pin.mDefaultValue;
        }
        break;
    }
    case DatumType::String:
    {
        static char buf[256];
        std::string s = pin.mDefaultValue.GetString();
        strncpy(buf, s.c_str(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        ImGui::SetNextItemWidth(120.0f);
        if (ImGui::InputText("##v", buf, sizeof(buf)))
        {
            pin.mDefaultValue = Datum(std::string(buf));
            pin.mValue = pin.mDefaultValue;
        }
        break;
    }
    case DatumType::Execution:
        // No widget for execution pins
        break;
    case DatumType::Node:
    case DatumType::Node3D:
    case DatumType::Widget:
    case DatumType::Text:
    case DatumType::Quad:
    case DatumType::Audio3D:
        ImGui::TextDisabled("%s", GetDatumTypeName(pin.mDataType));
        break;
    case DatumType::Scene:
    case DatumType::Asset:
        ImGui::TextDisabled("%s", GetDatumTypeName(pin.mDataType));
        break;
    default:
        break;
    }
}

static void DrawOutputPinValue(const GraphPin& pin)
{
    switch (pin.mDataType)
    {
    case DatumType::Float:
        ImGui::Text("%.3f", pin.mValue.GetFloat());
        break;
    case DatumType::Integer:
        ImGui::Text("%d", pin.mValue.GetInteger());
        break;
    case DatumType::Bool:
        ImGui::Text("%s", pin.mValue.GetBool() ? "true" : "false");
        break;
    case DatumType::Vector2D:
    {
        glm::vec2 v = pin.mValue.GetVector2D();
        ImGui::Text("(%.1f, %.1f)", v.x, v.y);
        break;
    }
    case DatumType::Vector:
    {
        glm::vec3 v = pin.mValue.GetVector();
        ImGui::Text("(%.1f, %.1f, %.1f)", v.x, v.y, v.z);
        break;
    }
    case DatumType::Color:
    {
        glm::vec4 c = pin.mValue.GetColor();
        ImVec4 col(c.x, c.y, c.z, c.w);
        ImGui::ColorButton("##v", col, ImGuiColorEditFlags_NoTooltip, ImVec2(12, 12));
        break;
    }
    case DatumType::String:
        ImGui::Text("%s", pin.mValue.GetString().c_str());
        break;
    case DatumType::Execution:
        break;
    case DatumType::Node:
    case DatumType::Node3D:
    case DatumType::Widget:
    case DatumType::Text:
    case DatumType::Quad:
    case DatumType::Audio3D:
        ImGui::TextDisabled("%s", GetDatumTypeName(pin.mDataType));
        break;
    case DatumType::Scene:
    case DatumType::Asset:
        ImGui::TextDisabled("%s", GetDatumTypeName(pin.mDataType));
        break;
    default:
        break;
    }
}

static void BeginColumns()  { ImGui::BeginGroup(); }
static void NextColumn()    { ImGui::EndGroup(); ImGui::SameLine(0.0f, kMiddleGap); ImGui::BeginGroup(); }
static void EndColumns()    { ImGui::EndGroup(); }

static NodeGraph* FindRuntimeGraph()
{
    if (sEditedAsset == nullptr)
        return nullptr;

    World* world = GetWorld(0);
    if (world == nullptr)
        return nullptr;

    std::vector<Node*> nodes = world->GatherNodes();
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        NodeGraphPlayer* player = nodes[i]->As<NodeGraphPlayer>();
        if (player != nullptr &&
            player->GetNodeGraphAsset() == sEditedAsset &&
            player->IsPlaying())
        {
            return player->GetRuntimeGraph();
        }
    }

    return nullptr;
}

static void RecordExecutionFlash(NodeGraph& graph)
{
    double currentTime = ImGui::GetTime();

    // Erase stale entries
    for (auto it = sNodeFlashTimes.begin(); it != sNodeFlashTimes.end(); )
    {
        if (currentTime - it->second > kFlashDuration)
            it = sNodeFlashTimes.erase(it);
        else
            ++it;
    }

    if (graph.mExecutedPinIds.empty())
        return;

    // Record flash for nodes that own any triggered execution pin
    const std::vector<GraphNode*>& nodes = graph.GetNodes();
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        GraphNode* node = nodes[i];
        for (uint32_t j = 0; j < node->GetNumOutputPins(); ++j)
        {
            const GraphPin& pin = node->GetOutputPins()[j];
            if (pin.mDataType == DatumType::Execution &&
                graph.mExecutedPinIds.count(pin.mId))
            {
                sNodeFlashTimes[node->GetId()] = currentTime;
                break;
            }
        }
    }

    // Clear so it re-accumulates next frame
    graph.mExecutedPinIds.clear();
}

static void DrawNodes(NodeGraph& graph)
{
    const std::vector<GraphNode*>& nodes = graph.GetNodes();

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        GraphNode* node = nodes[i];
        glm::vec4 color = node->GetNodeColor();
        ImColor headerColor(color.x, color.y, color.z, color.w);

        // Flash node border if recently executed
        bool flashing = false;
        auto flashIt = sNodeFlashTimes.find(node->GetId());
        if (flashIt != sNodeFlashTimes.end())
        {
            float elapsed = (float)(ImGui::GetTime() - flashIt->second);
            float alpha = 1.0f - (elapsed / kFlashDuration);
            if (alpha > 0.0f)
            {
                ed::PushStyleColor(ed::StyleColor_NodeBorder, ImVec4(1.0f, 0.7f, 0.2f, alpha));
                flashing = true;
            }
        }

        ed::BeginNode(MakeNodeId(node->GetId()));

        // Title
        ImGui::Text("%s", node->GetNodeTypeName());

        // Editable name label for input nodes
        if (node->IsInputNode())
        {
            const std::string& inputName = node->GetInputName();
            char nameBuf[128];
            strncpy(nameBuf, inputName.c_str(), sizeof(nameBuf) - 1);
            nameBuf[sizeof(nameBuf) - 1] = '\0';
            ImGui::SetNextItemWidth(120.0f);
            ImGui::PushID(node->GetId() + 10000);
            if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
            {
                node->SetInputName(nameBuf);
            }
            ImGui::PopID();
        }

        ImGui::Spacing();

        bool isSinkNode = (node->GetNumOutputPins() == 0);
        bool hasInputs = (node->GetNumInputPins() > 0) ||
                         (node->GetType() == FunctionOutputNode::GetStaticType());
        bool hasOutputs = (node->GetNumOutputPins() > 0);

        BeginColumns();

        // LEFT COLUMN: input pins
        if (hasInputs)
        {
            for (uint32_t j = 0; j < node->GetNumInputPins(); ++j)
            {
                GraphPin& pin = node->GetInputPins()[j];
                bool connected = IsPinConnected(graph, pin.mId);

                ed::BeginPin(MakePinId(pin.mId), ed::PinKind::Input);
                DrawPinIcon(pin.mDataType, connected);
                ed::EndPin();

                ImGui::SameLine();
                ImGui::Text("%s", pin.mName.c_str());

                if (!connected)
                {
                    ImGui::SameLine();
                    ImGui::PushID(pin.mId);
                    DrawInputPinWidget(pin, node, j);
                    ImGui::PopID();
                }
                else if (isSinkNode)
                {
                    // Show the evaluated value on sink nodes (Viewer, Material Output)
                    ImGui::SameLine();
                    ImGui::PushID(pin.mId);
                    DrawOutputPinValue(pin);
                    ImGui::PopID();
                }
            }

            // FunctionOutputNode: add dynamic pin button
            if (node->GetType() == FunctionOutputNode::GetStaticType())
            {
                ImGui::PushID(node->GetId() + 20000);
                if (ImGui::SmallButton("+ Output"))
                {
                    ImGui::OpenPopup("AddOutputPin");
                }

                if (ImGui::BeginPopup("AddOutputPin"))
                {
                    FunctionOutputNode* funcOutput = static_cast<FunctionOutputNode*>(node);
                    if (ImGui::MenuItem("Float"))   { funcOutput->AddOutputField("Float", DatumType::Float); }
                    if (ImGui::MenuItem("Integer")) { funcOutput->AddOutputField("Integer", DatumType::Integer); }
                    if (ImGui::MenuItem("Bool"))    { funcOutput->AddOutputField("Bool", DatumType::Bool); }
                    if (ImGui::MenuItem("String"))  { funcOutput->AddOutputField("String", DatumType::String); }
                    if (ImGui::MenuItem("Vector"))  { funcOutput->AddOutputField("Vector", DatumType::Vector); }
                    if (ImGui::MenuItem("Color"))   { funcOutput->AddOutputField("Color", DatumType::Color); }
                    ImGui::EndPopup();
                }

                // Right-click on input pins to remove them
                for (uint32_t j = 0; j < node->GetNumInputPins(); ++j)
                {
                    char popupId[32];
                    snprintf(popupId, sizeof(popupId), "RemovePin_%u", j);

                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                    {
                        // Check if mouse is over this specific pin area
                    }
                }
                ImGui::PopID();
            }
        }
        else if (hasOutputs)
        {
            // Source-only nodes: empty left column with minimum width
            ImGui::Dummy(ImVec2(1.0f, 0.0f));
        }

        NextColumn();

        // RIGHT COLUMN: output pins
        for (uint32_t j = 0; j < node->GetNumOutputPins(); ++j)
        {
            const GraphPin& pin = node->GetOutputPins()[j];
            bool connected = IsPinConnected(graph, pin.mId);

            // Render pin icon first to establish the line, then label via SameLine.
            // imgui-node-editor positions the link endpoint at the pin icon
            // regardless of draw order within the BeginPin/EndPin block.
            ed::BeginPin(MakePinId(pin.mId), ed::PinKind::Output);
            if (pin.mDataType == DatumType::Execution)
            {
                ImGui::Text("%s", pin.mName.c_str());
                ImGui::SameLine();
            }
            else
            {
                ImGui::PushID(pin.mId);
                DrawOutputPinValue(pin);
                ImGui::PopID();
                ImGui::SameLine();
            }
            DrawPinIcon(pin.mDataType, connected);
            ed::EndPin();
        }

        EndColumns();

        ed::EndNode();

        if (flashing)
        {
            ed::PopStyleColor();
        }
    }
}

static void DrawLinks(NodeGraph& graph)
{
    const std::vector<GraphLink>& links = graph.GetLinks();

    for (uint32_t i = 0; i < links.size(); ++i)
    {
        GraphPin* outputPin = graph.FindPin(links[i].mOutputPinId);
        ImColor color(0.7f, 0.7f, 0.7f, 1.0f);
        if (outputPin != nullptr)
        {
            color = GetPinColor(outputPin->mDataType);
        }

        ed::Link(
            MakeLinkId(links[i].mId),
            MakePinId(links[i].mOutputPinId),
            MakePinId(links[i].mInputPinId),
            color,
            2.0f);

        // Animate flow on active execution links
        if (outputPin != nullptr && outputPin->mDataType == DatumType::Execution)
        {
            // Check ephemeral flag (preview mode) or persistent set (PIE)
            bool triggered = outputPin->mExecutionTriggered;
            if (!triggered && sRuntimeGraph != nullptr)
            {
                triggered = sRuntimeGraph->mExecutedPinIds.count(links[i].mOutputPinId) > 0;
            }
            if (triggered)
            {
                ed::Flow(MakeLinkId(links[i].mId));
            }
        }
    }
}

static void HandleCreation(NodeGraph& graph)
{
    if (ed::BeginCreate())
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            if (inputPinId && outputPinId)
            {
                GraphPinId pinId1 = FromPinId(inputPinId);
                GraphPinId pinId2 = FromPinId(outputPinId);
                GraphPin* pin1 = graph.FindPin(pinId1);
                GraphPin* pin2 = graph.FindPin(pinId2);

                if (pin1 != nullptr && pin2 != nullptr)
                {
                    // Ensure we have output -> input direction
                    GraphPinId outId = pinId2;
                    GraphPinId inId = pinId1;

                    if (pin1->mDirection == GraphPinDirection::Output && pin2->mDirection == GraphPinDirection::Input)
                    {
                        outId = pinId1;
                        inId = pinId2;
                    }
                    else if (pin1->mDirection == GraphPinDirection::Input && pin2->mDirection == GraphPinDirection::Output)
                    {
                        outId = pinId2;
                        inId = pinId1;
                    }

                    GraphPin* outPin = graph.FindPin(outId);
                    GraphPin* inPin = graph.FindPin(inId);

                    if (outPin != nullptr && inPin != nullptr &&
                        outPin->mDirection == GraphPinDirection::Output &&
                        inPin->mDirection == GraphPinDirection::Input)
                    {
                        if (AreGraphPinTypesCompatible(outPin->mDataType, inPin->mDataType))
                        {
                            GraphNode* outNode = graph.FindPinOwner(outId);
                            GraphNode* inNode = graph.FindPinOwner(inId);

                            if (outNode && inNode && outNode->GetId() != inNode->GetId() &&
                                !graph.WouldCreateCycle(outNode->GetId(), inNode->GetId()))
                            {
                                if (ed::AcceptNewItem())
                                {
                                    graph.AddLink(outId, inId);
                                }
                            }
                            else
                            {
                                ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                            }
                        }
                        else
                        {
                            ed::RejectNewItem(ImColor(255, 128, 0), 2.0f);
                        }
                    }
                    else
                    {
                        ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                    }
                }
            }
        }
    }
    ed::EndCreate();
}

static void HandleDeletion(NodeGraph& graph)
{
    if (ed::BeginDelete())
    {
        ed::LinkId linkId;
        while (ed::QueryDeletedLink(&linkId))
        {
            if (ed::AcceptDeletedItem())
            {
                graph.RemoveLink(FromLinkId(linkId));
            }
        }

        ed::NodeId nodeId;
        while (ed::QueryDeletedNode(&nodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                graph.RemoveNode(FromNodeId(nodeId));
            }
        }
    }
    ed::EndDelete();
}

static void AddNodeAtCenter(NodeGraph& graph, TypeId nodeType)
{
    GraphNode* node = graph.AddNode(nodeType);
    if (node != nullptr)
    {
        sPendingNodeId = node->GetId();
        sPendingNodePos = glm::vec2(sCanvasCenter.x, sCanvasCenter.y);
    }
}

static FunctionCallNode* AddFunctionCallNode(NodeGraph& graph, const std::string& functionName)
{
    if (sEditedAsset == nullptr)
        return nullptr;

    GraphNode* node = graph.AddNode(FunctionCallNode::GetStaticType());
    if (node == nullptr)
        return nullptr;

    FunctionCallNode* callNode = static_cast<FunctionCallNode*>(node);
    callNode->SetFunctionName(functionName);
    callNode->RebuildPinsFromFunction(sEditedAsset);

    sPendingNodeId = node->GetId();
    sPendingNodePos = glm::vec2(sCanvasCenter.x, sCanvasCenter.y);

    return callNode;
}

static GetVariableNode* AddGetVariableNode(NodeGraph& graph, const std::string& varName)
{
    if (sEditedAsset == nullptr)
        return nullptr;

    GraphNode* node = graph.AddNode(GetVariableNode::GetStaticType());
    if (node == nullptr)
        return nullptr;

    GetVariableNode* varNode = static_cast<GetVariableNode*>(node);
    varNode->SetVariableName(varName);
    varNode->RebuildPinFromVariable(sEditedAsset);

    sPendingNodeId = node->GetId();
    sPendingNodePos = glm::vec2(sCanvasCenter.x, sCanvasCenter.y);

    return varNode;
}

static SetVariableNode* AddSetVariableNode(NodeGraph& graph, const std::string& varName)
{
    if (sEditedAsset == nullptr)
        return nullptr;

    GraphNode* node = graph.AddNode(SetVariableNode::GetStaticType());
    if (node == nullptr)
        return nullptr;

    SetVariableNode* varNode = static_cast<SetVariableNode*>(node);
    varNode->SetVariableName(varName);
    varNode->RebuildPinsFromVariable(sEditedAsset);

    sPendingNodeId = node->GetId();
    sPendingNodePos = glm::vec2(sCanvasCenter.x, sCanvasCenter.y);

    return varNode;
}

// Mapping from DatumType to the appropriate InputNode type
static TypeId GetInputNodeTypeForDatum(DatumType type)
{
    switch (type)
    {
    case DatumType::Float:    return FloatInputNode::GetStaticType();
    case DatumType::Integer:  return IntInputNode::GetStaticType();
    case DatumType::Bool:     return BoolInputNode::GetStaticType();
    case DatumType::String:   return StringInputNode::GetStaticType();
    case DatumType::Vector:   return VectorInputNode::GetStaticType();
    case DatumType::Color:    return ColorInputNode::GetStaticType();
    case DatumType::Byte:     return ByteInputNode::GetStaticType();
    case DatumType::Asset:    return AssetInputNode::GetStaticType();
    default:                  return FloatInputNode::GetStaticType();
    }
}

static void CreateFunctionFromSelection(NodeGraph& graph)
{
    if (sEditedAsset == nullptr)
        return;

    // 1. Collect selected node IDs
    int selectedCount = ed::GetSelectedObjectCount();
    if (selectedCount <= 0)
        return;

    std::vector<ed::NodeId> selectedEdIds(selectedCount);
    int nodeCount = ed::GetSelectedNodes(selectedEdIds.data(), selectedCount);
    if (nodeCount <= 0)
        return;

    std::unordered_set<GraphNodeId> selectedIds;
    for (int i = 0; i < nodeCount; ++i)
    {
        selectedIds.insert(FromNodeId(selectedEdIds[i]));
    }

    // 2. Analyze boundary connections
    struct IncomingConnection
    {
        GraphPinId externalOutputPinId;    // pin in unselected node
        GraphPinId internalInputPinId;     // pin in selected node
        GraphNodeId internalNodeId;
        DatumType type;
    };
    struct OutgoingConnection
    {
        GraphPinId internalOutputPinId;    // pin in selected node
        GraphPinId externalInputPinId;     // pin in unselected node
        GraphNodeId externalNodeId;
        DatumType type;
    };

    std::vector<IncomingConnection> incoming;
    std::vector<OutgoingConnection> outgoing;
    std::vector<GraphLink> internalLinks;

    for (const GraphLink& link : graph.GetLinks())
    {
        bool srcSelected = selectedIds.count(link.mOutputNodeId) > 0;
        bool dstSelected = selectedIds.count(link.mInputNodeId) > 0;

        if (srcSelected && dstSelected)
        {
            internalLinks.push_back(link);
        }
        else if (!srcSelected && dstSelected)
        {
            GraphPin* pin = graph.FindPin(link.mInputPinId);
            DatumType type = pin ? pin->mDataType : DatumType::Float;
            incoming.push_back({ link.mOutputPinId, link.mInputPinId, link.mInputNodeId, type });
        }
        else if (srcSelected && !dstSelected)
        {
            GraphPin* pin = graph.FindPin(link.mOutputPinId);
            DatumType type = pin ? pin->mDataType : DatumType::Float;
            outgoing.push_back({ link.mOutputPinId, link.mInputPinId, link.mOutputNodeId, type });
        }
    }

    // 3. Create new function graph
    static int sFunctionCounter = 1;
    char funcName[64];
    snprintf(funcName, sizeof(funcName), "Function_%d", sFunctionCounter++);

    NodeGraph* funcGraph = sEditedAsset->AddFunctionGraph(funcName);
    if (funcGraph == nullptr)
        return;

    // 4. Copy selected nodes to function graph (with ID remapping)
    std::unordered_map<GraphNodeId, GraphNodeId> nodeIdMap;
    std::unordered_map<GraphPinId, GraphPinId> pinIdMap;

    // Compute centroid of selection
    glm::vec2 centroid(0.0f);
    int centroidCount = 0;

    for (GraphNodeId selId : selectedIds)
    {
        GraphNode* srcNode = graph.FindNode(selId);
        if (srcNode == nullptr)
            continue;

        centroid += srcNode->GetEditorPosition();
        centroidCount++;

        GraphNode* newNode = funcGraph->AddNode(srcNode->GetType());
        if (newNode == nullptr)
            continue;

        newNode->SetEditorPosition(srcNode->GetEditorPosition());

        // Copy pin values
        for (uint32_t j = 0; j < srcNode->GetNumInputPins() && j < newNode->GetNumInputPins(); ++j)
        {
            newNode->GetInputPins()[j].mDefaultValue = srcNode->GetInputPins()[j].mDefaultValue;
            newNode->GetInputPins()[j].mValue = srcNode->GetInputPins()[j].mValue;
            pinIdMap[srcNode->GetInputPins()[j].mId] = newNode->GetInputPins()[j].mId;
        }
        for (uint32_t j = 0; j < srcNode->GetNumOutputPins() && j < newNode->GetNumOutputPins(); ++j)
        {
            pinIdMap[srcNode->GetOutputPins()[j].mId] = newNode->GetOutputPins()[j].mId;
        }

        if (srcNode->IsInputNode())
        {
            newNode->SetInputName(srcNode->GetInputName());
        }

        newNode->CopyCustomData(srcNode);
        nodeIdMap[selId] = newNode->GetId();
    }

    if (centroidCount > 0)
    {
        centroid /= (float)centroidCount;
    }

    // 5. Copy internal links with remapped IDs
    for (const GraphLink& link : internalLinks)
    {
        auto outIt = pinIdMap.find(link.mOutputPinId);
        auto inIt = pinIdMap.find(link.mInputPinId);
        if (outIt != pinIdMap.end() && inIt != pinIdMap.end())
        {
            funcGraph->AddLink(outIt->second, inIt->second);
        }
    }

    // 6. Add InputNodes for each incoming connection
    std::unordered_map<GraphPinId, GraphPinId> inputNodeOutputPinMap;  // external output pin → new InputNode output pin
    float inputYOffset = 0.0f;

    for (const IncomingConnection& conn : incoming)
    {
        // Check if we already created an InputNode for this external pin
        if (inputNodeOutputPinMap.count(conn.externalOutputPinId) > 0)
        {
            // Reuse: link existing InputNode output to internal target
            auto targetIt = pinIdMap.find(conn.internalInputPinId);
            if (targetIt != pinIdMap.end())
            {
                funcGraph->AddLink(inputNodeOutputPinMap[conn.externalOutputPinId], targetIt->second);
            }
            continue;
        }

        TypeId inputType = GetInputNodeTypeForDatum(conn.type);
        GraphNode* inputNode = funcGraph->AddNode(inputType);
        if (inputNode == nullptr)
            continue;

        // Position to the left of selection
        inputNode->SetEditorPosition(glm::vec2(centroid.x - 300.0f, centroid.y + inputYOffset));
        inputYOffset += 80.0f;

        // Name the input after the pin
        GraphPin* srcPin = graph.FindPin(conn.internalInputPinId);
        if (srcPin != nullptr)
        {
            inputNode->SetInputName(srcPin->mName);
        }

        // Link InputNode output → internal target
        if (inputNode->GetNumOutputPins() > 0)
        {
            auto targetIt = pinIdMap.find(conn.internalInputPinId);
            if (targetIt != pinIdMap.end())
            {
                funcGraph->AddLink(inputNode->GetOutputPinId(0), targetIt->second);
            }
            inputNodeOutputPinMap[conn.externalOutputPinId] = inputNode->GetOutputPinId(0);
        }
    }

    // 7. Add FunctionOutputNode with dynamic pins for each outgoing connection
    FunctionOutputNode* funcOutputNode = nullptr;
    if (!outgoing.empty())
    {
        GraphNode* outputNodeBase = funcGraph->AddNode(FunctionOutputNode::GetStaticType());
        funcOutputNode = static_cast<FunctionOutputNode*>(outputNodeBase);
        funcOutputNode->SetEditorPosition(glm::vec2(centroid.x + 300.0f, centroid.y));

        for (uint32_t i = 0; i < outgoing.size(); ++i)
        {
            GraphPin* srcPin = graph.FindPin(outgoing[i].internalOutputPinId);
            std::string pinName = srcPin ? srcPin->mName : "Out";
            funcOutputNode->AddOutputField(pinName, outgoing[i].type);

            // Link internal source → FunctionOutputNode input
            auto srcIt = pinIdMap.find(outgoing[i].internalOutputPinId);
            if (srcIt != pinIdMap.end() && funcOutputNode->GetNumInputPins() > i)
            {
                funcGraph->AddLink(srcIt->second, funcOutputNode->GetInputPinId(i));
            }
        }
    }

    // 8. Remove selected nodes from main graph
    // First remove all links connected to selected nodes
    for (int32_t i = (int32_t)graph.GetLinks().size() - 1; i >= 0; --i)
    {
        const GraphLink& link = graph.GetLinks()[i];
        if (selectedIds.count(link.mOutputNodeId) > 0 || selectedIds.count(link.mInputNodeId) > 0)
        {
            graph.RemoveLink(link.mId);
        }
    }
    // Then remove the nodes
    for (GraphNodeId selId : selectedIds)
    {
        graph.RemoveNode(selId);
    }

    // 9. Insert FunctionCallNode at centroid
    GraphNode* callNodeBase = graph.AddNode(FunctionCallNode::GetStaticType());
    FunctionCallNode* callNode = static_cast<FunctionCallNode*>(callNodeBase);
    callNode->SetFunctionName(funcName);
    callNode->RebuildPinsFromFunction(sEditedAsset);
    callNode->SetEditorPosition(centroid);
    ed::SetNodePosition(MakeNodeId(callNode->GetId()), ImVec2(centroid.x, centroid.y));

    // 10. Reconnect external links to FunctionCallNode's pins
    // Reconnect incoming: external output pin → FunctionCallNode input pin
    uint32_t inputPinIdx = 0;
    std::unordered_set<GraphPinId> reconnectedExternalPins;
    for (const IncomingConnection& conn : incoming)
    {
        if (reconnectedExternalPins.count(conn.externalOutputPinId) > 0)
            continue;

        if (inputPinIdx < callNode->GetNumInputPins())
        {
            graph.AddLink(conn.externalOutputPinId, callNode->GetInputPinId(inputPinIdx));
            reconnectedExternalPins.insert(conn.externalOutputPinId);
        }
        inputPinIdx++;
    }

    // Reconnect outgoing: FunctionCallNode output pin → external input pin
    for (uint32_t i = 0; i < outgoing.size() && i < callNode->GetNumOutputPins(); ++i)
    {
        graph.AddLink(callNode->GetOutputPinId(i), outgoing[i].externalInputPinId);
    }

    ed::ClearSelection();
}

static void DrawContextMenu(NodeGraph& graph)
{
    static ImVec2 sContextCanvasPos;
    static ed::NodeId sContextNodeId;
    static ed::LinkId sContextLinkId;

    // Use imgui-node-editor's built-in context menu detection.
    // OpenPopup and BeginPopup must share the same Suspend block so they
    // operate in the same ImGui window context.
    ed::Suspend();
    if (ed::ShowNodeContextMenu(&sContextNodeId))
    {
        sContextCanvasPos = ed::ScreenToCanvas(ImGui::GetMousePos());
        ImGui::OpenPopup("Node Context");
    }
    else if (ed::ShowLinkContextMenu(&sContextLinkId))
    {
        sContextCanvasPos = ed::ScreenToCanvas(ImGui::GetMousePos());
        ImGui::OpenPopup("Link Context");
    }
    else if (ed::ShowBackgroundContextMenu())
    {
        sContextCanvasPos = ed::ScreenToCanvas(ImGui::GetMousePos());
        ImGui::OpenPopup("Create Node");
    }
    // Shift+A shortcut to open Create Node menu (Blender-style)
    else if (ImGui::IsKeyPressed(ImGuiKey_A) && ImGui::GetIO().KeyShift && !ImGui::IsAnyItemActive())
    {
        sContextCanvasPos = ed::ScreenToCanvas(ImGui::GetMousePos());
        ImGui::OpenPopup("Create Node");
    }

    if (ImGui::BeginPopup("Create Node"))
    {
        // Clear search buffer when popup first opens
        if (ImGui::IsWindowAppearing())
        {
            sNodeSearchBuffer[0] = '\0';
            sSearchSelectedIndex = 0;
            sPopupFrameCount = 0;
            ImGui::SetKeyboardFocusHere();
        }
        sPopupFrameCount++;

        bool searchChanged = ImGui::InputTextWithHint("##NodeSearch", "Search nodes...", sNodeSearchBuffer, IM_ARRAYSIZE(sNodeSearchBuffer));
        if (searchChanged)
        {
            sSearchSelectedIndex = 0;
        }

        // When the search bar is empty and the mouse has moved away from it,
        // release active state so BeginMenu category items can highlight properly.
        // The InputText holding active state suppresses hover on other items
        // in the same popup window.
        // Wait a few frames so SetKeyboardFocusHere() can take effect (it's
        // queued and applies on the next frame's widget).
        if (sNodeSearchBuffer[0] == '\0' && ImGui::IsItemActive() && !ImGui::IsItemHovered() && sPopupFrameCount > 3)
        {
            ImGui::ClearActiveID();
        }

        ImGui::Separator();

        GraphDomainManager* domainMgr = GraphDomainManager::Get();
        GraphDomain* domain = nullptr;
        if (domainMgr != nullptr)
        {
            domain = domainMgr->GetDomain(graph.GetDomainName().c_str());
        }

        bool hasSearchFilter = sNodeSearchBuffer[0] != '\0';

        if (domain != nullptr)
        {
            const std::vector<GraphNodeTypeInfo>& nodeTypes = domain->GetNodeTypes();

            if (hasSearchFilter)
            {
                // --- Flat filtered list with keyboard navigation ---
                std::string searchLower = sNodeSearchBuffer;
                std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(),
                    [](unsigned char c) { return (char)std::tolower(c); });

                // Collect matching results: index into nodeTypes, or ~0 + func index for functions
                struct SearchResult
                {
                    uint32_t nodeTypeIndex;  // index into nodeTypes, or UINT32_MAX for function
                    uint32_t funcIndex;      // only valid when nodeTypeIndex == UINT32_MAX
                };
                std::vector<SearchResult> results;

                for (uint32_t i = 0; i < nodeTypes.size(); ++i)
                {
                    std::string nameLower = nodeTypes[i].mTypeName;
                    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                        [](unsigned char c) { return (char)std::tolower(c); });

                    std::string catLower = nodeTypes[i].mCategory;
                    std::transform(catLower.begin(), catLower.end(), catLower.begin(),
                        [](unsigned char c) { return (char)std::tolower(c); });

                    if (nameLower.find(searchLower) != std::string::npos ||
                        catLower.find(searchLower) != std::string::npos)
                    {
                        results.push_back({ i, 0 });
                    }
                }

                if (sEditedAsset != nullptr)
                {
                    for (uint32_t i = 0; i < sEditedAsset->GetNumFunctionGraphs(); ++i)
                    {
                        NodeGraph* fg = sEditedAsset->GetFunctionGraph(i);
                        if (fg != nullptr)
                        {
                            std::string fnameLower = fg->GetGraphName();
                            std::transform(fnameLower.begin(), fnameLower.end(), fnameLower.begin(),
                                [](unsigned char c) { return (char)std::tolower(c); });

                            if (fnameLower.find(searchLower) != std::string::npos)
                            {
                                results.push_back({ UINT32_MAX, i });
                            }
                        }
                    }

                    // Search variables (Get/Set)
                    for (uint32_t i = 0; i < sEditedAsset->GetNumVariables(); ++i)
                    {
                        const GraphVariable* var = sEditedAsset->GetVariable(i);
                        if (var == nullptr)
                            continue;

                        std::string vnameLower = var->mName;
                        std::transform(vnameLower.begin(), vnameLower.end(), vnameLower.begin(),
                            [](unsigned char c) { return (char)std::tolower(c); });

                        // Also check against "get <name>" and "set <name>"
                        std::string getLower = "get " + vnameLower;
                        std::string setLower = "set " + vnameLower;

                        if (vnameLower.find(searchLower) != std::string::npos ||
                            getLower.find(searchLower) != std::string::npos ||
                            setLower.find(searchLower) != std::string::npos)
                        {
                            // Use UINT32_MAX-1 tag for get variable, UINT32_MAX-2 for set variable
                            results.push_back({ UINT32_MAX - 1, i });
                            results.push_back({ UINT32_MAX - 2, i });
                        }
                    }
                }

                // Keyboard navigation (Up/Down/Enter)
                int resultCount = (int)results.size();
                if (resultCount > 0)
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
                    {
                        sSearchSelectedIndex = (sSearchSelectedIndex + 1) % resultCount;
                    }
                    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
                    {
                        sSearchSelectedIndex = (sSearchSelectedIndex - 1 + resultCount) % resultCount;
                    }
                    if (sSearchSelectedIndex >= resultCount)
                    {
                        sSearchSelectedIndex = 0;
                    }
                }

                // Render results
                bool enterPressed = ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter);
                for (int r = 0; r < resultCount; ++r)
                {
                    bool isSelected = (r == sSearchSelectedIndex);
                    const SearchResult& sr = results[r];

                    if (sr.nodeTypeIndex != UINT32_MAX)
                    {
                        // Domain node
                        ImGui::PushID((int)sr.nodeTypeIndex);
                        if (ImGui::Selectable(nodeTypes[sr.nodeTypeIndex].mTypeName.c_str(), isSelected) ||
                            (enterPressed && isSelected))
                        {
                            AddNodeAtCenter(graph, nodeTypes[sr.nodeTypeIndex].mTypeId);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled("[%s]", nodeTypes[sr.nodeTypeIndex].mCategory.c_str());
                        ImGui::PopID();
                    }
                    else if (sr.nodeTypeIndex == UINT32_MAX)
                    {
                        // Function graph
                        NodeGraph* fg = sEditedAsset->GetFunctionGraph(sr.funcIndex);
                        ImGui::PushID((int)(nodeTypes.size() + sr.funcIndex));
                        if (ImGui::Selectable(fg->GetGraphName().c_str(), isSelected) ||
                            (enterPressed && isSelected))
                        {
                            AddFunctionCallNode(graph, fg->GetGraphName());
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled("[Function]");
                        ImGui::PopID();
                    }
                    else if (sr.nodeTypeIndex == UINT32_MAX - 1)
                    {
                        // Get Variable
                        const GraphVariable* var = sEditedAsset->GetVariable(sr.funcIndex);
                        char label[256];
                        snprintf(label, sizeof(label), "Get %s", var->mName.c_str());
                        ImGui::PushID((int)(nodeTypes.size() + 1000 + sr.funcIndex * 2));
                        if (ImGui::Selectable(label, isSelected) ||
                            (enterPressed && isSelected))
                        {
                            AddGetVariableNode(graph, var->mName);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled("[Variable]");
                        ImGui::PopID();
                    }
                    else if (sr.nodeTypeIndex == UINT32_MAX - 2)
                    {
                        // Set Variable
                        const GraphVariable* var = sEditedAsset->GetVariable(sr.funcIndex);
                        char label[256];
                        snprintf(label, sizeof(label), "Set %s", var->mName.c_str());
                        ImGui::PushID((int)(nodeTypes.size() + 1001 + sr.funcIndex * 2));
                        if (ImGui::Selectable(label, isSelected) ||
                            (enterPressed && isSelected))
                        {
                            AddSetVariableNode(graph, var->mName);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled("[Variable]");
                        ImGui::PopID();
                    }

                    // Auto-scroll to keep selected item visible
                    if (isSelected && (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)))
                    {
                        ImGui::SetScrollHereY();
                    }
                }
            }
            else
            {
                // --- Category submenus ---
                std::string lastCategory;

                for (uint32_t i = 0; i < nodeTypes.size(); ++i)
                {
                    if (nodeTypes[i].mCategory != lastCategory)
                    {
                        if (!lastCategory.empty())
                        {
                            ImGui::EndMenu();
                        }

                        lastCategory = nodeTypes[i].mCategory;

                        if (!ImGui::BeginMenu(lastCategory.c_str()))
                        {
                            // Skip all nodes in this category
                            std::string skipCat = lastCategory;
                            while (i + 1 < nodeTypes.size() && nodeTypes[i + 1].mCategory == skipCat)
                            {
                                ++i;
                            }
                            lastCategory.clear();
                            continue;
                        }
                    }

                    if (ImGui::MenuItem(nodeTypes[i].mTypeName.c_str()))
                    {
                        AddNodeAtCenter(graph, nodeTypes[i].mTypeId);
                    }
                }

                if (!lastCategory.empty())
                {
                    ImGui::EndMenu();
                }

                // Functions submenu
                if (sEditedAsset != nullptr && sEditedAsset->GetNumFunctionGraphs() > 0)
                {
                    if (ImGui::BeginMenu("Functions"))
                    {
                        for (uint32_t i = 0; i < sEditedAsset->GetNumFunctionGraphs(); ++i)
                        {
                            NodeGraph* fg = sEditedAsset->GetFunctionGraph(i);
                            if (fg != nullptr)
                            {
                                if (ImGui::MenuItem(fg->GetGraphName().c_str()))
                                {
                                    AddFunctionCallNode(graph, fg->GetGraphName());
                                }
                            }
                        }
                        ImGui::EndMenu();
                    }
                }

                // Variables submenu
                if (sEditedAsset != nullptr && sEditedAsset->GetNumVariables() > 0)
                {
                    if (ImGui::BeginMenu("Variables"))
                    {
                        for (uint32_t i = 0; i < sEditedAsset->GetNumVariables(); ++i)
                        {
                            const GraphVariable* var = sEditedAsset->GetVariable(i);
                            if (var == nullptr)
                                continue;

                            char getName[256];
                            snprintf(getName, sizeof(getName), "Get %s", var->mName.c_str());
                            if (ImGui::MenuItem(getName))
                            {
                                AddGetVariableNode(graph, var->mName);
                            }

                            char setName[256];
                            snprintf(setName, sizeof(setName), "Set %s", var->mName.c_str());
                            if (ImGui::MenuItem(setName))
                            {
                                AddSetVariableNode(graph, var->mName);
                            }

                            if (i + 1 < sEditedAsset->GetNumVariables())
                            {
                                ImGui::Separator();
                            }
                        }
                        ImGui::EndMenu();
                    }
                }

                // Paste
                ImGui::Separator();
                if (ImGui::MenuItem("Paste", "Ctrl+V"))
                {
                    glm::vec2 pastePos(sContextCanvasPos.x, sContextCanvasPos.y);
                    GraphClipboard::PasteFromClipboard(graph, pastePos, sEditedAsset);
                }

                // Import
                if (ImGui::MenuItem("Import..."))
                {
                    sShowImportPopup = true;
                    sFilePathBuffer[0] = '\0';
                }
            }
        }
        else
        {
            ImGui::TextDisabled("No domain set");
        }

        // Create Function from Selection (always visible at bottom)
        int selCount = ed::GetSelectedObjectCount();
        if (selCount > 0)
        {
            ImGui::Separator();
            if (ImGui::MenuItem("Create Function from Selection"))
            {
                CreateFunctionFromSelection(graph);
            }
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Node Context"))
    {
        GraphNodeId nodeId = FromNodeId(sContextNodeId);
        GraphNode* node = graph.FindNode(nodeId);

        if (node != nullptr)
        {
            ImGui::Text("%s", node->GetNodeTypeName());
            ImGui::Separator();
        }

        // Remove output pin option for FunctionOutputNode
        if (node != nullptr && node->GetType() == FunctionOutputNode::GetStaticType())
        {
            FunctionOutputNode* funcOutput = static_cast<FunctionOutputNode*>(node);
            if (funcOutput->GetNumInputPins() > 0)
            {
                if (ImGui::BeginMenu("Remove Output Pin"))
                {
                    for (uint32_t i = 0; i < funcOutput->GetNumInputPins(); ++i)
                    {
                        if (ImGui::MenuItem(funcOutput->GetInputPins()[i].mName.c_str()))
                        {
                            funcOutput->RemoveOutputField(i);
                            break;
                        }
                    }
                    ImGui::EndMenu();
                }
            }
        }

        if (ImGui::MenuItem("Copy"))
        {
            std::vector<GraphNodeId> nodeIds;
            int selectedCount = ed::GetSelectedObjectCount();
            if (selectedCount > 0)
            {
                std::vector<ed::NodeId> selectedEdIds(selectedCount);
                int nodeCount = ed::GetSelectedNodes(selectedEdIds.data(), selectedCount);
                for (int i = 0; i < nodeCount; ++i)
                {
                    nodeIds.push_back(FromNodeId(selectedEdIds[i]));
                }
            }
            if (nodeIds.empty())
            {
                nodeIds.push_back(nodeId);
            }
            GraphClipboard::CopyToClipboard(graph, nodeIds);
        }

        if (ImGui::MenuItem("Delete"))
        {
            ed::DeleteNode(sContextNodeId);
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context"))
    {
        if (ImGui::MenuItem("Delete"))
        {
            ed::DeleteLink(sContextLinkId);
        }

        ImGui::EndPopup();
    }

    ed::Resume();
}

static void SyncPositionsToEditor(NodeGraph& graph)
{
    const std::vector<GraphNode*>& nodes = graph.GetNodes();
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        glm::vec2 pos = nodes[i]->GetEditorPosition();
        ed::SetNodePosition(MakeNodeId(nodes[i]->GetId()), ImVec2(pos.x, pos.y));
    }
}

static void UpdateNodePositions(NodeGraph& graph)
{
    const std::vector<GraphNode*>& nodes = graph.GetNodes();
    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        ImVec2 pos = ed::GetNodePosition(MakeNodeId(nodes[i]->GetId()));
        nodes[i]->SetEditorPosition(glm::vec2(pos.x, pos.y));
    }
}

static void SwitchToGraph(NodeGraphAsset* asset, int32_t index)
{
    if (asset == nullptr)
        return;

    // Save current node positions before switching
    if (sEditedGraph != nullptr && sEditorContext != nullptr)
    {
        ed::SetCurrentEditor(sEditorContext);
        UpdateNodePositions(*sEditedGraph);
    }

    // Determine which editor context key to use
    std::string currentKey;
    if (sSelectedGraphIndex == -1)
    {
        currentKey = "__main__";
    }
    else if (sSelectedGraphIndex >= 0 && sSelectedGraphIndex < (int32_t)asset->GetNumFunctionGraphs())
    {
        currentKey = asset->GetFunctionGraph(sSelectedGraphIndex)->GetGraphName();
    }

    // Store the current context in the map
    if (sEditorContext != nullptr && !currentKey.empty())
    {
        sEditorContextMap[currentKey] = sEditorContext;
        sEditorContext = nullptr;
    }

    // Update selection
    sSelectedGraphIndex = index;

    // Set the edited graph
    if (index == -1)
    {
        sEditedGraph = &asset->GetGraph();
    }
    else if (index >= 0 && index < (int32_t)asset->GetNumFunctionGraphs())
    {
        sEditedGraph = asset->GetFunctionGraph(index);
    }
    else
    {
        return;
    }

    // Get or create editor context for the new graph
    std::string newKey;
    if (index == -1)
    {
        newKey = "__main__";
    }
    else
    {
        newKey = sEditedGraph->GetGraphName();
    }

    auto it = sEditorContextMap.find(newKey);
    if (it != sEditorContextMap.end())
    {
        sEditorContext = it->second;
        sEditorContextMap.erase(it);
    }
    else
    {
        sEditorContext = CreateNewEditorContext();
        sNeedsPositionSync = true;
    }
}

static void DrawFunctionSidebar(NodeGraphAsset* asset)
{
    ImGui::BeginChild("FunctionSidebar", ImVec2(kFunctionSidebarWidth, 0), true);

    // Event Graph entry (always present)
    bool isMainSelected = (sSelectedGraphIndex == -1);
    if (ImGui::Selectable("Event Graph", isMainSelected))
    {
        if (!isMainSelected)
        {
            SwitchToGraph(asset, -1);
        }
    }

    ImGui::Separator();

    // Functions header with "+" button
    ImGui::Text("Functions");
    ImGui::SameLine(kFunctionSidebarWidth - 40.0f);
    if (ImGui::SmallButton("+"))
    {
        static int sNewFuncCounter = 1;
        char name[64];
        snprintf(name, sizeof(name), "NewFunction_%d", sNewFuncCounter++);
        NodeGraph* fg = asset->AddFunctionGraph(name);
        if (fg != nullptr)
        {
            // Add a FunctionOutputNode to the new function graph
            GraphNode* outputNode = fg->AddNode(FunctionOutputNode::GetStaticType());
            if (outputNode != nullptr)
            {
                outputNode->SetEditorPosition(glm::vec2(400.0f, 200.0f));
            }

            // Switch to the new function graph
            uint32_t newIdx = asset->GetNumFunctionGraphs() - 1;
            SwitchToGraph(asset, (int32_t)newIdx);

            // Start rename
            sRenamingFunction = true;
            sRenamingIndex = (int32_t)newIdx;
            strncpy(sRenameBuffer, name, sizeof(sRenameBuffer) - 1);
            sRenameBuffer[sizeof(sRenameBuffer) - 1] = '\0';
        }
    }

    // Function list
    for (uint32_t i = 0; i < asset->GetNumFunctionGraphs(); ++i)
    {
        NodeGraph* fg = asset->GetFunctionGraph(i);
        if (fg == nullptr)
            continue;

        ImGui::PushID((int)i);

        bool isSelected = (sSelectedGraphIndex == (int32_t)i);

        if (sRenamingFunction && sRenamingIndex == (int32_t)i)
        {
            // Inline rename
            ImGui::SetNextItemWidth(kFunctionSidebarWidth - 30.0f);
            if (ImGui::InputText("##rename", sRenameBuffer, sizeof(sRenameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                if (strlen(sRenameBuffer) > 0)
                {
                    asset->RenameFunctionGraph(i, sRenameBuffer);
                }
                sRenamingFunction = false;
                sRenamingIndex = -1;
            }
            // Cancel on escape or lost focus
            if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
                (!ImGui::IsItemActive() && !ImGui::IsItemFocused() && sRenamingFunction))
            {
                sRenamingFunction = false;
                sRenamingIndex = -1;
            }
            // Auto-focus the rename field on first frame
            if (sRenamingFunction && sRenamingIndex == (int32_t)i)
            {
                ImGui::SetKeyboardFocusHere(-1);
            }
        }
        else
        {
            if (ImGui::Selectable(fg->GetGraphName().c_str(), isSelected))
            {
                SwitchToGraph(asset, (int32_t)i);
            }

            // Drag source for drag-drop onto canvas
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                const std::string& funcName = fg->GetGraphName();
                ImGui::SetDragDropPayload("DND_FUNCTION", funcName.c_str(), funcName.size() + 1);
                ImGui::Text("Function: %s", funcName.c_str());
                ImGui::EndDragDropSource();
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Rename"))
                {
                    sRenamingFunction = true;
                    sRenamingIndex = (int32_t)i;
                    strncpy(sRenameBuffer, fg->GetGraphName().c_str(), sizeof(sRenameBuffer) - 1);
                    sRenameBuffer[sizeof(sRenameBuffer) - 1] = '\0';
                }
                if (ImGui::MenuItem("Delete"))
                {
                    // Switch to main graph if we're deleting the current one
                    if (sSelectedGraphIndex == (int32_t)i)
                    {
                        SwitchToGraph(asset, -1);
                    }
                    else if (sSelectedGraphIndex > (int32_t)i)
                    {
                        sSelectedGraphIndex--;
                    }
                    asset->RemoveFunctionGraph(i);
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break;  // Break because we modified the vector
                }
                ImGui::EndPopup();
            }
        }

        ImGui::PopID();
    }

    // ---- Variables Section ----
    ImGui::Separator();
    ImGui::Text("Variables");
    ImGui::SameLine(kFunctionSidebarWidth - 40.0f);
    if (ImGui::SmallButton("+##AddVar"))
    {
        ImGui::OpenPopup("AddVariablePopup");
    }

    if (ImGui::BeginPopup("AddVariablePopup"))
    {
        static char sNewVarName[128] = "NewVariable";
        ImGui::InputText("Name", sNewVarName, sizeof(sNewVarName));

        const char* typeNames[] = { "Float", "Integer", "Bool", "String", "Vector2D", "Vector", "Color" };
        DatumType typeValues[] = {
            DatumType::Float, DatumType::Integer, DatumType::Bool, DatumType::String,
            DatumType::Vector2D, DatumType::Vector, DatumType::Color
        };
        static int sSelectedType = 0;
        ImGui::Combo("Type", &sSelectedType, typeNames, IM_ARRAYSIZE(typeNames));

        if (ImGui::Button("Add"))
        {
            if (strlen(sNewVarName) > 0)
            {
                asset->AddVariable(sNewVarName, typeValues[sSelectedType]);
                snprintf(sNewVarName, sizeof(sNewVarName), "NewVariable");
                sSelectedType = 0;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Variable list
    for (uint32_t i = 0; i < asset->GetNumVariables(); ++i)
    {
        GraphVariable* var = asset->GetVariable(i);
        if (var == nullptr)
            continue;

        ImGui::PushID((int)(1000 + i));

        if (sRenamingVariable && sRenamingVarIndex == (int32_t)i)
        {
            // Inline rename
            ImGui::SetNextItemWidth(kFunctionSidebarWidth - 30.0f);
            if (ImGui::InputText("##varRename", sVarRenameBuffer, sizeof(sVarRenameBuffer),
                ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                if (strlen(sVarRenameBuffer) > 0)
                {
                    asset->RenameVariable(i, sVarRenameBuffer);
                }
                sRenamingVariable = false;
                sRenamingVarIndex = -1;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
                (!ImGui::IsItemActive() && !ImGui::IsItemFocused() && sRenamingVariable))
            {
                sRenamingVariable = false;
                sRenamingVarIndex = -1;
            }
            if (sRenamingVariable && sRenamingVarIndex == (int32_t)i)
            {
                ImGui::SetKeyboardFocusHere(-1);
            }
        }
        else
        {
            // Type color indicator
            glm::vec4 typeColor = GetDatumTypeColor(var->mType);
            ImGui::ColorButton("##typeCol", ImVec4(typeColor.x, typeColor.y, typeColor.z, typeColor.w),
                ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(8, 14));
            ImGui::SameLine();

            ImGui::Selectable(var->mName.c_str(), false);

            // Drag source for drag-drop onto canvas
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                ImGui::SetDragDropPayload("DND_VARIABLE", var->mName.c_str(), var->mName.size() + 1);
                ImGui::Text("Variable: %s", var->mName.c_str());
                ImGui::EndDragDropSource();
            }

            // Right-click context menu
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Rename"))
                {
                    sRenamingVariable = true;
                    sRenamingVarIndex = (int32_t)i;
                    strncpy(sVarRenameBuffer, var->mName.c_str(), sizeof(sVarRenameBuffer) - 1);
                    sVarRenameBuffer[sizeof(sVarRenameBuffer) - 1] = '\0';
                }

                if (ImGui::BeginMenu("Change Type"))
                {
                    const char* typeNames2[] = { "Float", "Integer", "Bool", "String", "Vector2D", "Vector", "Color" };
                    DatumType typeValues2[] = {
                        DatumType::Float, DatumType::Integer, DatumType::Bool, DatumType::String,
                        DatumType::Vector2D, DatumType::Vector, DatumType::Color
                    };
                    for (int t = 0; t < 7; ++t)
                    {
                        bool isCurrent = (var->mType == typeValues2[t]);
                        if (ImGui::MenuItem(typeNames2[t], nullptr, isCurrent))
                        {
                            if (!isCurrent)
                            {
                                var->mType = typeValues2[t];
                                // Reset default value for new type
                                switch (typeValues2[t])
                                {
                                case DatumType::Integer:  var->mDefaultValue = Datum(0); break;
                                case DatumType::Float:    var->mDefaultValue = Datum(0.0f); break;
                                case DatumType::Bool:     var->mDefaultValue = Datum(false); break;
                                case DatumType::String:   var->mDefaultValue = Datum(std::string("")); break;
                                case DatumType::Vector2D: var->mDefaultValue = Datum(glm::vec2(0.0f)); break;
                                case DatumType::Vector:   var->mDefaultValue = Datum(glm::vec3(0.0f)); break;
                                case DatumType::Color:    var->mDefaultValue = Datum(glm::vec4(1.0f)); break;
                                default: break;
                                }
                                var->mRuntimeValue = var->mDefaultValue;
                            }
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::MenuItem("Delete"))
                {
                    asset->RemoveVariable(i);
                    ImGui::EndPopup();
                    ImGui::PopID();
                    break;
                }
                ImGui::EndPopup();
            }
        }

        // Inline default value editor
        if (!sRenamingVariable || sRenamingVarIndex != (int32_t)i)
        {
            ImGui::PushID((int)(2000 + i));
            ImGui::SetNextItemWidth(kFunctionSidebarWidth - 30.0f);
            switch (var->mType)
            {
            case DatumType::Float:
            {
                float f = var->mDefaultValue.GetFloat();
                if (ImGui::DragFloat("##def", &f, 0.01f))
                {
                    var->mDefaultValue = Datum(f);
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            case DatumType::Integer:
            {
                int32_t v = var->mDefaultValue.GetInteger();
                if (ImGui::DragInt("##def", &v))
                {
                    var->mDefaultValue = Datum(v);
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            case DatumType::Bool:
            {
                bool b = var->mDefaultValue.GetBool();
                if (ImGui::Checkbox("##def", &b))
                {
                    var->mDefaultValue = Datum(b);
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            case DatumType::String:
            {
                static char buf[256];
                std::string s = var->mDefaultValue.GetString();
                strncpy(buf, s.c_str(), sizeof(buf) - 1);
                buf[sizeof(buf) - 1] = '\0';
                if (ImGui::InputText("##def", buf, sizeof(buf)))
                {
                    var->mDefaultValue = Datum(std::string(buf));
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            case DatumType::Vector2D:
            {
                glm::vec2 v = var->mDefaultValue.GetVector2D();
                if (ImGui::DragFloat2("##def", &v.x, 0.01f))
                {
                    var->mDefaultValue = Datum(v);
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            case DatumType::Vector:
            {
                glm::vec3 v = var->mDefaultValue.GetVector();
                if (ImGui::DragFloat3("##def", &v.x, 0.01f))
                {
                    var->mDefaultValue = Datum(v);
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            case DatumType::Color:
            {
                glm::vec4 c = var->mDefaultValue.GetColor();
                if (ImGui::ColorEdit4("##def", &c.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
                {
                    var->mDefaultValue = Datum(c);
                    var->mRuntimeValue = var->mDefaultValue;
                }
                break;
            }
            default: break;
            }
            ImGui::PopID();
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void DrawNodeGraphContent()
{
    if (sEditedGraph == nullptr)
    {
        ImGui::TextDisabled("No node graph open. Double-click a Node Graph asset to open it.");
        return;
    }

    EnsureEditorContext();

    // Draw sidebar + canvas layout if we have a NodeGraphAsset
    if (sEditedAsset != nullptr)
    {
        DrawFunctionSidebar(sEditedAsset);
        ImGui::SameLine();
    }

    // Canvas group
    ImGui::BeginGroup();

    NodeGraph& graph = *sEditedGraph;

    // Toolbar - line 1: name + preview
    const char* displayName = sEditedOwner ? sEditedOwner->GetName().c_str() : "Node Graph";
    if (sSelectedGraphIndex >= 0 && sEditedAsset != nullptr)
    {
        // Show function name when editing a function
        ImGui::Text("%s > %s", displayName, graph.GetGraphName().c_str());
    }
    else
    {
        ImGui::Text("%s", displayName);
    }
    ImGui::SameLine();

    if (ImGui::Checkbox("Preview", &sPreviewEnabled))
    {
        if (!sPreviewEnabled)
        {
            sNodeFlashTimes.clear();
        }
    }

    if (sPreviewEnabled)
    {
        // Collect unique event names so execution chains actually fire during preview
        std::unordered_set<std::string> previewEvents;
        const std::vector<GraphNode*>& allNodes = graph.GetNodes();
        for (uint32_t i = 0; i < allNodes.size(); ++i)
        {
            if (allNodes[i]->IsEventNode())
            {
                previewEvents.insert(allNodes[i]->GetEventName());
            }
        }

        if (previewEvents.empty())
        {
            sProcessor.Evaluate(&graph);
        }
        else
        {
            for (const auto& evt : previewEvents)
            {
                sProcessor.Evaluate(&graph, evt.c_str());
            }
        }
        RecordExecutionFlash(graph);

        // If owner is a MaterialLite, apply graph values to material params
        if (sEditedOwner != nullptr)
        {
            Material* mat = sEditedOwner->Is(Material::ClassRuntimeId()) ? (Material*)sEditedOwner : nullptr;
            if (mat != nullptr && mat->HasNodeGraph())
            {
                mat->ApplyGraphValues(&graph);
            }
            else
            {
                GraphDomainManager* domainMgr = GraphDomainManager::Get();
                if (domainMgr != nullptr)
                {
                    GraphDomain* domain = domainMgr->GetDomain(graph.GetDomainName().c_str());
                    if (domain != nullptr)
                    {
                        domain->OnGraphEvaluated(&graph);
                    }
                }
            }
        }
    }

    // During Play In Editor, read execution state from the runtime graph
    sRuntimeGraph = nullptr;
    if (!sPreviewEnabled && IsPlayingInEditor())
    {
        sRuntimeGraph = FindRuntimeGraph();
        if (sRuntimeGraph != nullptr)
        {
            RecordExecutionFlash(*sRuntimeGraph);
        }
    }

    ImGui::SameLine();
    ImGui::Text("Nodes: %u  Links: %u", graph.GetNumNodes(), graph.GetNumLinks());

    // Toolbar - line 2: quick-add buttons
    if (ImGui::SmallButton("+ Float"))  { AddNodeAtCenter(graph, FloatConstantNode::GetStaticType()); }
    ImGui::SameLine();
    if (ImGui::SmallButton("+ Int"))    { AddNodeAtCenter(graph, IntConstantNode::GetStaticType()); }
    ImGui::SameLine();
    if (ImGui::SmallButton("+ Vector")) { AddNodeAtCenter(graph, VectorConstantNode::GetStaticType()); }
    ImGui::SameLine();
    if (ImGui::SmallButton("+ Color"))  { AddNodeAtCenter(graph, ColorConstantNode::GetStaticType()); }
    ImGui::SameLine();
    if (ImGui::SmallButton("+ Input"))  { ImGui::OpenPopup("AddInputPopup"); }

    if (ImGui::BeginPopup("AddInputPopup"))
    {
        if (ImGui::MenuItem("Float Input"))  { AddNodeAtCenter(graph, FloatInputNode::GetStaticType()); }
        if (ImGui::MenuItem("Int Input"))    { AddNodeAtCenter(graph, IntInputNode::GetStaticType()); }
        if (ImGui::MenuItem("Bool Input"))   { AddNodeAtCenter(graph, BoolInputNode::GetStaticType()); }
        if (ImGui::MenuItem("String Input")) { AddNodeAtCenter(graph, StringInputNode::GetStaticType()); }
        if (ImGui::MenuItem("Vector Input")) { AddNodeAtCenter(graph, VectorInputNode::GetStaticType()); }
        if (ImGui::MenuItem("Color Input"))  { AddNodeAtCenter(graph, ColorInputNode::GetStaticType()); }
        if (ImGui::MenuItem("Asset Input"))  { AddNodeAtCenter(graph, AssetInputNode::GetStaticType()); }
        if (ImGui::MenuItem("Byte Input"))   { AddNodeAtCenter(graph, ByteInputNode::GetStaticType()); }
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    if (ImGui::SmallButton("Save"))
    {
        if (sEditedOwner != nullptr)
        {
            AssetManager::Get()->SaveAsset(sEditedOwner->GetName());
        }
    }

    ImGui::SameLine();
    if (ImGui::SmallButton("Export"))
    {
        int selectedCount = ed::GetSelectedObjectCount();
        if (selectedCount > 0)
        {
            sShowExportPopup = true;
            sFilePathBuffer[0] = '\0';
        }
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Import"))
    {
        sShowImportPopup = true;
        sFilePathBuffer[0] = '\0';
    }

    // Export popup
    if (sShowExportPopup)
    {
        ImGui::OpenPopup("ExportNodesPopup");
        sShowExportPopup = false;
    }
    if (ImGui::BeginPopup("ExportNodesPopup"))
    {
        ImGui::Text("Export Selection to File");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("Path", sFilePathBuffer, sizeof(sFilePathBuffer));

        if (ImGui::Button("Export"))
        {
            if (strlen(sFilePathBuffer) > 0)
            {
                int selectedCount = ed::GetSelectedObjectCount();
                if (selectedCount > 0)
                {
                    std::vector<ed::NodeId> selectedEdIds(selectedCount);
                    int nodeCount = ed::GetSelectedNodes(selectedEdIds.data(), selectedCount);
                    if (nodeCount > 0)
                    {
                        std::vector<GraphNodeId> nodeIds;
                        for (int i = 0; i < nodeCount; ++i)
                        {
                            nodeIds.push_back(FromNodeId(selectedEdIds[i]));
                        }
                        // Append .octgraph if no extension
                        std::string path = sFilePathBuffer;
                        if (path.find('.') == std::string::npos)
                        {
                            path += ".octgraph";
                        }
                        GraphClipboard::ExportToFile(graph, nodeIds, path);
                    }
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // Import popup
    if (sShowImportPopup)
    {
        ImGui::OpenPopup("ImportNodesPopup");
        sShowImportPopup = false;
    }
    if (ImGui::BeginPopup("ImportNodesPopup"))
    {
        ImGui::Text("Import Nodes from File");
        ImGui::SetNextItemWidth(300.0f);
        ImGui::InputText("Path", sFilePathBuffer, sizeof(sFilePathBuffer));

        if (ImGui::Button("Import"))
        {
            if (strlen(sFilePathBuffer) > 0)
            {
                glm::vec2 importPos(sCanvasCenter.x, sCanvasCenter.y);
                GraphClipboard::ImportFromFile(graph, sFilePathBuffer, importPos, sEditedAsset);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    // Node editor canvas
    ed::SetCurrentEditor(sEditorContext);
    ed::Begin("NodeGraphEditor");

    // Cache visible canvas center (used by AddNodeAtCenter)
    {
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();
        ImVec2 screenCenter = ImVec2(winPos.x + winSize.x * 0.5f, winPos.y + winSize.y * 0.5f);
        sCanvasCenter = ed::ScreenToCanvas(screenCenter);
    }

    if (sNeedsPositionSync)
    {
        SyncPositionsToEditor(graph);
        sNeedsPositionSync = false;
    }

    // Apply pending node position from AddNodeAtCenter (deferred to when editor is active)
    if (sPendingNodeId != 0)
    {
        ed::SetNodePosition(MakeNodeId(sPendingNodeId), ImVec2(sPendingNodePos.x, sPendingNodePos.y));
        sPendingNodeId = 0;
    }

    // Handle drag-drop from function sidebar onto canvas
    if (sEditedAsset != nullptr)
    {
        // Accept drag-drop on the canvas area
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_FUNCTION"))
            {
                const char* funcName = static_cast<const char*>(payload->Data);
                AddFunctionCallNode(graph, funcName);
            }
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_VARIABLE"))
            {
                const char* varName = static_cast<const char*>(payload->Data);
                bool altHeld = ImGui::GetIO().KeyAlt;
                if (altHeld)
                {
                    AddSetVariableNode(graph, varName);
                }
                else
                {
                    AddGetVariableNode(graph, varName);
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    DrawNodes(graph);
    DrawLinks(graph);
    HandleCreation(graph);

    // Delete key hotkey — remove selected items directly from the graph
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !ImGui::IsAnyItemActive())
    {
        int selectedCount = ed::GetSelectedObjectCount();
        if (selectedCount > 0)
        {
            std::vector<ed::NodeId> selectedNodes(selectedCount);
            std::vector<ed::LinkId> selectedLinks(selectedCount);
            int nodeCount = ed::GetSelectedNodes(selectedNodes.data(), selectedCount);
            int linkCount = ed::GetSelectedLinks(selectedLinks.data(), selectedCount);

            for (int i = 0; i < linkCount; ++i)
            {
                graph.RemoveLink(FromLinkId(selectedLinks[i]));
            }
            for (int i = 0; i < nodeCount; ++i)
            {
                graph.RemoveNode(FromNodeId(selectedNodes[i]));
            }

            ed::ClearSelection();
        }
    }

    // Ctrl+C — Copy selected nodes to clipboard
    if (ImGui::IsKeyPressed(ImGuiKey_C) && ImGui::GetIO().KeyCtrl && !ImGui::IsAnyItemActive())
    {
        int selectedCount = ed::GetSelectedObjectCount();
        if (selectedCount > 0)
        {
            std::vector<ed::NodeId> selectedEdIds(selectedCount);
            int nodeCount = ed::GetSelectedNodes(selectedEdIds.data(), selectedCount);
            if (nodeCount > 0)
            {
                std::vector<GraphNodeId> nodeIds;
                for (int i = 0; i < nodeCount; ++i)
                {
                    nodeIds.push_back(FromNodeId(selectedEdIds[i]));
                }
                GraphClipboard::CopyToClipboard(graph, nodeIds);
            }
        }
    }

    // Ctrl+V — Paste from clipboard
    if (ImGui::IsKeyPressed(ImGuiKey_V) && ImGui::GetIO().KeyCtrl && !ImGui::IsAnyItemActive())
    {
        glm::vec2 pastePos(sCanvasCenter.x, sCanvasCenter.y);
        GraphClipboard::PasteFromClipboard(graph, pastePos, sEditedAsset);
    }

    // Ctrl+D — Duplicate selection
    if (ImGui::IsKeyPressed(ImGuiKey_D) && ImGui::GetIO().KeyCtrl && !ImGui::IsAnyItemActive())
    {
        int selectedCount = ed::GetSelectedObjectCount();
        if (selectedCount > 0)
        {
            std::vector<ed::NodeId> selectedEdIds(selectedCount);
            int nodeCount = ed::GetSelectedNodes(selectedEdIds.data(), selectedCount);
            if (nodeCount > 0)
            {
                std::vector<GraphNodeId> nodeIds;
                for (int i = 0; i < nodeCount; ++i)
                {
                    nodeIds.push_back(FromNodeId(selectedEdIds[i]));
                }

                // Serialize to temp stream, deserialize back with offset
                Stream tempStream;
                GraphClipboard::SerializeSelection(graph, nodeIds, tempStream);
                tempStream.SetPos(0);

                // Get centroid of selected nodes for offset
                glm::vec2 centroid(0.0f);
                for (int i = 0; i < nodeCount; ++i)
                {
                    GraphNode* node = graph.FindNode(nodeIds[i]);
                    if (node != nullptr)
                    {
                        centroid += node->GetEditorPosition();
                    }
                }
                if (nodeCount > 0)
                {
                    centroid /= (float)nodeCount;
                }

                GraphClipboard::DeserializeIntoGraph(graph, tempStream, centroid + glm::vec2(50.0f, 50.0f), sEditedAsset);
                ed::ClearSelection();
            }
        }
    }

    HandleDeletion(graph);
    DrawContextMenu(graph);

    // Deferred pin enum popup (must be in Suspend block for correct screen-space positioning)
    ed::Suspend();
    if (sPinEnumJustOpened)
    {
        ImGui::OpenPopup("PinEnumPopup");
        sPinEnumJustOpened = false;
    }
    if (ImGui::BeginPopup("PinEnumPopup"))
    {
        if (sPinEnumTarget != nullptr && sPinEnumOwnerNode != nullptr)
        {
            std::vector<PinEnumOption> enumOptions;
            sPinEnumOwnerNode->GetPinEnumOptions(sPinEnumPinIndex, enumOptions);

            int32_t currentVal = sPinEnumTarget->mDefaultValue.GetInteger();
            for (int i = 0; i < (int)enumOptions.size(); ++i)
            {
                bool selected = (enumOptions[i].mValue == currentVal);
                if (ImGui::MenuItem(enumOptions[i].mName, nullptr, selected))
                {
                    sPinEnumTarget->mDefaultValue = Datum(enumOptions[i].mValue);
                    sPinEnumTarget->mValue = sPinEnumTarget->mDefaultValue;
                }
            }
        }
        ImGui::EndPopup();
    }
    ed::Resume();

    ed::End();

    UpdateNodePositions(graph);

    ImGui::EndGroup();
}

void OpenNodeGraphForEditing(NodeGraphAsset* asset)
{
    if (asset == nullptr)
    {
        return;
    }

    // Clean up previous editor contexts
    for (auto& pair : sEditorContextMap)
    {
        ed::DestroyEditor(pair.second);
    }
    sEditorContextMap.clear();
    sSelectedGraphIndex = -1;
    sRenamingFunction = false;
    sRenamingIndex = -1;

    sEditedGraph = &asset->GetGraph();
    sEditedOwner = asset;
    sEditedAsset = asset;
    sRenamingVariable = false;
    sRenamingVarIndex = -1;
    GetEditorState()->mShowNodeGraphPanel = true;
    sNeedsPositionSync = true;

    // Wire FunctionCallNodes and VariableNodes in asset
    asset->ResolveFunctionCallNodes();
    asset->ResolveVariableNodes();

    // Reset editor context for new graph
    if (sEditorContext != nullptr)
    {
        ed::DestroyEditor(sEditorContext);
        sEditorContext = nullptr;
    }
}

void OpenNodeGraphForEditing(NodeGraph* graph, Asset* owner)
{
    if (graph == nullptr)
    {
        return;
    }

    // Clean up previous editor contexts
    for (auto& pair : sEditorContextMap)
    {
        ed::DestroyEditor(pair.second);
    }
    sEditorContextMap.clear();
    sSelectedGraphIndex = -1;
    sRenamingFunction = false;
    sRenamingIndex = -1;

    sEditedGraph = graph;
    sEditedOwner = owner;
    sEditedAsset = nullptr;
    GetEditorState()->mShowNodeGraphPanel = true;
    sNeedsPositionSync = true;

    // Reset editor context for new graph
    if (sEditorContext != nullptr)
    {
        ed::DestroyEditor(sEditorContext);
        sEditorContext = nullptr;
    }
}

void CloseNodeGraphPanel()
{
    sEditedGraph = nullptr;
    sEditedOwner = nullptr;
    sEditedAsset = nullptr;
    sSelectedGraphIndex = -1;
    sRenamingFunction = false;
    sRenamingIndex = -1;
    GetEditorState()->mShowNodeGraphPanel = false;

    if (sEditorContext != nullptr)
    {
        ed::DestroyEditor(sEditorContext);
        sEditorContext = nullptr;
    }

    for (auto& pair : sEditorContextMap)
    {
        ed::DestroyEditor(pair.second);
    }
    sEditorContextMap.clear();
}

#endif
