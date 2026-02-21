#pragma once

#include "NodeGraph/GraphNode.h"

#include <string>

static const glm::vec4 kInputNodeColor = glm::vec4(0.9f, 0.6f, 0.1f, 1.0f);

// --- Float Input ---
class FloatInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(FloatInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Float Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Float";
};

// --- Int Input ---
class IntInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(IntInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Int Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Integer";
};

// --- Bool Input ---
class BoolInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(BoolInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Bool Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Bool";
};

// --- String Input ---
class StringInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(StringInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "String Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "String";
};

// --- Byte Input ---
class ByteInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ByteInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Byte Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Byte";
};

// --- Asset Input ---
class AssetInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(AssetInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Asset Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Asset";
};

// --- Vector Input ---
class VectorInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(VectorInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Vector Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Vector";
};

// --- Color Input ---
class ColorInputNode : public GraphNode
{
public:
    DECLARE_GRAPH_NODE(ColorInputNode, GraphNode);
    virtual void SetupPins() override;
    virtual void Evaluate() override;
    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream, uint32_t version) override;
    virtual const char* GetNodeTypeName() const override { return "Color Input"; }
    virtual const char* GetNodeCategory() const override { return "Input"; }
    virtual glm::vec4 GetNodeColor() const override { return kInputNodeColor; }
    virtual bool IsInputNode() const override { return true; }
    virtual const std::string& GetInputName() const override { return mInputName; }
    virtual void SetInputName(const std::string& name) override { mInputName = name; }

protected:
    std::string mInputName = "Color";
};
