#pragma once

class Widget;
class OutlinerPanel;
class AssetsPanel;
class HierarchyPanel;
class PropertiesPanel;
class ViewportPanel;
class WidgetHierarchyPanel;
class WidgetViewportPanel;

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
    WidgetHierarchyPanel* GetWidgetHierarchyPanel();
    WidgetViewportPanel* GetWidgetViewportPanel();

    void OnSelectedComponentChanged();
    void OnSelectedAssetChanged();
    void OnSelectedWidgetChanged();
    void OnEditorModeChanged();

protected:

    PanelManager();
    static PanelManager* sInstance;

    OutlinerPanel* mOutlinerPanel = nullptr;
    AssetsPanel* mAssetsPanel = nullptr;
    HierarchyPanel* mHierarchyPanel = nullptr;
    PropertiesPanel* mPropertiesPanel = nullptr;
    ViewportPanel* mViewportPanel = nullptr;

    WidgetHierarchyPanel* mWidgetHierarchyPanel = nullptr;
    WidgetViewportPanel* mWidgetViewportPanel = nullptr;
};
