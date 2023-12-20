#if EDITOR

#include "Viewport2d.h"
#include "InputDevices.h"
#include "EditorState.h"
#include "Engine.h"
#include "EditorUtils.h"
#include "ActionManager.h"
#include "Maths.h"
#include "Renderer.h"

Viewport2D::Viewport2D()
{
    mWrapperWidget = Node::Construct<Widget>();
    mWrapperWidget->SetName("Wrapper");
    mWrapperWidget->SetPosition(0, 0);
}

Viewport2D::~Viewport2D()
{
    Node::Destruct(mWrapperWidget);
    mWrapperWidget = nullptr;
}

void Viewport2D::Update(float deltaTime)
{
    Renderer* renderer = Renderer::Get();

    HandleInput();

    mWrapperWidget->SetPosition(mRootOffset);
    mWrapperWidget->SetScale({ mZoom, mZoom });
    mWrapperWidget->SetDimensions((float)renderer->GetViewportWidth(), (float)renderer->GetViewportHeight());

    Widget* selWidget = GetEditorState()->GetSelectedWidget();
    Widget* hoverWidget = nullptr;
    if (mControlMode == WidgetControlMode::Default)
    {
        int32_t mouseX = 0;
        int32_t mouseY = 0;
        GetMousePosition(mouseX, mouseY);
        mouseX -= renderer->GetViewportX();
        mouseY -= renderer->GetViewportY();

        uint32_t maxDepth = 0;
        Node* rootNode = GetWorld()->GetRootNode();

        if (rootNode != nullptr)
        {
            hoverWidget = FindHoveredWidget(rootNode, maxDepth, mouseX, mouseY);
        }
    }

    if (hoverWidget &&
        hoverWidget != selWidget)
    {
        mHoveredWidget = hoverWidget;
    }
    else
    {
        mHoveredWidget = nullptr;
    }

    mWrapperWidget->UpdateRect();
    Renderer::Get()->DirtyAllWidgets();
}

bool Viewport2D::ShouldHandleInput() const
{
    bool imguiWantsText = ImGui::GetIO().WantTextInput;
    bool imguiAnyWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    bool imguiAnyPopupUp = ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);


    bool handleInput = (!imguiAnyWindowHovered && !imguiWantsText && !imguiAnyPopupUp);
    return handleInput;
}

bool Viewport2D::IsMouseInside() const
{
    return true;
}

Widget* Viewport2D::GetWrapperWidget()
{
    return mWrapperWidget;
}

Widget* Viewport2D::GetHoveredWidget()
{
    return mHoveredWidget.Get<Widget>();
}

void Viewport2D::ResetViewport()
{
    mZoom = 1.0f;
    mRootOffset = { 0.0f, 0.0f };
}

