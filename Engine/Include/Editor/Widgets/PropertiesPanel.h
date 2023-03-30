#pragma once

#include "Panel.h"
#include <vector>
#include "Components/Component.h"
#include "Asset.h"
#include "Widgets/ComboBox.h"

class PropertyWidget;

enum class PropertiesMode
{
    Component,
    Actor,
    Asset,
    Widget,

    Count
};

struct CanvasCache
{
    ~CanvasCache();
    Canvas* FindOrCreateCanvas(TypeId type);

    std::vector<Canvas*> mCanvases;
    std::vector<TypeId> mTypes;
};

struct InspectTarget
{
    PropertiesMode mMode = PropertiesMode::Count;
    RTTI* mObject = nullptr;
};

class PropertiesPanel : public Panel
{
public:

    PropertiesPanel();
    ~PropertiesPanel();

    void OnEditorModeChanged();

    void OnSelectedComponentChanged();
    void OnSelectedWidgetChanged();

    virtual void Update() override;
    virtual void HandleInput() override;
    virtual void Render() override;

    void InspectAsset(Asset* asset);

    void RefreshProperties();

    void SetMode(PropertiesMode mode);
    PropertiesMode GetMode() const;

    RTTI* GetCurrentObject();

    void ClearInspectHistory();

protected:

    void UpdateDisplayedCanvas();
    void PopulatePropertyWidgets(Canvas* propCanvas, const std::vector<Property>& props);
    void RefreshPropertyWidgetLayout(Canvas* propCanvas);

    void RecordInspectionHistory();
    void ProgressInspectFuture();
    void RegressInspectPast();

    Component* mCurrentComponent;
    Asset* mCurrentAsset;
    Widget* mCurrentWidget;
    Canvas* mComponentCanvas;
    Canvas* mActorCanvas;
    Canvas* mAssetCanvas;
    Canvas* mWidgetCanvas;
    Canvas* mPropertiesCanvas;
    ComboBox* mModeBox;
    float mScrollDistance;
    CanvasCache mComponentCanvasCache;
    CanvasCache mAssetCanvasCache;
    CanvasCache mWidgetCanvasCache;
    PropertiesMode mMode;

    std::vector<InspectTarget> mInspectPast;
    std::vector<InspectTarget> mInspectFuture;
    RTTI* mPrevInspectObject = nullptr;
    PropertiesMode mPrevInspectMode = PropertiesMode::Count;
};