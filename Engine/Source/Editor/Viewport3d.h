#pragma once

#include "EditorState.h"
#include "Nodes/Widgets/ModalList.h"

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

    bool IsMouseOnAnyButton() const;

    static void ShowSpawnActorPrompt(bool basic);

    float mFirstPersonMoveSpeed;
    float mFirstPersonRotationSpeed;

    std::vector<Button*> mButtons;

    // Transform Control vars
    int32_t mPrevMouseX;
    int32_t mPrevMouseY;
    std::vector<glm::mat4> mPreTransforms;

    float mFocalDistance;
    bool mNeedsMouseRecenter = false;
    bool mTransformLocal = false;
};