void Viewport2D::HandleInput()
{
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

void Viewport2D::SetWidgetControlMode(WidgetControlMode newMode)
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

void Viewport2D::HandleDefaultControls()
{
    if (IsMouseInside())
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
            mouseX -= Renderer::Get()->GetViewportX();
            mouseY -= Renderer::Get()->GetViewportY();

            Widget* hoveredWidget = nullptr;
            
            uint32_t maxDepth = 0;
            hoveredWidget = FindHoveredWidget(GetWorld()->GetRootNode(), maxDepth, mouseX, mouseY);

            if (shiftDown || controlDown)
            {
                if (hoveredWidget != nullptr)
                {
                    if (GetEditorState()->IsNodeSelected(hoveredWidget))
                    {
                        GetEditorState()->RemoveSelectedNode(hoveredWidget);
                    }
                    else
                    {
                        GetEditorState()->AddSelectedNode(hoveredWidget, false);
                    }
                }
            }
            else
            {
                if (altDown)
                {
                    GetEditorState()->SetSelectedNode(hoveredWidget);
                    GetEditorState()->mTrackSelectedNode = true;
                }
                else
                {
                    if (GetEditorState()->GetSelectedWidget() != hoveredWidget)
                    {
                        GetEditorState()->SetSelectedNode(hoveredWidget);
                    }
                    else
                    {
                        GetEditorState()->SetSelectedNode(nullptr);
                    }
                }
            }
        }

        if (GetEditorState()->GetSelectedWidget() != nullptr)
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
            ResetViewport();
        }

        if (controlDown && IsKeyJustDown(KEY_D))
        {
            // Duplicate node
            const std::vector<Node*>& selectedNodes = GetEditorState()->GetSelectedNodes();

            if (selectedNodes.size() > 0)
            {
                ActionManager::Get()->DuplicateNodes(selectedNodes);
                SetWidgetControlMode(WidgetControlMode::Translate);
                SavePreTransforms();
            }
        }

        if (IsKeyJustDown(KEY_DELETE))
        {
            ActionManager::Get()->DeleteSelectedNodes();
        }

        if (altDown && IsKeyJustDown(KEY_A))
        {
            GetEditorState()->SetSelectedNode(nullptr);
        }
        if (controlDown && IsKeyJustDown(KEY_A))
        {
            std::vector<Node*> nodes = GetWorld()->GatherNodes();

            for (uint32_t i = 0; i < nodes.size(); ++i)
            {
                GetEditorState()->AddSelectedNode(nodes[i], false);
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
            float fMouseX = float(mouseX) - Renderer::Get()->GetViewportX();
            float fMouseY = float(mouseY) - Renderer::Get()->GetViewportY();
            
            float dx = fMouseX / mZoom - fMouseX / prevZoom;
            float dy = fMouseY / mZoom - fMouseY / prevZoom;
            mRootOffset += glm::vec2(dx, dy);
        }
    }
}

void Viewport2D::HandleTransformControls()
{
    Widget* widget = GetEditorState()->GetSelectedWidget();

    if (widget == nullptr)
        return;

    const std::vector<Node*>& selectedNodes = GetEditorState()->GetSelectedNodes();
    std::vector<Widget*> widgets;
    for (uint32_t i = 0; i < selectedNodes.size(); ++i)
    {
        if (selectedNodes[i]->IsWidget())
        {
            widgets.push_back((Widget*)selectedNodes[i]);
        }
    }

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

            for (uint32_t i = 0; i < widgets.size(); ++i)
            {
                glm::vec2 offset = widgets[i]->GetOffset();
                offset += speed * stretchScale * delta;
                widgets[i]->SetOffset(offset.x, offset.y);
            }
        }
        else if (mControlMode == WidgetControlMode::Rotate)
        {
            const float rotateSpeed = 0.025f;
            float speed = shiftDown ? (shiftSpeedMult * rotateSpeed) : rotateSpeed;
            float totalDelta = -(delta.x - delta.y);

            for (uint32_t i = 0; i < widgets.size(); ++i)
            {
                float rotation = widgets[i]->GetRotation();
                rotation += speed * totalDelta;
                widgets[i]->SetRotation(rotation);
            }
        }
        else if (mControlMode == WidgetControlMode::Scale)
        {
            const float scaleSpeed = 0.050f;
            float speed = shiftDown ? (shiftSpeedMult * scaleSpeed) : scaleSpeed;

            if (mAxisLock == WidgetAxisLock::AxisX)
                delta.y = 0.0f;
            else if (mAxisLock == WidgetAxisLock::AxisY)
                delta.x = 0.0f;

            for (uint32_t i = 0; i < widgets.size(); ++i)
            {
                glm::vec2 size = widgets[i]->GetSize();
                size += speed * stretchScale * delta;
                widgets[i]->SetSize(size.x, size.y);
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_LEFT))
    {
        std::vector<VpWidgetTransform> newTransforms;
        for (uint32_t i = 0; i < widgets.size(); ++i)
        {
            newTransforms.push_back(VpWidgetTransform());
            newTransforms.back().mOffset = widgets[i]->GetOffset();
            newTransforms.back().mSize = widgets[i]->GetSize();
            newTransforms.back().mRotation = widgets[i]->GetRotation();
        }

        RestorePreTransforms();

        if (mControlMode == WidgetControlMode::Translate)
        {
            for (uint32_t i = 0; i < widgets.size(); ++i)
            {
                glm::vec2 offset = newTransforms[i].mOffset;
                ActionManager::Get()->EXE_EditProperty(widgets[i], PropertyOwnerType::Node, "Offset", 0, offset);
            }
        }
        else if (mControlMode == WidgetControlMode::Rotate)
        {
            for (uint32_t i = 0; i < widgets.size(); ++i)
            {
                float rotation = newTransforms[i].mRotation;
                ActionManager::Get()->EXE_EditProperty(widgets[i], PropertyOwnerType::Node, "Rotation", 0, rotation);
            }
        }
        else if (mControlMode == WidgetControlMode::Scale)
        {
            for (uint32_t i = 0; i < widgets.size(); ++i)
            {
                glm::vec2 size = newTransforms[i].mSize;
                ActionManager::Get()->EXE_EditProperty(widgets[i], PropertyOwnerType::Node, "Size", 0, size);
            }
        }

        SetWidgetControlMode(WidgetControlMode::Default);
    }

    if (IsMouseButtonDown(MOUSE_RIGHT))
    {
        RestorePreTransforms();
        SetWidgetControlMode(WidgetControlMode::Default);
    }
}

