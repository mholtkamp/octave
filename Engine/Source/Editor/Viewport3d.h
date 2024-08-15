#pragma once

#include "EditorState.h"
#include "Nodes/Widgets/ModalList.h"
#include "Nodes/3D/InstancedMesh3d.h"

class Button;


class Viewport3D
{
public:

    Viewport3D();
    ~Viewport3D();

    void Update(float deltaTime);
    bool ShouldHandleInput() const;
    bool IsMouseInside() const;

    float GetFocalDistance() const;
    void ToggleTransformMode();

protected:

    static constexpr float sDefaultFocalDistance = 10.0f;

    static void HandleFilePressed(Button* button);
    static void HandleViewPressed(Button* button);
    static void HandleWorldPressed(Button* button);
    static void HandlePlayPressed(Button* button);
    static void HandleStopPressed(Button* button);

    void HandleDefaultControls();
    void HandlePilotControls();
    void HandleTransformControls();
    void HandlePanControls();
    void HandleOrbitControls();

    glm::vec2 HandleLockedCursor();
    void HandleAxisLocking();

    glm::vec2 GetTransformDelta() const;
    void SavePreTransforms();
    void RestorePreTransforms();

    glm::vec3 GetLockedTranslationDelta(glm::vec3 deltaWS) const;
    glm::vec3 GetLockedRotationAxis() const;
    glm::vec3 GetLockedScaleDelta();
    bool ShouldTransformInstance() const;

    float mFirstPersonMoveSpeed = 10.0f;
    float mFirstPersonRotationSpeed = 0.07f;

    // Transform Control vars
    int32_t mPrevMouseX = 0;
    int32_t mPrevMouseY = 0;
    std::vector<glm::mat4> mPreTransforms;

    float mFocalDistance = sDefaultFocalDistance;
    bool mNeedsMouseRecenter = false;
    bool mTransformLocal = false;

    MeshInstanceData mInstancePreTransform;
};
