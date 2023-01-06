#pragma once

#include "Panel.h"
#include "EditorState.h"
#include "Widgets/ModalList.h"
#include "Widgets/PolyRect.h"

class Button;

enum class WidgetControlMode
{
    Default,
    Translate,
    Rotate,
    Scale,
    Pan
};

enum class WidgetAxisLock
{
    None,
    AxisX,
    AxisY,

    Count
};

class WidgetViewportPanel : public Panel
{
public:

    WidgetViewportPanel();
    ~WidgetViewportPanel();

    virtual void Update() override;
    virtual void HandleInput() override;

    void SetWidetControlMode(WidgetControlMode newMode);

protected:

    void HandleDefaultControls();
    void HandleTransformControls();
    void HandlePanControls();

    glm::vec2 HandleLockedCursor();
    void HandleAxisLocking();

    void SavePreTransforms();
    void RestorePreTransforms();

    Widget* FindHoveredWidget(Widget* widget, uint32_t& maxDepth, int32_t mouseX, int32_t mouseY, uint32_t depth = 0);

    float mZoom = 1.0f;
    glm::vec2 mOffset = { 0.0f, 0.0f };
    WidgetControlMode mControlMode = WidgetControlMode::Default;
    WidgetAxisLock mAxisLock = WidgetAxisLock::None;

    Widget* mEditRootWidget = nullptr;

    PolyRect* mSelectedRect = nullptr;
    PolyRect* mHoveredRect = nullptr;

    int32_t mPrevMouseX = 0;
    int32_t mPrevMouseY = 0;

    glm::vec2 mSavedPosition;
    glm::vec2 mSavedDimensions;
    float mSavedRotation;
};