void Viewport2D::HandlePanControls()
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


glm::vec2 Viewport2D::HandleLockedCursor()
{
    int32_t dX = 0;
    int32_t dY = 0;
    INP_GetMouseDelta(dX, dY);

    return glm::vec2((float)dX, (float)dY);
}

void Viewport2D::HandleAxisLocking()
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

void Viewport2D::SavePreTransforms()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
    mSavedTransforms.clear();

    for (uint32_t i = 0; i < selNodes.size(); ++i)
    {
        Widget* widget = (selNodes[i] && selNodes[i]->IsWidget()) ? static_cast<Widget*>(selNodes[i]) : nullptr;

        if (widget)
        {
            mSavedTransforms.push_back(VpWidgetTransform());

            mSavedTransforms.back().mOffset = widget->GetOffset();
            mSavedTransforms.back().mSize = widget->GetSize();
            mSavedTransforms.back().mRotation = widget->GetRotation();
        }
    }
}

void Viewport2D::RestorePreTransforms()
{
    const std::vector<Node*>& selNodes = GetEditorState()->GetSelectedNodes();
    for (uint32_t i = 0; i < selNodes.size(); ++i)
    {
        if (i >= mSavedTransforms.size())
        {
            LogError("Component/Transform array mismatch?");
            break;
        }

        Widget* widget = (selNodes[i] && selNodes[i]->IsWidget()) ? static_cast<Widget*>(selNodes[i]) : nullptr;

        if (widget)
        {
            VpWidgetTransform& trans = mSavedTransforms[i];

            widget->SetOffset(trans.mOffset.x, trans.mOffset.y);
            widget->SetSize(trans.mSize.x, trans.mSize.y);
            widget->SetRotation(trans.mRotation);
        }
    }
}

Widget* Viewport2D::FindHoveredWidget(Node* node, uint32_t& maxDepth, int32_t mouseX, int32_t mouseY, uint32_t depth)
{
    if (node == nullptr)
        return nullptr;

    Widget* retWidget = nullptr;
    Widget* widget = node->As<Widget>();

    if (widget)
    {
        Rect rect = widget->GetRect();

        if (rect.ContainsPoint(float(mouseX), float(mouseY)) &&
            depth >= maxDepth)
        {
            retWidget = widget;
            maxDepth = depth;
        }
    }

    if (!node->IsSceneLinked())
    {
        for (uint32_t i = 0; i < node->GetNumChildren(); ++i)
        {
            Node* child = node->GetChild(i);
            Widget* childWidgetFound = FindHoveredWidget(child, maxDepth, mouseX, mouseY, depth + 1);

            if (childWidgetFound)
            {
                retWidget = childWidgetFound;
            }
        }
    }

    return retWidget;
}

#endif
