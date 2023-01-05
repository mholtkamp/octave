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

    Count
};

struct CanvasCache
{
    ~CanvasCache();
    Canvas* FindOrCreateCanvas(TypeId type);

    std::vector<Canvas*> mCanvases;
    std::vector<TypeId> mTypes;
};

class PropertiesPanel : public Panel
{
public:

    PropertiesPanel();
    ~PropertiesPanel();
    void OnSelectedComponentChanged();
    void OnSelectedWidgetChanged();

    virtual void Update() override;
    virtual void HandleInput() override;
    virtual void Render() override;

    void InspectAsset(Asset* asset);

    void RefreshProperties();

    void SetMode(PropertiesMode mode);
    PropertiesMode GetMode() const;

protected:

    void UpdateDisplayedCanvas();
    void PopulatePropertyWidgets(Canvas* propCanvas, const std::vector<Property>& props);
    void RefreshPropertyWidgetLayout(Canvas* propCanvas);

    Component* mCurrentComponent;
    Asset* mCurrentAsset;
    Canvas* mComponentCanvas;
    Canvas* mActorCanvas;
    Canvas* mAssetCanvas;
    Canvas* mPropertiesCanvas;
    ComboBox* mModeBox;
    float mScrollDistance;
    CanvasCache mComponentCanvasCache;
    CanvasCache mAssetCanvasCache;
    PropertiesMode mMode;
};