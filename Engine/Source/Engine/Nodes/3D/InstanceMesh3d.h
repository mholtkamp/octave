#pragma once

#include "Nodes/3D/StaticMesh3d.h"

struct MeshInstanceData
{
    glm::vec3 mPosition = {};
    glm::vec3 mRotation = {};
    glm::vec3 mScale = {};
};

class InstanceMesh3D : public StaticMesh3D
{
public:

    DECLARE_NODE(InstanceMesh3D, StaticMesh3D);

    InstanceMesh3D();
    ~InstanceMesh3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;

    virtual void SaveStream(Stream& stream) override;
    virtual void LoadStream(Stream& stream) override;

    virtual bool IsInstanceMesh3D() const override;

    virtual Bounds GetLocalBounds() const override;

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws) override;

    uint32_t GetNumInstances() const;
    const MeshInstanceData& GetInstanceData(int32_t index) const;
    MeshInstanceData& GetInstanceData(int32_t index);
    void SetInstanceData(int32_t index, const MeshInstanceData& data);
    void SetInstanceData(const std::vector<MeshInstanceData>& data);
    void AddInstanceData(const MeshInstanceData& data);
    void RemoveInstanceData(int32_t index);

    uint32_t GetTotalVertexCount() const;

protected:

    virtual void RecreateCollisionShape() override;


    std::vector<MeshInstanceData> mInstanceData;

};
