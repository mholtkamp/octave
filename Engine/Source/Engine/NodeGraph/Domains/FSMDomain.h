#pragma once

#include "NodeGraph/GraphDomain.h"

class FSMDomain : public GraphDomain
{
public:

    virtual const char* GetDomainName() const override { return "FSM"; }
    virtual void RegisterNodeTypes() override;
    virtual void OnGraphEvaluated(NodeGraph* graph) override;
    virtual TypeId GetDefaultOutputNodeType() const override;
};
