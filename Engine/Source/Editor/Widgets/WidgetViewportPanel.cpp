#if EDITOR

#include "Widgets/WidgetViewportPanel.h"
#include "InputDevices.h"
#include "EditorState.h"
#include "Engine.h"
#include "EditorUtils.h"
#include "Widgets/ActionList.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "Maths.h"

WidgetViewportPanel::WidgetViewportPanel()
{
    SetTitle("Widget Viewport");
    SetAnchorMode(AnchorMode::FullStretch);
    SetMargins(sDefaultWidth, 0.0f, sDefaultWidth, 0.0f);

    mWrapperWidget = new Widget();
    mWrapperWidget->SetName("Wrapper");
    mWrapperWidget->SetPosition(0, 0);
    AddChild(mWrapperWidget);

    mSelectedRect = new PolyRect();
    mSelectedRect->SetName("Select Rect");
    mSelectedRect->SetVisible(false);
    mSelectedRect->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
    AddChild(mSelectedRect);

    mHoveredRect = new PolyRect();
    mHoveredRect->SetName("Hover Rect");
    mHoveredRect->SetVisible(false);
    mHoveredRect->SetColor({ 0.0f, 1.0f, 1.0f, 1.0f });
    AddChild(mHoveredRect);

    mEditorLabel = new Text();
    mEditorLabel->SetName("Label");
    mEditorLabel->SetText("WIDGET");
    mEditorLabel->SetColor({ 1.0f, 1.0f, 1.0f, 0.5f });
    mEditorLabel->SetTextSize(72);
    mEditorLabel->SetAnchorMode(AnchorMode::TopRight);
    mEditorLabel->SetDimensions(300.0f, 80.0f);
    mEditorLabel->SetPosition(-310.0f, 4.0f);
    AddChild(mEditorLabel);

    mHeaderText->SetVisible(false);
    mHeaderQuad->SetVisible(false);
    mBodyQuad->SetVisible(false);
}

WidgetViewportPanel::~WidgetViewportPanel()
{

}


void WidgetViewportPanel::Update()
{
    Panel::Update();

    SyncEditRootWidget();

    mWrapperWidget->SetPosition(mRootOffset);
    mWrapperWidget->SetScale({ mZoom, mZoom });
    mWrapperWidget->SetDimensions(GetWidth(), GetHeight());

    Widget* selWidget = GetSelectedWidget();
    if (selWidget)
    {
        selWidget->Update();
        mSelectedRect->SetVisible(true);
        Rect overRect = selWidget->GetRect();

        // Viewport panel is pushed over to the right a bit,
        // so to translated the absolute rect position to the relative rect
        // position, we need to subtract the parent X;
        overRect.mX -= GetX();

        mSelectedRect->SetRect(overRect);
        mSelectedRect->Update();
    }
    else
    {
        mSelectedRect->SetVisible(false);
    }

    Widget* hoverWidget = nullptr;
    if (mEditRootWidget &&
        mControlMode == WidgetControlMode::Default)
    {
        int32_t mouseX = 0;
        int32_t mouseY = 0;
        GetMousePosition(mouseX, mouseY);

        uint32_t maxDepth = 0;
        hoverWidget = FindHoveredWidget(mEditRootWidget, maxDepth, mouseX, mouseY);
    }

    if (hoverWidget &&
        hoverWidget != selWidget)
    {
        hoverWidget->Update();
        mHoveredRect->SetVisible(true);
        Rect overRect = hoverWidget->GetRect();

        // Viewport panel is pushed over to the right a bit,
        // so to translated the absolute rect position to the relative rect
        // position, we need to subtract the parent X;
        overRect.mX -= GetX();

        mHoveredRect->SetRect(overRect);
        mHoveredRect->Update();
    }
    else
    {
        mHoveredRect->SetVisible(false);
    }
}

