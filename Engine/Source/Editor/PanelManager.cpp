#if EDITOR

#include "PanelManager.h"
#include "Widgets/Widget.h"
#include "Widgets/OutlinerPanel.h"
#include "Widgets/AssetsPanel.h"
#include "Widgets/HierarchyPanel.h"
#include "Widgets/PropertiesPanel.h"
#include "Widgets/ViewportPanel.h"
#include "Widgets/WidgetHierarchyPanel.h"
#include "Widgets/WidgetViewportPanel.h"

PanelManager* PanelManager::sInstance = nullptr;

PanelManager::~PanelManager()
{
    for (uint32_t i = 0; i < mPanels.size(); ++i)
    {
        mPanels[i]->DetachFromParent();
        delete mPanels[i];
        mPanels[i] = nullptr;
    }

    mPanels.clear();
}

void PanelManager::Create()
{
    Destroy();
    sInstance = new PanelManager();
}

void PanelManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

PanelManager::PanelManager()
{
    mOutlinerPanel = new OutlinerPanel();
    mAssetsPanel = new AssetsPanel();
    mHierarchyPanel = new HierarchyPanel();
    mPropertiesPanel = new PropertiesPanel();
    mViewportPanel = new ViewportPanel();
    mWidgetHierarchyPanel = new WidgetHierarchyPanel();
    mWidgetViewportPanel = new WidgetViewportPanel();

    mPanels.push_back(mOutlinerPanel);
    mPanels.push_back(mAssetsPanel);
    mPanels.push_back(mHierarchyPanel);
    mPanels.push_back(mPropertiesPanel);
    mPanels.push_back(mViewportPanel);
    mPanels.push_back(mWidgetHierarchyPanel);
    mPanels.push_back(mWidgetViewportPanel);

    OnEditorModeChanged();
}

PanelManager* PanelManager::Get()
{
    return sInstance;
}

void PanelManager::Update()
{
    // Handle input
    if (!GetEditorState()->mPlayInEditor || GetEditorState()->mEjected)
    {
        switch (GetEditorMode())
        {
        case EditorMode::Level:
            mOutlinerPanel->HandleInput();
            mAssetsPanel->HandleInput();
            mHierarchyPanel->HandleInput();
            mPropertiesPanel->HandleInput();
            mViewportPanel->HandleInput();
            break;
        case EditorMode::Widget:
            mAssetsPanel->HandleInput();
            mWidgetHierarchyPanel->HandleInput();
            mPropertiesPanel->HandleInput();
            mWidgetViewportPanel->HandleInput();
            break;
        case EditorMode::Blueprint:
            break;

        default:
            OCT_ASSERT(0);
            break;
        }
    }
}

void PanelManager::AttachPanels(Widget* parent)
{
    for (uint32_t i = 0; i < mPanels.size(); ++i)
    {
        parent->AddChild(mPanels[i]);
    }
}

void PanelManager::SetPanelsVisible(bool visible)
{
    mPanelsVisible = visible;
    UpdatePanelVisibility();
}

void PanelManager::UpdatePanelVisibility()
{
    for (uint32_t i = 0; i < mPanels.size(); ++i)
    {
        mPanels[i]->SetVisible(false);
    }


    bool visible = mPanelsVisible;

    switch (GetEditorMode())
    {
    case EditorMode::Level:
    case EditorMode::Blueprint:
        mOutlinerPanel->SetVisible(visible);
        mAssetsPanel->SetVisible(visible);
        mHierarchyPanel->SetVisible(visible);
        mPropertiesPanel->SetVisible(visible);
        mViewportPanel->SetVisible(visible);
        break;
    case EditorMode::Widget:
        mAssetsPanel->SetVisible(visible);
        mPropertiesPanel->SetVisible(visible);
        mWidgetHierarchyPanel->SetVisible(visible);
        mWidgetViewportPanel->SetVisible(visible);
        break;
    }

}

OutlinerPanel* PanelManager::GetOutlinerPanel()
{
    return mOutlinerPanel;
}

AssetsPanel* PanelManager::GetAssetsPanel()
{
    return mAssetsPanel;
}

HierarchyPanel* PanelManager::GetHierarchyPanel()
{
    return mHierarchyPanel;
}

PropertiesPanel* PanelManager::GetPropertiesPanel()
{
    return mPropertiesPanel;
}

ViewportPanel* PanelManager::GetViewportPanel()
{
    return mViewportPanel;
}

WidgetHierarchyPanel* PanelManager::GetWidgetHierarchyPanel()
{
    return mWidgetHierarchyPanel;
}

WidgetViewportPanel* PanelManager::GetWidgetViewportPanel()
{
    return mWidgetViewportPanel;
}

void PanelManager::OnSelectedComponentChanged()
{
    mHierarchyPanel->OnSelectedComponentChanged();
    mPropertiesPanel->OnSelectedComponentChanged();
}

void PanelManager::OnSelectedAssetChanged()
{

}

void PanelManager::OnSelectedWidgetChanged()
{
    mWidgetHierarchyPanel->OnSelectedWidgetChanged();
    mPropertiesPanel->OnSelectedWidgetChanged();
}

void PanelManager::OnEditorModeChanged()
{
    EditorMode mode = GetEditorMode();

    UpdatePanelVisibility();

    mPropertiesPanel->OnEditorModeChanged();
}

#endif
