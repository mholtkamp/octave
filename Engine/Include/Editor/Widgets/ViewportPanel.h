#pragma once

#include "Panel.h"
#include "EditorState.h"
#include "Widgets/ModalList.h"

class Button;

class ViewportPanel : public Panel
{
public:

    ViewportPanel();
    ~ViewportPanel();

    virtual void Update() override;
    virtual void HandleInput() override;

    float GetFocalDistance() const;

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
    void ToggleTransformMode();

    glm::vec3 GetLockedTranslationDelta(glm::vec3 deltaWS) const;
    glm::vec3 GetLockedRotationAxis() const;
    glm::vec3 GetLockedScaleDelta();

    static void ShowSpawnActorPrompt(bool basic);

    Button* mFileButton = nullptr;
    Button* mViewButton = nullptr;
    Button* mWorldButton = nullptr;
    Button* mPlayButton = nullptr;
    Button* mStopButton = nullptr;

    Text* mPieWarningText = nullptr;

    float mFirstPersonMoveSpeed;
    float mFirstPersonRotationSpeed;

    // Transform Control vars
    int32_t mPrevMouseX;
    int32_t mPrevMouseY;
    std::vector<glm::mat4> mPreTransforms;

    float mFocalDistance;
    bool mNeedsMouseRecenter = false;
    bool mTransformLocal = false;
};