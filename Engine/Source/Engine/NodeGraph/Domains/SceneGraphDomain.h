#pragma once

#include "NodeGraph/GraphDomain.h"

class SceneGraphDomain : public GraphDomain
{
public:

    virtual const char* GetDomainName() const override { return "SceneGraph"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
