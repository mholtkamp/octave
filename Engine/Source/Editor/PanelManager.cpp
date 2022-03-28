#if EDITOR

#include "PanelManager.h"
#include "Widgets/Widget.h"
#include "Widgets/OutlinerPanel.h"
#include "Widgets/AssetsPanel.h"
#include "Widgets/HierarchyPanel.h"
#include "Widgets/PropertiesPanel.h"
#include "Widgets/ViewportPanel.h"

PanelManager* PanelManager::sInstance = nullptr;

PanelManager::~PanelManager()
{
    mOutlinerPanel->DetachFromParent();
    mAssetsPanel->DetachFromParent();
    mHierarchyPanel->DetachFromParent();
    mPropertiesPanel->DetachFromParent();
    mViewportPanel->DetachFromParent();

    delete mOutlinerPanel;
    delete mAssetsPanel;
    delete mHierarchyPanel;
    delete mPropertiesPanel;
    delete mViewportPanel;

    mOutlinerPanel = nullptr;
    mAssetsPanel = nullptr;
    mHierarchyPanel = nullptr;
    mPropertiesPanel = nullptr;
    mViewportPanel = nullptr;
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
}

PanelManager* PanelManager::Get()
{
    return sInstance;
}

void PanelManager::Update()
{
    // Handle input
    mOutlinerPanel->HandleInput();
    mAssetsPanel->HandleInput();
    mHierarchyPanel->HandleInput();
    mPropertiesPanel->HandleInput();
    mViewportPanel->HandleInput();
}

void PanelManager::AttachPanels(Widget* parent)
{
    parent->AddChild(mOutlinerPanel);
    parent->AddChild(mAssetsPanel);
    parent->AddChild(mHierarchyPanel);
    parent->AddChild(mPropertiesPanel);
    parent->AddChild(mViewportPanel);
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

void PanelManager::OnSelectedComponentChanged()
{
    mHierarchyPanel->OnSelectedComponentChanged();
    mPropertiesPanel->OnSelectedComponentChanged();
}

void PanelManager::OnSelectedAssetChanged()
{

}

#endif
