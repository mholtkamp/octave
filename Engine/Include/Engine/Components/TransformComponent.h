#pragma once

#include "Component.h"
#include "EngineTypes.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "AssetRef.h"

class SkeletalMeshComponent;

class TransformComponent : public Component
{
public:

    DECLARE_COMPONENT(TransformComponent, Component);

    TransformComponent();
    virtual ~TransformComponent();

    virtual void SaveStream(Stream& stream) override;
	virtual void LoadStream(Stream& stream) override;

    virtual void Create() override;
    virtual void Destroy() override;

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual bool IsTransformComponent() const override;

    void Attach(TransformComponent* parent, bool keepWorldTransform = false);
    void AddChild(TransformComponent* child);
    void RemoveChild(TransformComponent* child);
    void RemoveChild(int32_t index);

    void AttachToBone(SkeletalMeshComponent* parent, const char* boneName, bool keepWorldTransform = false);
    void AttachToBone(SkeletalMeshComponent* parent, int32_t boneIndex, bool keepWorldTransform = false);

    TransformComponent* GetParent();
    const std::vector<TransformComponent*>& GetChildren() const;

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

    const glm::mat4& GetTransform() const;

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

    int32_t GetChildIndex(const char* childName);
    TransformComponent* GetChild(const char* childName);
    TransformComponent* GetChild(int32_t index);
    uint32_t GetNumChildren() const;
    int32_t FindParentComponentIndex() const;

    glm::mat4 GetParentTransform();
    int32_t GetParentBoneIndex() const;

protected:

    TransformComponent* mParent;
    std::vector<TransformComponent*> mChildren;

    glm::vec3 mPosition;
    glm::vec3 mRotationEuler;
    glm::vec3 mScale;

    glm::quat mRotationQuat;

    glm::mat4 mTransform;
    int32_t mParentBoneIndex;

    bool mTransformDirty;
};