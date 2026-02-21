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
#include "Log.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-node-editor/imgui_node_editor.h"

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

static void EnsureEditorContext()
{
    if (sEditorContext == nullptr)
    {
        ed::Config config;
        config.SettingsFile = nullptr;
        sEditorContext = ed::CreateEditor(&config);
    }
}

static ImColor GetPinColor(DatumType type)
{
    glm::vec4 c = GetDatumTypeColor(type);
    return ImColor(c.x, c.y, c.z, c.w);
}

static void DrawPinIcon(DatumType type, bool connected)
{
    ImVec2 size(12.0f, 12.0f);
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(cursorPos.x + size.x * 0.5f, cursorPos.y + size.y * 0.5f);
    float radius = size.x * 0.5f;
    ImColor color = GetPinColor(type);

    if (connected)
    {
        drawList->AddCircleFilled(center, radius, color);
    }
    else
    {
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

static void DrawInputPinWidget(GraphPin& pin)
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
        int32_t v = pin.mDefaultValue.GetInteger();
        ImGui::SetNextItemWidth(60.0f);
        if (ImGui::DragInt("##v", &v))
        {
            pin.mDefaultValue = Datum(v);
            pin.mValue = pin.mDefaultValue;
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
        if (ImGui::ColorButton("##colbtn", ImVec4(c.x, c.y, c.z, c.w), ImGuiColorEditFlags_NoTooltip))
        {
            ImGui::OpenPopup("##colorpicker");
        }
        ed::Suspend();
        if (ImGui::BeginPopup("##colorpicker"))
        {
            if (ImGui::ColorPicker4("##picker", &c.x))
            {
                pin.mDefaultValue = Datum(c);
                pin.mValue = pin.mDefaultValue;
            }
            ImGui::EndPopup();
        }
        ed::Resume();
        break;
    }
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
    default:
        break;
    }
}

