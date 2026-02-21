#pragma once

#include "Asset.h"
#include "Factory.h"
#include "NodeGraph/NodeGraph.h"

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

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

protected:

    NodeGraph mGraph;
    int32_t mDomainIndex = 0;
};
