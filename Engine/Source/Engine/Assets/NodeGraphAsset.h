#pragma once

#include "Asset.h"
#include "Factory.h"
#include "NodeGraph/NodeGraph.h"

#include <vector>
#include <string>

class NodeGraphAsset : public Asset
{
public:

    DECLARE_ASSET(NodeGraphAsset, Asset);

    NodeGraphAsset();
    ~NodeGraphAsset();

    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual void Create() override;
    virtual void Destroy() override;

    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;

    NodeGraph& GetGraph() { return mGraph; }
    const NodeGraph& GetGraph() const { return mGraph; }

    // Function graphs
    NodeGraph* AddFunctionGraph(const std::string& name);
    void RemoveFunctionGraph(uint32_t index);
    NodeGraph* FindFunctionGraph(const std::string& name) const;
    uint32_t GetNumFunctionGraphs() const { return (uint32_t)mFunctionGraphs.size(); }
    NodeGraph* GetFunctionGraph(uint32_t index);
    const std::vector<NodeGraph*>& GetFunctionGraphs() const { return mFunctionGraphs; }
    void RenameFunctionGraph(uint32_t index, const std::string& newName);
    void ResolveFunctionCallNodes();

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    NodeGraph mGraph;
    std::vector<NodeGraph*> mFunctionGraphs;
    int32_t mDomainIndex = 0;
};
