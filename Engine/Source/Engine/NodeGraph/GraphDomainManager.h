#pragma once

#include <vector>

class GraphDomain;

class GraphDomainManager
{
public:

    static void Create();
    static void Destroy();
    static GraphDomainManager* Get();

    void RegisterDomain(GraphDomain* domain);
    GraphDomain* GetDomain(const char* name) const;
    GraphDomain* GetOrCreateDomain(const char* name);
    const std::vector<GraphDomain*>& GetDomains() const { return mDomains; }

    // Process node types registered via REGISTER_GRAPH_NODE macros
    void ProcessExternalRegistrations();

private:

    GraphDomainManager();
    ~GraphDomainManager();

    std::vector<GraphDomain*> mDomains;

    static GraphDomainManager* sInstance;
};
