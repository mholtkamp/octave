#pragma once

#include "Factory.h"
#include "Object.h"
#include "NodeGraph/GraphTypes.h"
#include "NodeGraph/GraphPin.h"
#include "Maths.h"

#include <string>
#include <vector>

class Stream;

#define DECLARE_GRAPH_NODE(Class, Parent) \
    DECLARE_FACTORY(Class, GraphNode); \
    DECLARE_OBJECT(Class, Parent); \
    typedef Parent Super;

#define DEFINE_GRAPH_NODE(Class) \
    DEFINE_FACTORY(Class, GraphNode); \
    DEFINE_OBJECT(Class);

// Register a custom graph node type with a domain.
// Place this in your .cpp file after DEFINE_GRAPH_NODE.
// The node will be added to the specified domain's context menu.
//
// Usage:
//   REGISTER_GRAPH_NODE(MyNode, "My Node", "Custom", "Material", glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));
//
// Parameters:
//   Class      - The node class name (must match DEFINE_GRAPH_NODE)
//   TypeName   - Display name shown in the context menu
//   Category   - Category grouping in the context menu
//   DomainName - Domain to register with (e.g. "Material"). Created if it doesn't exist.
//   Color      - Node header color as glm::vec4
void RegisterExternalGraphNode(uint32_t typeId, const char* typeName, const char* category, const char* domainName, const glm::vec4& color);

#define REGISTER_GRAPH_NODE(Class, TypeName, Category, DomainName, Color) \
    namespace { struct AutoRegGraphNode_##Class { AutoRegGraphNode_##Class() { \
        RegisterExternalGraphNode(OctHashString(#Class), TypeName, Category, DomainName, Color); \
    }} s_autoRegGraphNode_##Class; }

// Register a custom graph node type with multiple domains at once.
// Place this in your .cpp file after DEFINE_GRAPH_NODE.
//
// Usage:
//   REGISTER_GRAPH_NODE_MULTI(MyNode, "My Node", "Custom", glm::vec4(0.8f, 0.4f, 0.1f, 1.0f), "Material", "Shader");
//
// Parameters:
//   Class      - The node class name (must match DEFINE_GRAPH_NODE)
//   TypeName   - Display name shown in the context menu
//   Category   - Category grouping in the context menu
//   Color      - Node header color as glm::vec4
//   ...        - One or more domain name strings
void RegisterExternalGraphNodeMulti(uint32_t typeId, const char* typeName, const char* category,
    const char** domainNames, uint32_t domainCount, const glm::vec4& color);

#define REGISTER_GRAPH_NODE_MULTI(Class, TypeName, Category, Color, ...) \
    namespace { struct AutoRegGraphNodeMulti_##Class { AutoRegGraphNodeMulti_##Class() { \
        static const char* sDomains[] = { __VA_ARGS__ }; \
        RegisterExternalGraphNodeMulti(OctHashString(#Class), TypeName, Category, \
            sDomains, sizeof(sDomains)/sizeof(sDomains[0]), Color); \
    }} s_autoRegGraphNodeMulti_##Class; }

class GraphNode : public Object
{
public:

    DECLARE_FACTORY_MANAGER(GraphNode);
    DECLARE_FACTORY(GraphNode, GraphNode);
    DECLARE_OBJECT(GraphNode, Object);

    GraphNode();
    virtual ~GraphNode();

    virtual void SetupPins();
    virtual void Evaluate();

    virtual const char* GetNodeTypeName() const { return "Node"; }
    virtual const char* GetNodeCategory() const { return "General"; }
    virtual glm::vec4 GetNodeColor() const;

    virtual bool IsInputNode() const { return false; }
    virtual const std::string& GetInputName() const;
    virtual void SetInputName(const std::string& name) {}

    virtual void SaveStream(Stream& stream);
    virtual void LoadStream(Stream& stream, uint32_t version);

    GraphPin& AddInputPin(const char* name, DatumType type, const Datum& defaultValue = Datum());
    GraphPin& AddOutputPin(const char* name, DatumType type);

    GraphPinId GetInputPinId(uint32_t index) const;
    GraphPinId GetOutputPinId(uint32_t index) const;
    GraphPin* GetInputPin(uint32_t index);
    GraphPin* GetOutputPin(uint32_t index);
    const Datum& GetInputValue(uint32_t index) const;
    void SetOutputValue(uint32_t index, const Datum& value);

    uint32_t GetNumInputPins() const { return (uint32_t)mInputPins.size(); }
    uint32_t GetNumOutputPins() const { return (uint32_t)mOutputPins.size(); }

    const std::vector<GraphPin>& GetInputPins() const { return mInputPins; }
    const std::vector<GraphPin>& GetOutputPins() const { return mOutputPins; }
    std::vector<GraphPin>& GetInputPins() { return mInputPins; }
    std::vector<GraphPin>& GetOutputPins() { return mOutputPins; }

    GraphNodeId GetId() const { return mId; }
    void SetId(GraphNodeId id) { mId = id; }

    const glm::vec2& GetEditorPosition() const { return mEditorPosition; }
    void SetEditorPosition(const glm::vec2& pos) { mEditorPosition = pos; }

protected:

    GraphNodeId mId = INVALID_GRAPH_NODE_ID;
    std::vector<GraphPin> mInputPins;
    std::vector<GraphPin> mOutputPins;
    glm::vec2 mEditorPosition = glm::vec2(0.0f);

    GraphPinId mNextPinId = 1;
};
