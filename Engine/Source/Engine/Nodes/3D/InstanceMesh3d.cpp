#include "Nodes/3D/InstanceMesh3d.h"

FORCE_LINK_DEF(InstanceMesh3D);
DEFINE_NODE(InstanceMesh3D, StaticMesh3D);

InstanceMesh3D::InstanceMesh3D()
{
    mName = "Instance Mesh";
}

InstanceMesh3D::~InstanceMesh3D()
{

}

const char* InstanceMesh3D::GetTypeName() const
{
    return "InstanceMesh";
}

void InstanceMesh3D::GatherProperties(std::vector<Property>& outProps)
{
    StaticMesh3D::GatherProperties(outProps);
}

void InstanceMesh3D::Create()
{
    StaticMesh3D::Create();
}

void InstanceMesh3D::Destroy()
{
    StaticMesh3D::Destroy();
}

void InstanceMesh3D::SaveStream(Stream& stream)
{
    StaticMesh3D::SaveStream(stream);

    stream.WriteUint32((uint32_t)mInstanceData.size());
    for (uint32_t i = 0; i < mInstanceData.size(); ++i)
    {
        stream.WriteVec3(mInstanceData[i].mPosition);
        stream.WriteVec3(mInstanceData[i].mRotation);
        stream.WriteVec3(mInstanceData[i].mScale);
    }
}

void InstanceMesh3D::LoadStream(Stream& stream)
{
    StaticMesh3D::LoadStream(stream);
    
    uint32_t numInstances = stream.ReadUint32();
    mInstanceData.resize(numInstances);
    for (uint32_t i = 0; i < numInstances; ++i)
    {
        mInstanceData[i].mPosition = stream.ReadVec3();
        mInstanceData[i].mRotation = stream.ReadVec3();
        mInstanceData[i].mScale = stream.ReadVec3();
    }
}

bool InstanceMesh3D::IsInstanceMesh3D() const
{
    return true;
}

Bounds InstanceMesh3D::GetLocalBounds() const
{
    LogError("Implement InstanceMesh3D::GetLocalBounds()");
    return Bounds();
}

void InstanceMesh3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    LogError("Implement InstanceMesh3D::GatherProxyDraws()");
}

uint32_t InstanceMesh3D::GetNumInstances() const
{
    return uint32_t(mInstanceData.size());
}

const MeshInstanceData& InstanceMesh3D::GetInstanceData(int32_t index) const
{
    OCT_ASSERT(index >= 0 && index < int32_t(mInstanceData.size()));
    return mInstanceData[index];
}

MeshInstanceData& InstanceMesh3D::GetInstanceData(int32_t index)
{
    OCT_ASSERT(index >= 0 && index < int32_t(mInstanceData.size()));
    return mInstanceData[index];
}

void InstanceMesh3D::SetInstanceData(int32_t index, const MeshInstanceData& data)
{
    OCT_ASSERT(index >= 0 && index < int32_t(mInstanceData.size()));
    if (index >= 0 &&
        index < int32_t(mInstanceData.size()))
    {
        mInstanceData[index] = data;
        RecreateCollisionShape();
    }
}

void InstanceMesh3D::SetInstanceData(const std::vector<MeshInstanceData>& data)
{
    mInstanceData = data;
    RecreateCollisionShape();
}

void InstanceMesh3D::AddInstanceData(const MeshInstanceData& data)
{
    mInstanceData.push_back(data);
    RecreateCollisionShape();
}

void InstanceMesh3D::RemoveInstanceData(int32_t index)
{
    if (index == -1 &&
        mInstanceData.size() > 0)
    {
        mInstanceData.pop_back();
        RecreateCollisionShape();
    }
    else if (index >= 0 && index < int32_t(mInstanceData.size()))
    {
        mInstanceData.erase(mInstanceData.begin() + index);
        RecreateCollisionShape();
    }
}

uint32_t InstanceMesh3D::GetTotalVertexCount() const
{
    StaticMesh* mesh = mStaticMesh.Get<StaticMesh>();
    uint32_t meshVertices = mesh ? mesh->GetNumVertices() : 0;
    uint32_t numInstances = GetNumInstances();
    uint32_t totalVertices = numInstances * meshVertices;

    return totalVertices;
}

void InstanceMesh3D::RecreateCollisionShape()
{
    LogError("Implement InstanceMesh3D::RecreateCollisionShape()");
}