void WidgetViewportPanel::HandleInput()
{
    Panel::HandleInput();

    if (ShouldHandleInput())
    {
        if (GetEditorState()->mMouseNeedsRecenter)
        {
            EditorCenterCursor();
            GetEditorState()->mMouseNeedsRecenter = false;
        }

        switch (mControlMode)
        {
        case WidgetControlMode::Default: HandleDefaultControls(); break;
        case WidgetControlMode::Translate: HandleTransformControls(); break;
        case WidgetControlMode::Rotate: HandleTransformControls(); break;
        case WidgetControlMode::Scale: HandleTransformControls(); break;
        case WidgetControlMode::Pan: HandlePanControls(); break;
        }
    }

    INP_GetMousePosition(mPrevMouseX, mPrevMouseY);
}

void WidgetViewportPanel::SetWidgetControlMode(WidgetControlMode newMode)
{
    if (mControlMode == newMode)
    {
        return;
    }

    WidgetControlMode prevMode = mControlMode;
    mControlMode = newMode;

    if (prevMode == WidgetControlMode::Translate ||
        prevMode == WidgetControlMode::Rotate ||
        prevMode == WidgetControlMode::Scale ||
        prevMode == WidgetControlMode::Pan)
    {
        INP_ShowCursor(true);
        INP_LockCursor(false);
    }

    if (newMode == WidgetControlMode::Translate ||
        newMode == WidgetControlMode::Rotate ||
        newMode == WidgetControlMode::Scale ||
        newMode == WidgetControlMode::Pan)
    {
        INP_ShowCursor(false);
        INP_LockCursor(true);

        // But because of the event loop processing, we might get a bogus mouse motion event even after
        // we have just forced the position. So set a flag to let the viewport panel know that we need to
        // recenter the mouse next frame.
        GetEditorState()->mMouseNeedsRecenter = true;
    }

    // Always reset transform lock when switching control modes.
    mAxisLock = WidgetAxisLock::None;
}

void WidgetViewportPanel::OnSelectedWidgetChanged()
{
    SyncEditRootWidget();
}

void WidgetViewportPanel::HandleDefaultControls()
{
    if (IsMouseInsidePanel())
    {
        const int32_t scrollDelta = GetScrollWheelDelta();
        const bool controlDown = IsControlDown();
        const bool shiftDown = IsShiftDown();
        const bool altDown = IsAltDown();
        const bool cmdKeyDown = (controlDown || shiftDown || altDown);

        if (IsMouseButtonJustDown(MOUSE_RIGHT) ||
            IsMouseButtonJustDown(MOUSE_MIDDLE))
        {
            SetWidgetControlMode(WidgetControlMode::Pan);
        }

        if (IsMouseButtonJustDown(MOUSE_LEFT))
        {
            int32_t mouseX = 0;
            int32_t mouseY = 0;
            GetMousePosition(mouseX, mouseY);

            Widget* hoveredWidget = nullptr;
            
            if (mEditRootWidget)
            {
                uint32_t maxDepth = 0;
                hoveredWidget = FindHoveredWidget(mEditRootWidget, maxDepth, mouseX, mouseY);
            }

            if (GetSelectedWidget() != hoveredWidget)
            {
                SetSelectedWidget(hoveredWidget);
            }
            else
            {
                SetSelectedWidget(nullptr);
            }
        }

        if (GetSelectedWidget() != nullptr)
        {
            if (!controlDown && !altDown && IsKeyJustDown(KEY_G))
            {
                SetWidgetControlMode(WidgetControlMode::Translate);
                SavePreTransforms();
            }

            if (!controlDown && !altDown && IsKeyJustDown(KEY_R))
            {
                SetWidgetControlMode(WidgetControlMode::Rotate);
                SavePreTransforms();
            }

            if (!controlDown && !altDown && IsKeyJustDown(KEY_S))
            {
                SetWidgetControlMode(WidgetControlMode::Scale);
                SavePreTransforms();
            }
        }

        if (IsKeyJustDown(KEY_F) ||
            IsKeyJustDown(KEY_DECIMAL))
        {
            LogDebug("Reset viewport");
            mZoom = 1.0f;
            mRootOffset = { 0.0f, 0.0f };
        }

        if (controlDown && IsKeyJustDown(KEY_D))
        {
            // Duplicate selected widget
            Widget* selWidget = GetSelectedWidget();
            if (selWidget)
            {
                Widget* newWidget = selWidget->Clone();
                if (selWidget->GetParent() != mWrapperWidget)
                {
                    selWidget->GetParent()->AddChild(newWidget);
                }
                else
                {
                    mEditRootWidget->AddChild(newWidget);
                }

                SetSelectedWidget(newWidget);

                ActionManager::Get()->EXE_AddWidget(newWidget);
                SetWidgetControlMode(WidgetControlMode::Translate);
                SavePreTransforms();
            }
        }

        if (IsKeyJustDown(KEY_DELETE))
        {
            Widget* selWidget = GetSelectedWidget();
            if (selWidget != nullptr)
            {
                if (!selWidget->IsNativeChild())
                {
                    ActionManager::Get()->EXE_RemoveWidget(GetSelectedWidget());
                }
                else
                {
                    LogWarning("Cannot delete native child widget.");
                }
            }
        }

        if (scrollDelta != 0)
        {
            float prevZoom = mZoom;
            mZoom += (scrollDelta * 0.1f);
            mZoom = glm::clamp(mZoom, 0.05f, 10.0f);
            float deltaZoom = mZoom - prevZoom;

            int32_t mouseX = 0;
            int32_t mouseY = 0;
            GetMousePosition(mouseX, mouseY);
            float fMouseX = float(mouseX) - mRect.mX;
            float fMouseY = float(mouseY) - mRect.mY;
            
            float dx = fMouseX / mZoom - fMouseX / prevZoom;
            float dy = fMouseY / mZoom - fMouseY / prevZoom;
            mRootOffset += glm::vec2(dx, dy);
        }
    }
}

