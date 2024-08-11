#pragma once

#include "Nodes/3D/Mesh3d.h"
#include "AssetRef.h"
#include "Vertex.h"

enum class BoneInfluenceMode
{
    One,
    Four,
    Num
};

enum class AnimationUpdateMode
{
    AlwaysUpdateTimeAndBones,
    AlwaysUpdateTime,
    OnlyUpdateWhenRendered,

    Count
};

class SkeletalMesh;
struct AnimEvent;
struct Channel;
struct Animation;

struct ActiveAnimation
{
    std::string mName;
    float mTime = 0.0f;
    float mSpeed = 1.0f;;
    float mWeight = 0.0f;
    bool mLoop = false;
};

struct QueuedAnimation
{
    std::string mName;
    std::string mDependentAnim;
    float mTime = 0.0f;
    float mSpeed = 1.0f;;
    float mWeight = 0.0f;
    bool mLoop = false;
    uint8_t mPriority = 255;
};

typedef void(*AnimEventHandlerFP)(const AnimEvent& animEvent);

class SkeletalMesh3D : public Mesh3D
{
public:

    DECLARE_NODE(SkeletalMesh3D, Mesh3D);

    SkeletalMesh3D();
    ~SkeletalMesh3D();

    virtual const char* GetTypeName() const override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;

    virtual void Create() override;
    virtual void Destroy() override;
    SkeletalMeshCompResource* GetResource();

    virtual void Tick(float deltaTime) override;
    virtual void EditorTick(float deltaTime) override;

    virtual bool IsStaticMesh3D() const override;
    virtual bool IsSkeletalMesh3D() const override;

    void SetSkeletalMesh(SkeletalMesh* skeletalMesh);
    SkeletalMesh* GetSkeletalMesh();

    void PlayAnimation(const char* animName, bool loop, float speed = 1.0f, float weight = 1.0f, uint8_t priority = 255);
    void QueueAnimation(const char* animName, bool loop, const char* targetAnim = nullptr, float speed = 1.0f, float weight = 1.0f, uint8_t priority = 255);
    void StopAnimation(const char* animName, bool cancelQueued = false);
    void StopAllAnimations(bool cancelQueued = false);
    void CancelQueuedAnimation(const char* animName);
    void CancelAllQueuedAnimations();
    bool IsAnimationPlaying(const char* animName);
    void ResetAnimation();
    float GetAnimationSpeed() const;
    void SetAnimationSpeed(float speed);

    void SetAnimationPaused(bool paused);
    bool IsAnimationPaused() const;

    void SetInheritPose(bool inherit);
    bool IsInheritPoseEnabled() const;

    void SetBoundsRadiusOverride(float radius);
    float GetBoundsRadiusOverride() const;

    bool HasAnimatedThisFrame() const;

    ActiveAnimation* FindActiveAnimation(const char* animName);
    std::vector<ActiveAnimation>& GetActiveAnimations();

    QueuedAnimation* FindQueuedAnimation(const char* animName, const char* dependName = nullptr);
    std::vector<QueuedAnimation>& GetQueuedAnimations();

    glm::mat4 GetBoneTransform(const std::string& name) const;
    glm::vec3 GetBonePosition(const std::string& name) const;
    glm::quat GetBoneRotationQuat(const std::string& name) const;
    glm::vec3 GetBoneRotationEuler(const std::string& name) const;
    glm::vec3 GetBoneScale(const std::string& name) const;

    glm::mat4 GetBoneTransform(int32_t index) const;
    glm::vec3 GetBonePosition(int32_t boneIndex) const;
    glm::quat GetBoneRotationQuat(int32_t boneIndex) const;
    glm::vec3 GetBoneRotationEuler(int32_t boneIndex) const;
    glm::vec3 GetBoneScale(int32_t boneIndex) const;

    void SetBoneTransform(int32_t boneIndex, const glm::mat4& transform);
    void SetBonePosition(int32_t boneIndex, glm::vec3 position);
    void SetBoneRotation(int32_t boneIndex, glm::vec3 rotation);
    void SetBoneScale(int32_t boneIndex, glm::vec2 scale);

    uint32_t GetNumBones() const;
    BoneInfluenceMode GetBoneInfluenceMode() const;

    AnimationUpdateMode GetAnimationUpdateMode() const;
    void SetAnimationUpdateMode(AnimationUpdateMode mode);

    Vertex* GetSkinnedVertices();
    uint32_t GetNumSkinnedVertices();

    virtual Material* GetMaterial() override;
    virtual void Render() override;

    void UpdateAnimation(float deltaTime, bool updateBones);

    virtual Bounds GetLocalBounds() const override;

    int32_t FindBoneIndex(const std::string& name) const;

    void SetAnimEventHandler(AnimEventHandlerFP handler);
    AnimEventHandlerFP GetAnimEventHandler();
    void SetScriptAnimEventHandler(const ScriptFunc& func);

protected:

    static bool HandlePropChange(Datum* datum, uint32_t index, const void* newValue);

    void TickCommon(float deltaTime);

    glm::vec3 InterpolateScale(float time, const Channel& channel);
    glm::quat InterpolateRotation(float time, const Channel& channel);
    glm::vec3 InterpolatePosition(float time, const Channel& channel);
    void DetectTriggeredAnimEvents(
        const Animation& animation,
        float prevTickTime,
        float tickTime,
        float animationSpeed,
        std::vector<AnimEvent>& outEvents);

    uint32_t FindScaleIndex(float time, const Channel& channel);
    uint32_t FindRotationIndex(float time, const Channel& channel);
    uint32_t FindPositionIndex(float time, const Channel& channel);

    void UpdateAttachedChildren(float deltaTime);
    void CpuSkinVertices();

    SkeletalMeshRef mSkeletalMesh;
    std::vector<glm::mat4> mBoneMatrices;
    std::vector<Vertex> mSkinnedVertices; // Used by CPU skinning only.

    ScriptableFP<AnimEventHandlerFP> mAnimEventHandler;
    std::string mDefaultAnimation;
    float mAnimationSpeed = 1.0f;
    std::vector<ActiveAnimation> mActiveAnimations;
    std::vector<QueuedAnimation> mQueuedAnimations;
    float mBoundsRadiusOverride = 0.0f;
    bool mAnimationPaused;
    bool mRevertToBindPose;
    bool mInheritPose;
    bool mHasAnimatedThisFrame;

    BoneInfluenceMode mBoneInfluenceMode;
    AnimationUpdateMode mAnimationUpdateMode;

    // Graphics Resource
    SkeletalMeshCompResource mResource;
};