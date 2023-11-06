#pragma once

#include "EditorState.h"
#include "Nodes/Widgets/ModalList.h"
#include "Nodes/Widgets/PolyRect.h"

class Button;

struct VpWidgetTransform
{
    glm::vec2 mOffset;
    glm::vec2 mSize;
    float mRotation;
};

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

class Viewport2D
{
public:

    Viewport2D();
    ~Viewport2D();

    void Update(float deltaTime);
    bool ShouldHandleInput() const;
    bool IsMouseInside() const;
    void HandleInput();

    void SetWidgetControlMode(WidgetControlMode newMode);

    Widget* GetWrapperWidget();
    PolyRect* GetSelectedRect();
    PolyRect* GetHoveredRect();

protected:

    void HandleDefaultControls();
    void HandleTransformControls();
    void HandlePanControls();

    glm::vec2 HandleLockedCursor();
    void HandleAxisLocking();

    void SavePreTransforms();
    void RestorePreTransforms();

    Widget* FindHoveredWidget(Node* node, uint32_t& maxDepth, int32_t mouseX, int32_t mouseY, uint32_t depth = 0);

    float mZoom = 1.0f;
    glm::vec2 mRootOffset = { 0.0f, 0.0f };
    WidgetControlMode mControlMode = WidgetControlMode::Default;
    WidgetAxisLock mAxisLock = WidgetAxisLock::None;

    Widget* mWrapperWidget = nullptr;
    PolyRect* mSelectedRect = nullptr;
    PolyRect* mHoveredRect = nullptr;

    int32_t mPrevMouseX = 0;
    int32_t mPrevMouseY = 0;

    std::vector<VpWidgetTransform> mSavedTransforms;
};
