#pragma once

class Widget;
class OutlinerPanel;
class AssetsPanel;
class HierarchyPanel;
class PropertiesPanel;
class ViewportPanel;

class PanelManager
{
public:

    ~PanelManager();
    static void Create();
    static void Destroy();
    static PanelManager* Get();

    void Update();

    void AttachPanels(Widget* parent);
    void SetPanelsVisible(bool visible);

    OutlinerPanel* GetOutlinerPanel();
    AssetsPanel* GetAssetsPanel();
    HierarchyPanel* GetHierarchyPanel();
    PropertiesPanel* GetPropertiesPanel();
    ViewportPanel* GetViewportPanel();

    void OnSelectedComponentChanged();
    void OnSelectedAssetChanged();

protected:

    PanelManager();
    static PanelManager* sInstance;

    OutlinerPanel* mOutlinerPanel;
    AssetsPanel* mAssetsPanel;
    HierarchyPanel* mHierarchyPanel;
    PropertiesPanel* mPropertiesPanel;
    ViewportPanel* mViewportPanel;
};
