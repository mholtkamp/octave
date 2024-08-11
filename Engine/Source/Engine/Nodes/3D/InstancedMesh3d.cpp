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

void InstancedMesh3D::Tick(float deltaTime)
{
    StaticMesh3D::Tick(deltaTime);
    UpdateInstanceData();
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

void InstancedMesh3D::SetStaticMesh(StaticMesh* staticMesh)
{
    if (mStaticMesh.Get() != staticMesh)
    {
        StaticMesh3D::SetStaticMesh(staticMesh);
        MarkInstanceDataDirty();
    }
}

Bounds InstancedMesh3D::GetLocalBounds() const
{
    if (mInstanceDataDirty)
    {
        const_cast<InstancedMesh3D*>(this)->UpdateInstanceData();
    }

    return mBounds;
}

void InstancedMesh3D::GatherProxyDraws(std::vector<DebugDraw>& inoutDraws)
{
    StaticMesh3D::GatherProxyDraws(inoutDraws);
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
    if (mInstanceDataDirty)
    {
        RecreateCollisionShape();
        CalculateLocalBounds();

        mInstanceDataDirty = false;
    }
}

InstancedMeshCompResource* InstancedMesh3D::GetInstancedMeshResource()
{
    return &mInstancedMeshResource;
}

glm::mat4 InstancedMesh3D::CalculateInstanceTransform(int32_t instanceIndex)
{
    glm::mat4 transform = glm::mat4(1);

    if (instanceIndex >= 0 &&
        instanceIndex < int32_t(mInstanceData.size()))
    {
        const MeshInstanceData& instData = mInstanceData[instanceIndex];

        glm::quat rotQuat = glm::quat(instData.mRotation * DEGREES_TO_RADIANS);
        float scaleX = instData.mScale.x;
        glm::vec3 scale = glm::vec3(scaleX, scaleX, scaleX);

        transform = glm::translate(transform, instData.mPosition);
        transform *= glm::toMat4(rotQuat);
        transform = glm::scale(transform, scale);
    }

    return transform;
}

void InstancedMesh3D::RecreateCollisionShape()
{
    StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

    if (staticMesh != nullptr && mInstanceData.size() > 0)
    {
        btCompoundShape* compoundShape = new btCompoundShape();

        for (uint32_t i = 0; i < mInstanceData.size(); ++i)
        {
            glm::quat rotQuat = glm::quat(mInstanceData[i].mRotation * DEGREES_TO_RADIANS);
            glm::vec3 pos = mInstanceData[i].mPosition;

            btQuaternion bRotation = btQuaternion(rotQuat.x, rotQuat.y, rotQuat.z, rotQuat.w);
            btVector3 bPosition = { pos.x, pos.y, pos.z };
            btTransform bTransform = btTransform(bRotation, bPosition);

            if (mUseTriangleCollision && staticMesh->GetTriangleCollisionShape())
            {
                glm::vec3 scale = GetWorldScale();
                // Instances can only have uniform scale for now (based on X component)
                btVector3 btscale = btVector3(scale.x, scale.x, scale.x);
                btScaledBvhTriangleMeshShape* scaledTriangleShape = new btScaledBvhTriangleMeshShape(staticMesh->GetTriangleCollisionShape(), btscale);
                
                compoundShape->addChildShape(bTransform, scaledTriangleShape);
            }
            else
            {
                btCollisionShape* newShape = CloneCollisionShape(staticMesh->GetCollisionShape());
                compoundShape->addChildShape(bTransform, newShape);
            }
        }

        SetCollisionShape(compoundShape);
    }
    else
    {
        SetCollisionShape(Primitive3D::GetEmptyCollisionShape());
    }
}

void InstancedMesh3D::CalculateLocalBounds()
{
    StaticMesh* mesh = GetStaticMesh();

    if (mesh != nullptr && mInstanceData.size() > 0)
    {
        // TODO: This algorithm can be improved.
        Bounds meshBounds = mesh->GetBounds();

        std::vector<glm::vec3> instBoundsCenters;
        instBoundsCenters.resize(mInstanceData.size());

        // Determine center position (every instance weighted equally)
        glm::vec3 sumPosition = {};
        for (uint32_t i = 0; i < mInstanceData.size(); ++i)
        {
            glm::mat4 transform = CalculateInstanceTransform(i);
            instBoundsCenters[i] = transform * glm::vec4(meshBounds.mCenter, 1.0f);
            sumPosition += instBoundsCenters[i];
        }

        glm::vec3 centerPosition = sumPosition / float(mInstanceData.size());

        // Determine farthest possible position from center
        float maxDistance = 0.0f;
        for (uint32_t i = 0; i < mInstanceData.size(); ++i)
        {
            float distFromCenter = glm::distance(instBoundsCenters[i], centerPosition);

            // Right now, we only use x component scale for a uniform scale factor, but I'm leaving this code 
            // here for the future in case we allow non-uniform scale.
            float maxScale = glm::max(mInstanceData[i].mScale.x, mInstanceData[i].mScale.y);
            maxScale = glm::max(maxScale, mInstanceData[i].mScale.z);

            distFromCenter += meshBounds.mRadius * maxScale;

            if (distFromCenter > maxDistance)
            {
                maxDistance = distFromCenter;
            }
        }

        mBounds.mCenter = centerPosition;
        mBounds.mRadius = maxDistance;
    }
    else
    {
        mBounds.mCenter = GetWorldPosition();
        mBounds.mRadius = 0.0f;
    }
}
