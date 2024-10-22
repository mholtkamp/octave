#include "Nodes/3D/InstancedMesh3d.h"
#include "Assets/StaticMesh.h"

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

    outProps.push_back(Property(DatumType::Float, "Unrolled Cull Distance", this, &mUnrolledCullDistance));
    outProps.push_back(Property(DatumType::Float, "Unrolled Cell Size", this, &mUnrolledCellSize));
    outProps.push_back(Property(DatumType::Bool, "Always Unroll", this, &mAlwaysUnroll));
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
    if (!mUnrolled)
    {
        GFX_DrawInstancedMeshComp(this);
    }
}

void InstancedMesh3D::Tick(float deltaTime)
{
    StaticMesh3D::Tick(deltaTime);
    mInstanceDataUpdatedThisFrame = false;
    UpdateInstanceData();
}

void InstancedMesh3D::EditorTick(float deltaTime)
{
    StaticMesh3D::EditorTick(deltaTime);
    mInstanceDataUpdatedThisFrame = false;
    UpdateInstanceData();
}

void InstancedMesh3D::SaveStream(Stream& stream, Platform platform)
{
    StaticMesh3D::SaveStream(stream, platform);

    stream.WriteUint32((uint32_t)mInstanceData.size());
    for (uint32_t i = 0; i < mInstanceData.size(); ++i)
    {
        stream.WriteVec3(mInstanceData[i].mPosition);
        stream.WriteVec3(mInstanceData[i].mRotation);
        stream.WriteVec3(mInstanceData[i].mScale);
    }
}

