#include "Nodes/NodeGraphPlayer.h"
#include "Assets/NodeGraphAsset.h"
#include "NodeGraph/GraphDomainManager.h"
#include "NodeGraph/GraphDomain.h"
#include "Log.h"
#include "Utilities.h"

FORCE_LINK_DEF(NodeGraphPlayer);
DEFINE_NODE(NodeGraphPlayer, Node);

NodeGraphPlayer::NodeGraphPlayer()
{
    mName = "NodeGraphPlayer";
}

NodeGraphPlayer::~NodeGraphPlayer()
{
}

void NodeGraphPlayer::Create()
{
    Node::Create();
}

void NodeGraphPlayer::Destroy()
{
    if (mRuntimeGraph != nullptr)
    {
        delete mRuntimeGraph;
        mRuntimeGraph = nullptr;
    }

    Node::Destroy();
}

void NodeGraphPlayer::Tick(float deltaTime)
{
    Node::Tick(deltaTime);

    if (mPlaying && !mPaused)
    {
        if (mRuntimeGraph == nullptr)
            return;

        mProcessor.Evaluate(mRuntimeGraph);

        GraphDomainManager* domainMgr = GraphDomainManager::Get();
        if (domainMgr != nullptr)
        {
            GraphDomain* domain = domainMgr->GetDomain(mRuntimeGraph->GetDomainName().c_str());
            if (domain != nullptr)
            {
                domain->OnGraphEvaluated(mRuntimeGraph);
            }
        }
    }
}

void NodeGraphPlayer::Start()
{
    Node::Start();

    if (mPlayOnStart)
    {
        Play();
    }
}

void NodeGraphPlayer::Stop()
{
    StopGraph();
    Node::Stop();
}

void NodeGraphPlayer::GatherProperties(std::vector<Property>& outProps)
{
    Node::GatherProperties(outProps);

    SCOPED_CATEGORY("NodeGraph");

    outProps.push_back(Property(DatumType::Asset, "Node Graph", this, &mNodeGraphAsset, 1, nullptr, int32_t(NodeGraphAsset::GetStaticType())));
    outProps.push_back(Property(DatumType::Bool, "Play On Start", this, &mPlayOnStart));
}

const char* NodeGraphPlayer::GetTypeName() const
{
    return "NodeGraphPlayer";
}

void NodeGraphPlayer::Play()
{
    NodeGraphAsset* asset = mNodeGraphAsset.Get<NodeGraphAsset>();
    if (asset == nullptr)
    {
        LogWarning("NodeGraphPlayer::Play() - No node graph asset assigned");
        return;
    }

    EnsureRuntimeGraph();

    mPlaying = true;
    mPaused = false;
}

void NodeGraphPlayer::Pause()
{
    mPaused = true;
}

void NodeGraphPlayer::StopGraph()
{
    mPlaying = false;
    mPaused = false;
}

void NodeGraphPlayer::Reset()
{
    if (mRuntimeGraph != nullptr)
    {
        delete mRuntimeGraph;
        mRuntimeGraph = nullptr;
    }

    mPlaying = false;
    mPaused = false;
}

bool NodeGraphPlayer::IsPlaying() const
{
    return mPlaying && !mPaused;
}

bool NodeGraphPlayer::IsPaused() const
{
    return mPaused;
}

void NodeGraphPlayer::SetNodeGraphAsset(NodeGraphAsset* asset)
{
    mNodeGraphAsset = asset;

    if (mRuntimeGraph != nullptr)
    {
        delete mRuntimeGraph;
        mRuntimeGraph = nullptr;
    }
}

NodeGraphAsset* NodeGraphPlayer::GetNodeGraphAsset() const
{
    return mNodeGraphAsset.Get<NodeGraphAsset>();
}

bool NodeGraphPlayer::SetInputFloat(const char* name, float value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputFloat(name, value) : false;
}

bool NodeGraphPlayer::SetInputInt(const char* name, int32_t value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputInt(name, value) : false;
}

bool NodeGraphPlayer::SetInputBool(const char* name, bool value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputBool(name, value) : false;
}

bool NodeGraphPlayer::SetInputString(const char* name, const char* value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputString(name, value) : false;
}

bool NodeGraphPlayer::SetInputVector(const char* name, const glm::vec3& value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputVector(name, value) : false;
}

bool NodeGraphPlayer::SetInputColor(const char* name, const glm::vec4& value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputColor(name, value) : false;
}

bool NodeGraphPlayer::SetInputByte(const char* name, uint8_t value)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputByte(name, value) : false;
}

bool NodeGraphPlayer::SetInputAsset(const char* name, Asset* asset)
{
    EnsureRuntimeGraph();
    return mRuntimeGraph ? mRuntimeGraph->SetInputAsset(name, asset) : false;
}

float NodeGraphPlayer::GetOutputFloat(uint32_t pinIndex) const
{
    if (mRuntimeGraph != nullptr)
    {
        return mRuntimeGraph->GetOutputValue(pinIndex).GetFloat();
    }
    return 0.0f;
}

int32_t NodeGraphPlayer::GetOutputInt(uint32_t pinIndex) const
{
    if (mRuntimeGraph != nullptr)
    {
        return mRuntimeGraph->GetOutputValue(pinIndex).GetInteger();
    }
    return 0;
}

bool NodeGraphPlayer::GetOutputBool(uint32_t pinIndex) const
{
    if (mRuntimeGraph != nullptr)
    {
        return mRuntimeGraph->GetOutputValue(pinIndex).GetBool();
    }
    return false;
}

glm::vec3 NodeGraphPlayer::GetOutputVector(uint32_t pinIndex) const
{
    if (mRuntimeGraph != nullptr)
    {
        return mRuntimeGraph->GetOutputValue(pinIndex).GetVector();
    }
    return glm::vec3(0.0f);
}

glm::vec4 NodeGraphPlayer::GetOutputColor(uint32_t pinIndex) const
{
    if (mRuntimeGraph != nullptr)
    {
        return mRuntimeGraph->GetOutputValue(pinIndex).GetColor();
    }
    return glm::vec4(0.0f);
}

void NodeGraphPlayer::EnsureRuntimeGraph()
{
    NodeGraphAsset* asset = mNodeGraphAsset.Get<NodeGraphAsset>();
    if (asset == nullptr)
        return;

    if (mRuntimeGraph == nullptr)
    {
        mRuntimeGraph = new NodeGraph();
        mRuntimeGraph->CopyFrom(asset->GetGraph());
    }
}
