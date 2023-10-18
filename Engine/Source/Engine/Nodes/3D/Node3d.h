#pragma once

#include "Nodes/Node.h"
#include "EngineTypes.h"

#include "Maths.h"

#include "AssetRef.h"

class SkeletalMesh3D;

class Node3D : public Node
{
public:

    DECLARE_NODE(Node3D, Node);

    Node3D();
    virtual ~Node3D();

    virtual void SaveStream(Stream& stream) override;
	virtual void LoadStream(Stream& stream) override;

    virtual void Create() override;
    virtual void Destroy() override;
    virtual void Tick(float deltaTime) override;

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual void GatherReplicatedData(std::vector<NetDatum>& outData);

    virtual bool IsNode3D() const override;

    void AttachToBone(SkeletalMesh3D* parent, const char* boneName, bool keepWorldTransform = false);
    void AttachToBone(SkeletalMesh3D* parent, int32_t boneIndex, bool keepWorldTransform = false);

    void MarkTransformDirty();
    bool IsTransformDirty() const;
    virtual void UpdateTransform(bool updateChildren);

    virtual void GatherProxyDraws(std::vector<DebugDraw>& inoutDraws);

    virtual bool IsTransient() const override;

    glm::vec3 GetPosition() const;
    glm::vec3 GetRotationEuler() const;
    glm::quat GetRotationQuat() const;
    glm::vec3 GetScale() const;

    glm::vec3& GetPositionRef();
    glm::vec3& GetRotationEulerRef();
    glm::quat& GetRotationQuatRef();
    glm::vec3& GetScaleRef();

    const glm::mat4& GetTransform();

    void SetPosition(glm::vec3 position);
    void SetRotation(glm::vec3 rotation);
    void SetRotation(glm::quat quat);
    void SetScale(glm::vec3 scale);
    virtual void SetTransform(const glm::mat4& transform);

    glm::vec3 GetAbsolutePosition();
    glm::vec3 GetAbsoluteRotationEuler();
    glm::quat GetAbsoluteRotationQuat();
    glm::vec3 GetAbsoluteScale();

    void SetAbsolutePosition(glm::vec3 position);
    void SetAbsoluteRotation(glm::vec3 rotation);
    void SetAbsoluteRotation(glm::quat rotation);
    void SetAbsoluteScale(glm::vec3 scale);

    void AddRotation(glm::quat rotation);
    void AddRotation(glm::vec3 rotation);
    void AddAbsoluteRotation(glm::quat rotation);
    void AddAbsoluteRotation(glm::vec3 rotation);
    void RotateAround(glm::vec3 pivot, glm::vec3 axis, float degrees);

    void LookAt(glm::vec3 target, glm::vec3 up);

    glm::vec3 GetCachedEulerRotation() const;

    glm::vec3 GetForwardVector() const;
    glm::vec3 GetRightVector() const;
    glm::vec3 GetUpVector() const;

    glm::mat4 GetParentTransform();
    int32_t GetParentBoneIndex() const;

    virtual void Attach(Node* parent, bool keepWorldTransform = false) override;

    static bool OnRep_RootPosition(Datum* datum, uint32_t index, const void* newValue);
    static bool OnRep_RootRotation(Datum* datum, uint32_t index, const void* newValue);
    static bool OnRep_RootScale(Datum* datum, uint32_t index, const void* newValue);

protected:

    glm::vec3 mPosition;
    glm::vec3 mRotationEuler;
    glm::vec3 mScale;

    glm::quat mRotationQuat;

    glm::mat4 mTransform;
    int32_t mParentBoneIndex;

    bool mTransformDirty;
};