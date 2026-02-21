#include "NodeGraph/GraphDomainManager.h"
#include "NodeGraph/GraphDomain.h"
#include "Log.h"
#include "Maths.h"

#include <cstring>
#include <string>

// ---------------------------------------------------------------------------
// GenericDomain - auto-created for external registrations targeting unknown domains
// ---------------------------------------------------------------------------
class GenericDomain : public GraphDomain
{
public:
    GenericDomain(const char* name) : mName(name) {}
    const char* GetDomainName() const override { return mName.c_str(); }
    void RegisterNodeTypes() override {} // Nodes added externally
private:
    std::string mName;
};

// ---------------------------------------------------------------------------
// External node registration storage
// ---------------------------------------------------------------------------
struct ExternalNodeRegistration
{
    uint32_t mTypeId;
    std::string mTypeName;
    std::string mCategory;
    std::string mDomainName;
    glm::vec4 mColor;
};

static std::vector<ExternalNodeRegistration>& GetPendingRegistrations()
{
    static std::vector<ExternalNodeRegistration> sRegistrations;
    return sRegistrations;
}

void RegisterExternalGraphNode(uint32_t typeId, const char* typeName, const char* category, const char* domainName, const glm::vec4& color)
{
    ExternalNodeRegistration reg;
    reg.mTypeId = typeId;
    reg.mTypeName = typeName;
    reg.mCategory = category;
    reg.mDomainName = domainName;
    reg.mColor = color;
    GetPendingRegistrations().push_back(reg);
}

void RegisterExternalGraphNodeMulti(uint32_t typeId, const char* typeName, const char* category,
    const char** domainNames, uint32_t domainCount, const glm::vec4& color)
{
    for (uint32_t i = 0; i < domainCount; ++i)
    {
        RegisterExternalGraphNode(typeId, typeName, category, domainNames[i], color);
    }
}

// ---------------------------------------------------------------------------
GraphDomainManager* GraphDomainManager::sInstance = nullptr;

void GraphDomainManager::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new GraphDomainManager();
    }
}

void GraphDomainManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

GraphDomainManager* GraphDomainManager::Get()
{
    return sInstance;
}

GraphDomainManager::GraphDomainManager()
{
}

GraphDomainManager::~GraphDomainManager()
{
    for (uint32_t i = 0; i < mDomains.size(); ++i)
    {
        delete mDomains[i];
    }
    mDomains.clear();
}

void GraphDomainManager::RegisterDomain(GraphDomain* domain)
{
    if (domain != nullptr)
    {
        domain->RegisterNodeTypes();
        mDomains.push_back(domain);
    }
}

GraphDomain* GraphDomainManager::GetDomain(const char* name) const
{
    for (uint32_t i = 0; i < mDomains.size(); ++i)
    {
        if (strncmp(mDomains[i]->GetDomainName(), name, 256) == 0)
        {
            return mDomains[i];
        }
    }
    return nullptr;
}

GraphDomain* GraphDomainManager::GetOrCreateDomain(const char* name)
{
    GraphDomain* domain = GetDomain(name);
    if (domain == nullptr)
    {
        domain = new GenericDomain(name);
        mDomains.push_back(domain);
    }
    return domain;
}

void GraphDomainManager::ProcessExternalRegistrations()
{
    std::vector<ExternalNodeRegistration>& regs = GetPendingRegistrations();

    for (uint32_t i = 0; i < regs.size(); ++i)
    {
        GraphDomain* domain = GetOrCreateDomain(regs[i].mDomainName.c_str());
        domain->AddNodeType(regs[i].mTypeId, regs[i].mTypeName.c_str(), regs[i].mCategory.c_str(), regs[i].mColor);
    }

    regs.clear();
}