void InstancedMesh3D::LoadStream(Stream& stream, Platform platform, uint32_t version)
{
    StaticMesh3D::LoadStream(stream, platform, version);

    uint32_t numInstances = stream.ReadUint32();
    mInstanceData.resize(numInstances);
    for (uint32_t i = 0; i < numInstances; ++i)
    {
        mInstanceData[i].mPosition = stream.ReadVec3();
        mInstanceData[i].mRotation = stream.ReadVec3();
        mInstanceData[i].mScale = stream.ReadVec3();
    }

    if (ShouldUnroll())
    {
        Unroll();
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

void InstancedMesh3D::AddInstanceData(const MeshInstanceData& data, int32_t index)
{
    if (index < 0)
    {
        mInstanceData.push_back(data);
    }
    else
    {
        if (index >= 0 &&
            index <= mInstanceData.size())
        {
            mInstanceData.insert(mInstanceData.begin() + index, data);
        }
        else
        {
            LogError("Out of bounds insertion index in AddInstanceData");
        }
    }
    
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
        mInstanceDataUpdatedThisFrame = true;
    }
}

bool InstancedMesh3D::WasInstanceDataUpdatedThisFrame() const
{
    return mInstanceDataUpdatedThisFrame;
}

bool InstancedMesh3D::ShouldUnroll() const
{
    Platform platform = GetPlatform();

#if EDITOR
    if (!IsPlayingInEditor())
    {
        // In editor, never unroll.
        return false;
    }
#endif

    if (mAlwaysUnroll)
    {
        return true;
    }

    if (platform == Platform::Windows ||
        platform == Platform::Linux ||
        platform == Platform::Android)
    {
        return false;
    }

    return true;
}

InstancedMeshCompResource* InstancedMesh3D::GetInstancedMeshResource()
{
    return &mInstancedMeshResource;
}

btTransform InstancedMesh3D::CalculateInstanceBulletTransform(int32_t instanceIndex)
{
    btTransform retTransform;

    if (instanceIndex >= 0 &&
        instanceIndex < int32_t(mInstanceData.size()))
    {
        retTransform = MakeBulletTransform(mInstanceData[instanceIndex].mPosition, mInstanceData[instanceIndex].mRotation);
    }

    return retTransform;
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

Bounds InstancedMesh3D::CalculateInstanceBounds(int32_t instanceIndex)
{
    Bounds retBounds;

    if (instanceIndex >= 0 &&
        instanceIndex < int32_t(mInstanceData.size()))
    {
        glm::mat4 instTransform = MakeTransform(
            mInstanceData[instanceIndex].mPosition,
            mInstanceData[instanceIndex].mRotation,
            mInstanceData[instanceIndex].mScale);

        retBounds.mCenter = GetTransform() * instTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        retBounds.mRadius = glm::vec3(GetTransform() * instTransform * glm::vec4(1.0f, 1.0f, 1.0f, 0.0f)).x;
    }

    return retBounds;
}

btCompoundShape* InstancedMesh3D::GeneratePaintCollisionShape()
{
    btCompoundShape* compoundShape = nullptr;

    StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

    if (staticMesh != nullptr &&
        mInstanceData.size() > 0)
    {
        compoundShape = new btCompoundShape();

        for (uint32_t i = 0; i < mInstanceData.size(); ++i)
        {
            // Add sphere children!
            glm::mat4 instTransform = MakeTransform(
                mInstanceData[i].mPosition,
                mInstanceData[i].mRotation,
                mInstanceData[i].mScale);

            float scale = staticMesh->GetBounds().mRadius * mInstanceData[i].mScale.x;
            glm::vec3 center = instTransform * glm::vec4(staticMesh->GetBounds().mCenter * scale, 1.0f);
            btSphereShape* sphereShape = new btSphereShape(scale);
            btTransform transform = MakeBulletTransform(center, {0.0f, 0.0f, 0.0f});
            compoundShape->addChildShape(transform, sphereShape);
        }

        float nodeScale = GetWorldScale().x;
        compoundShape->setLocalScaling(btVector3(nodeScale, nodeScale, nodeScale));
    }

    return compoundShape;
}

btCompoundShape* InstancedMesh3D::GenerateTriangleCollisionShape()
{
    btCompoundShape* compoundShape = nullptr;
    StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

    if (staticMesh != nullptr &&
        mInstanceData.size() > 0)
    {
        compoundShape = new btCompoundShape();

        for (uint32_t i = 0; i < mInstanceData.size(); ++i)
        {
            btTransform bTransform = CalculateInstanceBulletTransform(i);

            // Instances can only have uniform scale for now (based on X component)
            float scale = mInstanceData[i].mScale.x;
            btVector3 btscale = btVector3(scale, scale, scale);

            if (mUseTriangleCollision && staticMesh->GetTriangleCollisionShape())
            {
                btScaledBvhTriangleMeshShape* scaledTriangleShape = new btScaledBvhTriangleMeshShape(staticMesh->GetTriangleCollisionShape(), btscale);
                compoundShape->addChildShape(bTransform, scaledTriangleShape);
            }
            else
            {
                btCollisionShape* newShape = CloneCollisionShape(staticMesh->GetCollisionShape());
                newShape->setLocalScaling(btscale);
                compoundShape->addChildShape(bTransform, newShape);
            }
        }
    }

    return compoundShape;
}

void InstancedMesh3D::RecreateCollisionShape()
{
    StaticMesh* staticMesh = mStaticMesh.Get<StaticMesh>();

    if (staticMesh != nullptr && 
        mInstanceData.size() > 0 &&
        ((mUseTriangleCollision && staticMesh->GetTriangleCollisionShape()) ||
        (!mUseTriangleCollision && staticMesh->GetCollisionShape())))
    {
        btCompoundShape* compoundShape = GenerateTriangleCollisionShape();
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

void InstancedMesh3D::Unroll()
{
    if (!ShouldUnroll())
    {
        LogError("InstancedMesh3D::Unroll() called when it shouldn't be.");
        return;
    }

    if (mUnrolled)
    {
        LogError("Already unrolled mesh. Currently we only support unrolling once on load");
        return;
    }

    StaticMesh* mesh = GetStaticMesh();

    if (mesh->HasVertexColor())
    {
        LogError("Instanced Mesh unrolling only supports meshes without vertex color currently.");
    }

    if (mInstanceData.size() == 0 ||
        mesh == nullptr ||
        mesh->HasVertexColor())
        return;

    glm::vec3 minExt = mInstanceData[0].mPosition;
    glm::vec3 maxExt = minExt;

    // Find min/max extents (and thus the dimensions)
    for (uint32_t i = 1; i < mInstanceData.size(); ++i)
    {
        glm::vec3 pos = mInstanceData[i].mPosition;

        minExt = glm::min(minExt, pos);
        maxExt = glm::max(maxExt, pos);
    }

    glm::vec3 dim = maxExt - minExt;

    // Determine the total (maximum) numbers of cells in X/Z direction and resize vector
    uint32_t numCellsX = uint32_t(dim.x / mUnrolledCellSize) + 1;
    uint32_t numCellsZ = uint32_t(dim.z / mUnrolledCellSize) + 1;
    uint32_t numCells = numCellsX * numCellsZ;

    std::vector<std::vector<Vertex>> unrolledVertexData;
    std::vector<std::vector<IndexType>> unrolledIndexData;
    unrolledVertexData.resize(numCells);
    unrolledIndexData.resize(numCells);

    uint32_t numVertices = mesh->GetNumVertices();
    uint32_t numIndices = mesh->GetNumIndices();
    Vertex* srcVertexData = mesh->GetVertices();
    IndexType* srcIndexData = mesh->GetIndices();

    // Then iterate over all instances, determine which cell it lies in, and then add it to that cell data
    for (uint32_t i = 0; i < mInstanceData.size(); ++i)
    {
        glm::vec3 pos = mInstanceData[i].mPosition;
        glm::vec3 rot = mInstanceData[i].mRotation;
        glm::vec3 scale = mInstanceData[i].mScale;

        uint32_t x = uint32_t((pos.x - minExt.x) / mUnrolledCellSize);
        uint32_t z = uint32_t((pos.z - minExt.z) / mUnrolledCellSize);

        OCT_ASSERT(x < numCellsX);
        OCT_ASSERT(z < numCellsZ);

        std::vector<Vertex>& dstVertexData = unrolledVertexData[z * numCellsX + x];
        std::vector<IndexType>& dstIndexData = unrolledIndexData[z * numCellsX + x];

        // We need to offset all of the index data for this new mesh instance data
        uint32_t indexOffset = (uint32_t)dstVertexData.size();

        if (indexOffset + numIndices > MAX_MESH_VERTEX_COUNT)
        {
            LogWarning("Failing to unroll instances because vertex count has been exceeded.");
            continue;
        }

        const glm::mat4 instTransform = MakeTransform(pos, rot, scale);

        for (uint32_t v = 0; v < numVertices; ++v)
        {
            dstVertexData.push_back(srcVertexData[v]);
            Vertex& vert = dstVertexData.back();
            vert.mPosition = instTransform * glm::vec4(vert.mPosition, 1);
            vert.mNormal = instTransform * glm::vec4(vert.mNormal, 0);
        }

        for (uint32_t x = 0; x < numIndices; ++x)
        {
            dstIndexData.push_back(IndexType(srcIndexData[x] + indexOffset));
        }
    }

    // Iterate over all cell datas and remove ones that have 0 instances
    uint32_t numUnrolledCells = 0;
    for (uint32_t i = 0; i < unrolledVertexData.size(); ++i)
    {
        if (unrolledVertexData[i].size() == 0)
            continue;

        OCT_ASSERT(unrolledIndexData[i].size() > 0);

        StaticMesh* unrolledMesh = NewTransientAsset<StaticMesh>();
        unrolledMesh->SetName("Unrolled Mesh");
        unrolledMesh->SetMaterial(GetMaterial());
        unrolledMesh->CreateRaw(
            (uint32_t)unrolledVertexData[i].size(),
            unrolledVertexData[i].data(),
            (uint32_t)unrolledIndexData[i].size(),
            unrolledIndexData[i].data());

        char nodeName[64];
        snprintf(nodeName, 64, "Unrolled %d", numUnrolledCells);
        StaticMesh3D* cellNode = CreateChild<StaticMesh3D>(nodeName);
        cellNode->SetStaticMesh(unrolledMesh);
        cellNode->SetCullDistance(mUnrolledCullDistance);

        ++numUnrolledCells;
    }

    mUnrolled = true;
}
