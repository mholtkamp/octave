#pragma once

#include "Nodes/3D/StaticMesh3d.h"

struct MeshInstanceData
{
    glm::vec3 mPosition = {0.0f, 0.0f, 0.0f};
    glm::vec3 mRotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 mScale = {1.0f, 1.0f, 1.0f};
};

class InstancedMesh3D : public StaticMesh3D
{
public:

    DECLARE_NODE(InstancedMesh3D, StaticMesh3D);

    InstancedMesh3D();
    ~InstancedMesh3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Render() override;
    virtual void Tick(float deltaTime) override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual bool IsInstancedMesh3D() const override;

    virtual void SetStaticMesh(StaticMesh* staticMesh);

    virtual Bounds GetLocalBounds() const override;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    uint32_t GetNumInstances() const;
    const MeshInstanceData& GetInstanceData(int32_t index) const;
    const std::vector<MeshInstanceData>& GetInstanceData() const;
    void SetInstanceData(int32_t index, const MeshInstanceData& data);
    void SetInstanceData(const std::vector<MeshInstanceData>& data);
    void AddInstanceData(const MeshInstanceData& data, int32_t index = -1);
    void RemoveInstanceData(int32_t index);

    uint32_t GetTotalVertexCount() const;

    bool IsInstanceDataDirty() const;
    void MarkInstanceDataDirty();
    void UpdateInstanceData();

    InstancedMeshCompResource* GetInstancedMeshResource();

    glm::mat4 CalculateInstanceTransform(int32_t instanceIndex);

protected:

    virtual void RecreateCollisionShape() override;
    void CalculateLocalBounds();

    std::vector<MeshInstanceData> mInstanceData;
    bool mInstanceDataDirty = true;
    Bounds mBounds;

    InstancedMeshCompResource mInstancedMeshResource;
};