static void DrawNodes(NodeGraph& graph)
{
    const std::vector<GraphNode*>& nodes = graph.GetNodes();

    for (uint32_t i = 0; i < nodes.size(); ++i)
    {
        GraphNode* node = nodes[i];
        glm::vec4 color = node->GetNodeColor();
        ImColor headerColor(color.x, color.y, color.z, color.w);

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

        ImGui::Separator();

        // Input pins
        bool isSinkNode = (node->GetNumOutputPins() == 0);
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
                DrawInputPinWidget(pin);
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

        // Output pins
        for (uint32_t j = 0; j < node->GetNumOutputPins(); ++j)
        {
            const GraphPin& pin = node->GetOutputPins()[j];
            bool connected = IsPinConnected(graph, pin.mId);

            ImGui::PushID(pin.mId);
            DrawOutputPinValue(pin);
            ImGui::PopID();
            ImGui::SameLine();

            ed::BeginPin(MakePinId(pin.mId), ed::PinKind::Output);
            DrawPinIcon(pin.mDataType, connected);
            ed::EndPin();
        }

        ed::EndNode();
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

static void DrawContextMenu(NodeGraph& graph)
{
    static ImVec2 sContextMenuPos;
    static ed::NodeId sContextNodeId;
    static ed::LinkId sContextLinkId;

    // Detect right-click ourselves using hovered queries (more reliable than Show*ContextMenu)
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && !ImGui::IsAnyItemHovered())
    {
        ed::NodeId hoveredNode = ed::GetHoveredNode();
        ed::LinkId hoveredLink = ed::GetHoveredLink();

        sContextMenuPos = ImGui::GetMousePos();

        if (hoveredNode)
        {
            sContextNodeId = hoveredNode;
            ed::Suspend();
            ImGui::OpenPopup("Node Context");
            ed::Resume();
        }
        else if (hoveredLink)
        {
            sContextLinkId = hoveredLink;
            ed::Suspend();
            ImGui::OpenPopup("Link Context");
            ed::Resume();
        }
        else
        {
            ed::Suspend();
            ImGui::OpenPopup("Create Node");
            ed::Resume();
        }
    }

    // Draw popups (must be inside Suspend/Resume)
    ed::Suspend();

    if (ImGui::BeginPopup("Create Node"))
    {
        GraphDomainManager* domainMgr = GraphDomainManager::Get();
        GraphDomain* domain = nullptr;
        if (domainMgr != nullptr)
        {
            domain = domainMgr->GetDomain(graph.GetDomainName().c_str());
        }

        if (domain != nullptr)
        {
            const std::vector<GraphNodeTypeInfo>& nodeTypes = domain->GetNodeTypes();
            std::string lastCategory;

            for (uint32_t i = 0; i < nodeTypes.size(); ++i)
            {
                if (nodeTypes[i].mCategory != lastCategory)
                {
                    if (!lastCategory.empty())
                    {
                        ImGui::Separator();
                    }
                    ImGui::TextDisabled("%s", nodeTypes[i].mCategory.c_str());
                    lastCategory = nodeTypes[i].mCategory;
                }

                if (ImGui::MenuItem(nodeTypes[i].mTypeName.c_str()))
                {
                    GraphNode* newNode = graph.AddNode(nodeTypes[i].mTypeId);
                    if (newNode != nullptr)
                    {
                        ed::SetNodePosition(
                            MakeNodeId(newNode->GetId()),
                            ed::ScreenToCanvas(sContextMenuPos));
                    }
                }
            }
        }
        else
        {
            ImGui::TextDisabled("No domain set");
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

static void AddNodeAtCenter(NodeGraph& graph, TypeId nodeType)
{
    GraphNode* node = graph.AddNode(nodeType);
    if (node != nullptr)
    {
        ImVec2 center = ed::ScreenToCanvas(ImVec2(
            ImGui::GetWindowPos().x + ImGui::GetWindowWidth() * 0.5f,
            ImGui::GetWindowPos().y + ImGui::GetWindowHeight() * 0.5f));
        ed::SetNodePosition(MakeNodeId(node->GetId()), center);
    }
}

void DrawNodeGraphContent()
{
    if (sEditedGraph == nullptr)
    {
        ImGui::TextDisabled("No node graph open. Double-click a Node Graph asset to open it.");
        return;
    }

    EnsureEditorContext();
    NodeGraph& graph = *sEditedGraph;

    // Toolbar
    const char* displayName = sEditedOwner ? sEditedOwner->GetName().c_str() : "Node Graph";
    ImGui::Text("%s", displayName);
    ImGui::SameLine();

    if (ImGui::Checkbox("Preview", &sPreviewEnabled))
    {
    }

    if (sPreviewEnabled)
    {
        sProcessor.Evaluate(&graph);

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

    ImGui::SameLine();

    // Quick-add toolbar buttons
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
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
    ImGui::Text("Nodes: %u  Links: %u", graph.GetNumNodes(), graph.GetNumLinks());

    ImGui::Separator();

    // Node editor canvas
    ed::SetCurrentEditor(sEditorContext);
    ed::Begin("NodeGraphEditor");

    if (sNeedsPositionSync)
    {
        SyncPositionsToEditor(graph);
        sNeedsPositionSync = false;
    }

    DrawNodes(graph);
    DrawLinks(graph);
    HandleCreation(graph);
    HandleDeletion(graph);
    DrawContextMenu(graph);

    ed::End();

    UpdateNodePositions(graph);
}

void OpenNodeGraphForEditing(NodeGraphAsset* asset)
{
    if (asset == nullptr)
    {
        return;
    }

    sEditedGraph = &asset->GetGraph();
    sEditedOwner = asset;
    sEditedAsset = asset;
    GetEditorState()->mShowNodeGraphPanel = true;
    sNeedsPositionSync = true;

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
    GetEditorState()->mShowNodeGraphPanel = false;

    if (sEditorContext != nullptr)
    {
        ed::DestroyEditor(sEditorContext);
        sEditorContext = nullptr;
    }
}

#endif
