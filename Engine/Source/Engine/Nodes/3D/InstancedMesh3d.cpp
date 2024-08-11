#include "Nodes/3D/InstancedMesh3d.h"

FORCE_LINK_DEF(InstancedMesh3D);
DEFINE_NODE(InstancedMesh3D, StaticMesh3D);

InstancedMesh3D::InstancedMesh3D()
{
    mName = "Instanced Mesh";
}

InstancedMesh3D::~InstancedMesh3D()
{

}

const char* InstancedMesh3D::GetTypeName() const
{
    return "InstancedMesh";
}

void InstancedMesh3D::GatherProperties(std::vector<Property>& outProps)
{
    StaticMesh3D::GatherProperties(outProps);
}

void InstancedMesh3D::Create()
{
    StaticMesh3D::Create();
}

void InstancedMesh3D::Destroy()
{
    StaticMesh3D::Destroy();
}

void InstancedMesh3D::Render()
{
    GFX_DrawInstancedMeshComp(this);
}

void InstancedMesh3D::SaveStream(Stream& stream)
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

void InstancedMesh3D::LoadStream(Stream& stream)
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

bool InstancedMesh3D::IsInstancedMesh3D() const
{
    return true;
}

Bounds InstancedMesh3D::GetLocalBounds() const
{
    LogError("Implement InstancedMesh3D::GetLocalBounds()");
    return Bounds();
}

void InstancedMesh3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    LogError("Implement InstancedMesh3D::GatherProxyDraws()");
}

uint32_t InstancedMesh3D::GetNumInstances() const
{
    return uint32_t(mInstanceData.size());
}

const MeshInstanceData& InstancedMesh3D::GetInstanceData(int32_t index) const
{
    OCT_ASSERT(index >= 0 && index < int32_t(mInstanceData.size()));
    return mInstanceData[index];
}

const std::vector<MeshInstanceData>& InstancedMesh3D::GetInstanceData() const
{
    return mInstanceData;
}

void InstancedMesh3D::SetInstanceData(int32_t index, const MeshInstanceData& data)
{
    OCT_ASSERT(index >= 0 && index < int32_t(mInstanceData.size()));
    if (index >= 0 &&
        index < int32_t(mInstanceData.size()))
    {
        mInstanceData[index] = data;
        MarkInstanceDataDirty();
    }
}

void InstancedMesh3D::SetInstanceData(const std::vector<MeshInstanceData>& data)
{
    mInstanceData = data;
    MarkInstanceDataDirty();
}

void InstancedMesh3D::AddInstanceData(const MeshInstanceData& data)
{
    mInstanceData.push_back(data);
    MarkInstanceDataDirty();
}

void InstancedMesh3D::RemoveInstanceData(int32_t index)
{
    if (index == -1 &&
        mInstanceData.size() > 0)
    {
        mInstanceData.pop_back();
        MarkInstanceDataDirty();
    }
    else if (index >= 0 && index < int32_t(mInstanceData.size()))
    {
        mInstanceData.erase(mInstanceData.begin() + index);
        MarkInstanceDataDirty();
    }
}

uint32_t InstancedMesh3D::GetTotalVertexCount() const
{
    StaticMesh* mesh = mStaticMesh.Get<StaticMesh>();
    uint32_t meshVertices = mesh ? mesh->GetNumVertices() : 0;
    uint32_t numInstances = GetNumInstances();
    uint32_t totalVertices = numInstances * meshVertices;

    return totalVertices;
}

bool InstancedMesh3D::IsInstanceDataDirty() const
{
    return mInstanceDataDirty;
}

void InstancedMesh3D::MarkInstanceDataDirty()
{
    mInstanceDataDirty = true;
    mInstancedMeshResource.mDirty = true;
}

void InstancedMesh3D::UpdateInstanceData()
{
    RecreateCollisionShape();
}

InstancedMeshCompResource* InstancedMesh3D::GetInstancedMeshResource()
{
    return &mInstancedMeshResource;
}

void InstancedMesh3D::RecreateCollisionShape()
{
    LogError("Implement InstancedMesh3D::RecreateCollisionShape()");
}