void WidgetViewportPanel::HandleTransformControls()
{
    Widget* widget = GetSelectedWidget();

    if (widget == nullptr)
        return;

    HandleAxisLocking();
    glm::vec2 delta = HandleLockedCursor();

    const bool shiftDown = IsShiftDown();
    const float shiftSpeedMult = 0.1f;

    glm::vec2 stretchScale = { 1.0f, 1.0f };
    if (widget->StretchX())
    {
        stretchScale.x = 0.002f;
    }
    if (widget->StretchY())
    {
        stretchScale.y = 0.002f;
    }

    if (delta != glm::vec2(0.0f, 0.0f))
    {
        if (mControlMode == WidgetControlMode::Translate)
        {
            const float translateSpeed = 0.1f;
            float speed = shiftDown ? (shiftSpeedMult * translateSpeed) : translateSpeed;

            if (mAxisLock == WidgetAxisLock::AxisX)
                delta.y = 0.0f;
            else if (mAxisLock == WidgetAxisLock::AxisY)
                delta.x = 0.0f;

            glm::vec2 offset = widget->GetOffset();
            offset += speed * stretchScale * delta;
            widget->SetOffset(offset.x, offset.y);
        }
        else if (mControlMode == WidgetControlMode::Rotate)
        {
            const float rotateSpeed = 0.025f;
            float speed = shiftDown ? (shiftSpeedMult * rotateSpeed) : rotateSpeed;
            float totalDelta = -(delta.x - delta.y);

            float rotation = widget->GetRotation();
            rotation += speed * totalDelta;
            widget->SetRotation(rotation);
        }
        else if (mControlMode == WidgetControlMode::Scale)
        {
            const float scaleSpeed = 0.050f;
            float speed = shiftDown ? (shiftSpeedMult * scaleSpeed) : scaleSpeed;

            if (mAxisLock == WidgetAxisLock::AxisX)
                delta.y = 0.0f;
            else if (mAxisLock == WidgetAxisLock::AxisY)
                delta.x = 0.0f;

            glm::vec2 size = widget->GetSize();
            size += speed * stretchScale * delta;
            widget->SetSize(size.x, size.y);
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT))
    {
        if (mControlMode == WidgetControlMode::Translate)
        {
            glm::vec2 offset = widget->GetOffset();
            RestorePreTransforms();
            ActionManager::Get()->EXE_EditProperty(widget, PropertyOwnerType::Widget, "Offset", 0, offset);
        }
        else if (mControlMode == WidgetControlMode::Rotate)
        {
            float rotation = widget->GetRotation();
            RestorePreTransforms();
            ActionManager::Get()->EXE_EditProperty(widget, PropertyOwnerType::Widget, "Rotation", 0, rotation);
        }
        else if (mControlMode == WidgetControlMode::Scale)
        {
            glm::vec2 size = widget->GetSize();
            RestorePreTransforms();
            ActionManager::Get()->EXE_EditProperty(widget, PropertyOwnerType::Widget, "Size", 0, size);
        }

        SetWidgetControlMode(WidgetControlMode::Default);
    }

    if (IsMouseButtonDown(MOUSE_RIGHT))
    {
        RestorePreTransforms();
        SetWidgetControlMode(WidgetControlMode::Default);
    }
}

void WidgetViewportPanel::HandlePanControls()
{
    glm::vec2 delta = HandleLockedCursor();
    float speed = 0.1f;
    mRootOffset += speed * delta;

    if (!IsMouseButtonDown(MOUSE_RIGHT) &&
        !IsMouseButtonDown(MOUSE_MIDDLE))
    {
        SetWidgetControlMode(WidgetControlMode::Default);
    }
}


glm::vec2 WidgetViewportPanel::HandleLockedCursor()
{
    int32_t dX = 0;
    int32_t dY = 0;
    INP_GetMouseDelta(dX, dY);

    return glm::vec2((float)dX, (float)dY);
}

void WidgetViewportPanel::HandleAxisLocking()
{
    WidgetAxisLock newLock = WidgetAxisLock::None;

    if (IsKeyJustDown(KEY_X))
    {
        newLock = WidgetAxisLock::AxisX;
    }

    if (IsKeyJustDown(KEY_Y))
    {
        newLock = WidgetAxisLock::AxisY;
    }

    if (newLock != WidgetAxisLock::None)
    {
        RestorePreTransforms();

        if (newLock != mAxisLock)
        {
            mAxisLock = newLock;
        }
        else
        {
            mAxisLock = WidgetAxisLock::None;
        }
    }
}

void WidgetViewportPanel::SavePreTransforms()
{
    Widget* widget = GetSelectedWidget();

    if (widget)
    {
        mSavedOffset = widget->GetOffset();
        mSavedSize = widget->GetSize();
        mSavedRotation = widget->GetRotation();
    }
}

void WidgetViewportPanel::RestorePreTransforms()
{
    Widget* widget = GetSelectedWidget();

    if (widget)
    {
        widget->SetOffset(mSavedOffset.x, mSavedOffset.y);
        widget->SetSize(mSavedSize.x, mSavedSize.y);
        widget->SetRotation(mSavedRotation);
    }
}

void WidgetViewportPanel::SyncEditRootWidget()
{
    // Sync the edit root widget.
    Widget* editRoot = GetEditRootWidget();

    if (editRoot != mEditRootWidget)
    {
        if (mEditRootWidget != nullptr)
        {
            mWrapperWidget->RemoveChild(mEditRootWidget);
        }

        mEditRootWidget = editRoot;

        if (mEditRootWidget != nullptr)
        {
            mWrapperWidget->AddChild(mEditRootWidget, 0);
        }
    }
}

Widget* WidgetViewportPanel::FindHoveredWidget(Widget* widget, uint32_t& maxDepth, int32_t mouseX, int32_t mouseY, uint32_t depth)
{
    Widget* retWidget = nullptr;

    Rect rect = widget->GetRect();

    if (rect.ContainsPoint(float(mouseX), float(mouseY)) &&
        depth >= maxDepth)
    {
        retWidget = widget;
        maxDepth = depth;
    }

    if (widget->GetWidgetMap() == nullptr)
    {
        for (uint32_t i = 0; i < widget->GetNumChildren(); ++i)
        {
            Widget* child = widget->GetChild(i);
            Widget* childFound = FindHoveredWidget(child, maxDepth, mouseX, mouseY, depth + 1);

            if (childFound)
            {
                retWidget = childFound;
            }
        }
    }

    return retWidget;
}

#endif
