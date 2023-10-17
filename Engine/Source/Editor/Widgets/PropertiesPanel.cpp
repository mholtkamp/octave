#if EDITOR

#include "Widgets/PropertiesPanel.h"
#include "EditorState.h"
#include "Property.h"
#include "PanelManager.h"
#include "Nodes/Node.h"
#include "Widgets/PropertyWidget.h"
#include "Widgets/HierarchyPanel.h"
#include "Widgets/TextField.h"
#include "Widgets/Text.h"
#include "InputDevices.h"
#include "Log.h"
#include "Actor.h"

void PropModeChangeHandler(Selector* selector)
{
    int32_t selIdx = selector->GetSelectionIndex();
    std::string selStr = selector->GetSelectionString();
    PropertiesPanel* propsPanel = PanelManager::Get()->GetPropertiesPanel();

    PropertiesMode newMode = (PropertiesMode)selIdx;
    propsPanel->SetMode(newMode);
}

CanvasCache::~CanvasCache()
{
    for (uint32_t i = 0; i < mCanvases.size(); ++i)
    {
        if (mCanvases[i] != nullptr && mCanvases[i]->GetParent() == nullptr)
        {
            delete mCanvases[i];
            mCanvases[i] = 0;
        }
    }

    mCanvases.clear();
    mTypes.clear();
}

Canvas* CanvasCache::FindOrCreateCanvas(TypeId type)
{
    Canvas* retCanvas = nullptr;
    
    for (uint32_t i = 0; i < mTypes.size(); ++i)
    {
        if (mTypes[i] == type)
        {
            retCanvas = mCanvases[i];
        }
    }

    if (retCanvas == nullptr)
    {
        retCanvas = new Canvas();
        mTypes.push_back(type);
        mCanvases.push_back(retCanvas);
    }

    return retCanvas;
}


PropertiesPanel::PropertiesPanel() :
    mCurrentComponent(nullptr),
    mCurrentAsset(nullptr),
    mCurrentWidget(nullptr),
    mComponentCanvas(nullptr),
    mActorCanvas(nullptr),
    mAssetCanvas(nullptr),
    mWidgetCanvas(nullptr),
    mGlobalCanvas(nullptr),
    mPropertiesCanvas(nullptr),
    mModeBox(nullptr),
    mScrollDistance(80.0f),
    mMode(PropertiesMode::Component)
{
    SetTitle("Properties");
    SetAnchorMode(AnchorMode::RightStretch);
    SetX(-sDefaultWidth);
    SetTopMargin(HierarchyPanel::sHierarchyPanelHeight);
    SetWidth(sDefaultWidth);
    SetBottomMargin(0.0f);

    mUseScissor = true;

    mModeBox = new ComboBox();
    mModeBox->SetSelectionIndex(0);
    mModeBox->SetSelectionChangeHandler(PropModeChangeHandler);
    mModeBox->SetAnchorMode(AnchorMode::TopRight);
    mModeBox->SetX(-84.0f);
    mModeBox->SetY(3.0f);
    mModeBox->SetWidth(80.0f);
    mModeBox->SetHeight(20.0f);
    mModeBox->GetText()->SetTextSize(14.0f);
    mModeBox->SetTextSize(14.0f);
    mModeBox->AddSelection("Component");
    mModeBox->AddSelection("Actor");
    mModeBox->AddSelection("Asset");
    mModeBox->AddSelection("Widget");
    mModeBox->AddSelection("Global");
    mHeaderCanvas->AddChild(mModeBox);

    std::vector<Property> globalProps;
    GatherGlobalProperties(globalProps);
    mGlobalCanvas = new Canvas();
    PopulatePropertyWidgets(mGlobalCanvas, globalProps);
}

PropertiesPanel::~PropertiesPanel()
{

}

void PropertiesPanel::OnEditorModeChanged()
{
    if (GetEditorMode() != EditorMode::Widget)
    {
        mCurrentWidget = nullptr;
        mWidgetCanvas = nullptr;

        if (mMode == PropertiesMode::Widget)
        {
            SetMode(PropertiesMode::Component);
        }
    }

    if (GetEditorMode() == EditorMode::Widget ||
        GetEditorMode() == EditorMode::Blueprint)
    {
        SetTopMargin(0.0f);
    }
    else
    {
        SetTopMargin(HierarchyPanel::sHierarchyPanelHeight);
    }
}

void PropertiesPanel::OnSelectedComponentChanged()
{
    if (mCurrentComponent != GetSelectedComponent())
    {
        Component* prevComponent = mCurrentComponent;
        mCurrentComponent = GetSelectedComponent();
        bool differentActor = (prevComponent == nullptr) ||
                              (mCurrentComponent == nullptr) ||
                              prevComponent->GetOwner() != mCurrentComponent->GetOwner();
        
        mComponentCanvas = nullptr;

        if (mActorCanvas != nullptr && differentActor)
        {
            // TODO: Get PropertyWidget pooling working!
            // Actor canvas is not cached. Gets recreated each time right now.
            // Need to create some sort of PropertyWidget pool to reduce allocations.
            if (mPropertiesCanvas == mActorCanvas)
            {
                mPropertiesCanvas = nullptr;
            }

            mActorCanvas->DetachFromParent();
            delete mActorCanvas;
            mActorCanvas = nullptr;
        }

        if (mCurrentComponent != nullptr)
        {
            // Update the component canvas for the new component
            mComponentCanvas = mComponentCanvasCache.FindOrCreateCanvas(mCurrentComponent->GetType());

            std::vector<Property> props;
            mCurrentComponent->GatherProperties(props);
            PopulatePropertyWidgets(mComponentCanvas, props);

            // And if the actor changed too, update the actor canvas.
            if (differentActor)
            {
                // Any selected component should have an owning actor.
                OCT_ASSERT(mCurrentComponent->GetOwner() != nullptr);
                OCT_ASSERT(mActorCanvas == nullptr);

                mActorCanvas = new Canvas();

                std::vector<Property> props;
                mCurrentComponent->GetOwner()->GatherProperties(props);
                PopulatePropertyWidgets(mActorCanvas, props);
            }
        }

        // If not in Actor or Component mode, just switch to Component mode.
        if (mMode != PropertiesMode::Actor &&
            mMode != PropertiesMode::Component)
        {
            SetMode(PropertiesMode::Component);
        }

        UpdateDisplayedCanvas();
    }
}

void PropertiesPanel::OnSelectedWidgetChanged()
{
    if (mCurrentWidget != GetSelectedWidget())
    {
        mCurrentWidget = GetSelectedWidget();

        if (mCurrentWidget)
        {
            mWidgetCanvas = mWidgetCanvasCache.FindOrCreateCanvas(mCurrentWidget->GetType());

            std::vector<Property> props;
            mCurrentWidget->GatherProperties(props);
            PopulatePropertyWidgets(mWidgetCanvas, props);

            SetMode(PropertiesMode::Widget);
        }
        else
        {
            mWidgetCanvas = nullptr;
        }

        UpdateDisplayedCanvas();
    }
}

void PropertiesPanel::InspectAsset(Asset* asset)
{
    mCurrentAsset = asset;

    if (mCurrentAsset != nullptr)
    {
        mAssetCanvas = mAssetCanvasCache.FindOrCreateCanvas(asset->GetType());

        std::vector<Property> props;
        mCurrentAsset.Get()->GatherProperties(props);
        PopulatePropertyWidgets(mAssetCanvas, props);
    }
    else
    {
        mAssetCanvas = nullptr;
    }

    // Always change to properties mode when the selected asset changes
    // This might change in the future.
    SetMode(PropertiesMode::Asset);

    UpdateDisplayedCanvas();
}

Asset* PropertiesPanel::GetInspectedAsset()
{
    return mCurrentAsset.Get();
}

void PropertiesPanel::RefreshProperties()
{
    if (GetSelectedComponent() != nullptr)
    {
        Component* selComp = GetSelectedComponent();
        mCurrentComponent = nullptr;
        OnSelectedComponentChanged();
    }
}

void PropertiesPanel::Update()
{
    Panel::Update();

    RecordInspectionHistory();

    if (IsDirty())
    {
        if (mPropertiesCanvas != nullptr)
        {
            mPropertiesCanvas->SetPosition(0.0f, -mScroll * mScrollDistance);
        }
    }

    if (mPropertiesCanvas != nullptr)
    {
        RefreshPropertyWidgetLayout(mPropertiesCanvas);
        SetMaxScroll(glm::max(static_cast<int32_t>(mPropertiesCanvas->GetHeight() / mScrollDistance) - 3, 0));
    }
}

void PropertiesPanel::HandleInput()
{
    Panel::HandleInput();

    if (ShouldHandleInput() &&
        IsMouseInsidePanel())
    {
        bool controlDown = IsControlDown();

        if (controlDown && IsKeyJustDown(KEY_A))
        {
            SetMode(PropertiesMode::Actor);
        }
        else if (controlDown && IsKeyJustDown(KEY_C))
        {
            SetMode(PropertiesMode::Component);
        }

        if (IsMouseButtonJustDown(MOUSE_X1))
        {
            RegressInspectPast();
        }
        else if (IsMouseButtonJustDown(MOUSE_X2))
        {
            ProgressInspectFuture();
        }
    }
    else if (IsKeyJustDown(KEY_TAB) && mPropertiesCanvas != nullptr)
    {
        // Check if any of our property widgets contain the selected textfield.
        // If they do, then call TabTextField() on the property widget to switch.
        uint32_t numProps = mPropertiesCanvas->GetNumChildren();
        TextField* selTf = TextField::GetSelectedTextField();
        bool tabbed = false;

        if (selTf != nullptr)
        {
            for (uint32_t i = 0; i < numProps; ++i)
            {
                // This will cause a problem if we ever add widgets to the PropertiesCanvas that arent PropWidgets...
                // Maybe we should add RTTI to Widget type.
                PropertyWidget* propWidget = (PropertyWidget*)mPropertiesCanvas->GetChild(i);
                uint32_t numChildren = propWidget->GetNumChildren();

                for (uint32_t c = 0; c < numChildren; ++c)
                {
                    if (propWidget->GetChild(c) == selTf)
                    {
                        propWidget->TabTextField();
                        tabbed = true;
                        break;
                    }
                }

                if (tabbed)
                {
                    break;
                }
            }
        }
    }
}

void PropertiesPanel::Render()
{
    Panel::Render();
}

void PropertiesPanel::SetMode(PropertiesMode mode)
{
    mMode = mode;
    UpdateDisplayedCanvas();

    if (int32_t(mMode) != mModeBox->GetSelectionIndex())
    {
        mModeBox->SetSelectionIndex((int32_t)mMode);
    }
}

PropertiesMode PropertiesPanel::GetMode() const
{
    return mMode;
}

RTTI* PropertiesPanel::GetCurrentObject()
{
    RTTI* retObj = nullptr;
    switch (mMode)
    {
    case PropertiesMode::Actor:
        retObj = mCurrentComponent ? mCurrentComponent->GetOwner() : nullptr;
        break;
    case PropertiesMode::Component:
        retObj = mCurrentComponent;
        break;
    case PropertiesMode::Asset:
        retObj = mCurrentAsset.Get();
        break;
    case PropertiesMode::Widget:
        retObj = mCurrentWidget;
        break;
    }

    return retObj;
}

void PropertiesPanel::UpdateDisplayedCanvas()
{
    if (mPropertiesCanvas != nullptr)
    {
        mPropertiesCanvas->DetachFromParent();
        mPropertiesCanvas = nullptr;
    }

    switch (mMode)
    {
    case PropertiesMode::Component: mPropertiesCanvas = mComponentCanvas; break;
    case PropertiesMode::Actor: mPropertiesCanvas = mActorCanvas; break;
    case PropertiesMode::Asset: mPropertiesCanvas = mAssetCanvas; break;
    case PropertiesMode::Widget: mPropertiesCanvas = mWidgetCanvas; break;
    case PropertiesMode::Global: mPropertiesCanvas = mGlobalCanvas; break;

    default: break;
    }

    if (mPropertiesCanvas != nullptr)
    {
        mBodyCanvas->AddChild(mPropertiesCanvas);
        SetMaxScroll(glm::max(static_cast<int32_t>(mPropertiesCanvas->GetHeight() / mScrollDistance) - 3, 0));
        MarkDirty();
    }
}

void PropertiesPanel::PopulatePropertyWidgets(Canvas* propCanvas, const std::vector<Property>& props)
{
    int32_t widgetIndex = 0;

    int32_t i = 0;
    for (i = 0; i < int32_t(props.size()); ++i)
    {
        const Property& prop = props[i];
        PropertyWidget* widget = nullptr;

        if (widgetIndex >= int32_t(propCanvas->GetNumChildren()))
        {
            widget = CreatePropWidget(prop);

            if (widget != nullptr)
            {
                //LogDebug("Created new property widget");
                propCanvas->AddChild(widget);
            }
        }
        else
        {
            // Recover the cached widget
            Widget* cachedWidget = propCanvas->GetChild(widgetIndex);
            OCT_ASSERT(cachedWidget != nullptr);

            if (cachedWidget != nullptr &&
                static_cast<PropertyWidget*>(cachedWidget)->IsArray() == prop.IsArray() && 
                static_cast<PropertyWidget*>(cachedWidget)->GetProperty().mType == prop.mType)
            {
                widget = static_cast<PropertyWidget*>(cachedWidget);
                widget->SetVisible(true);
            }
            else
            {
                Widget* delWidget = propCanvas->RemoveChild(widgetIndex);
                delete delWidget;

                widget = CreatePropWidget(prop);

                if (widget != nullptr)
                {
                    propCanvas->AddChild(widget, widgetIndex);
                }
            }
        }

        if (widget != nullptr)
        {
            widget->SetProperty(prop, 0);
            ++widgetIndex;
        }
    }

    // Hide unused properties
    for (; i < int32_t(propCanvas->GetNumChildren()); ++i)
    {
        propCanvas->GetChild(i)->SetVisible(false);
    }

    RefreshPropertyWidgetLayout(propCanvas);
}

void PropertiesPanel::RefreshPropertyWidgetLayout(Canvas* propCanvas)
{
    if (propCanvas != nullptr)
    {
        float yPos = 0.0f;

        for (uint32_t i = 0; i < propCanvas->GetNumChildren(); ++i)
        {
            PropertyWidget* propWidget = (PropertyWidget*)propCanvas->GetChild(i);

            if (propWidget && propWidget->IsVisible())
            {
                propWidget->SetPosition(0, yPos);
                yPos += propWidget->GetHeight();
            }
        }

        propCanvas->SetDimensions(Panel::sDefaultWidth, yPos);
    }
}

void PropertiesPanel::RecordInspectionHistory()
{
    RTTI* curObject = nullptr;

    switch (mMode)
    {
    case PropertiesMode::Actor:
        curObject = mCurrentComponent ? mCurrentComponent->GetOwner() : nullptr;
        break;
    case PropertiesMode::Component:
        curObject = mCurrentComponent;
        break;
    case PropertiesMode::Asset:
        curObject = mCurrentAsset.Get();
        break;
    case PropertiesMode::Widget:
        curObject = mCurrentWidget;
        break;
    }

    if (mPrevInspectObject != curObject)
    {
        if (mPrevInspectObject != nullptr)
        {
            InspectTarget target;
            target.mMode = mPrevInspectMode;
            target.mObject = mPrevInspectObject;
            mInspectPast.push_back(target);
            mInspectFuture.clear();
        }

        mPrevInspectMode = mMode;
        mPrevInspectObject = curObject;
    }
}

void PropertiesPanel::ClearInspectHistory()
{
    mInspectPast.clear();
    mInspectFuture.clear();
    mPrevInspectObject = nullptr;
    mPrevInspectMode = PropertiesMode::Count;
}

void PropertiesPanel::ProgressInspectFuture()
{
    if (mInspectFuture.size() > 0)
    {
        InspectTarget curTarget;
        curTarget.mMode = mMode;

        switch (mMode)
        {
        case PropertiesMode::Actor:
            curTarget.mObject = mCurrentComponent ? mCurrentComponent->GetOwner() : nullptr;
            break;
        case PropertiesMode::Component:
            curTarget.mObject = mCurrentComponent;
            break;
        case PropertiesMode::Asset:
            curTarget.mObject = mCurrentAsset.Get();
            break;
        case PropertiesMode::Widget:
            curTarget.mObject = mCurrentWidget;
            break;
        }

        if (mMode != PropertiesMode::Count &&
            curTarget.mObject != nullptr)
        {
            mInspectPast.push_back(curTarget);
        }

        InspectTarget target = mInspectFuture.back();
        OCT_ASSERT(target.mMode != PropertiesMode::Count);
        OCT_ASSERT(target.mObject != nullptr);
        mInspectFuture.pop_back();

        switch (target.mMode)
        {
        case PropertiesMode::Actor:
            SetSelectedActor(target.mObject->As<Actor>());
            break;
        case PropertiesMode::Component:
            SetSelectedComponent(target.mObject->As<Component>());
            break;
        case PropertiesMode::Asset:
            InspectAsset(target.mObject->As<Asset>());
            break;
        case PropertiesMode::Widget:
            SetSelectedWidget(target.mObject->As<Widget>());
            break;
        }

        SetMode(target.mMode);

        // Update these so we don't detect an inspection change.
        mPrevInspectMode = mMode;
        mPrevInspectObject = GetCurrentObject();
    }
}

void PropertiesPanel::RegressInspectPast()
{
    if (mInspectPast.size() > 0)
    {
        InspectTarget curTarget;
        curTarget.mMode = mMode;

        switch (mMode)
        {
        case PropertiesMode::Actor:
            curTarget.mObject = mCurrentComponent ? mCurrentComponent->GetOwner() : nullptr;
            break;
        case PropertiesMode::Component:
            curTarget.mObject = mCurrentComponent;
            break;
        case PropertiesMode::Asset:
            curTarget.mObject = mCurrentAsset.Get();
            break;
        case PropertiesMode::Widget:
            curTarget.mObject = mCurrentWidget;
            break;
        }

        if (mMode != PropertiesMode::Count &&
            curTarget.mObject != nullptr)
        {
            mInspectFuture.push_back(curTarget);
        }

        InspectTarget target = mInspectPast.back();
        OCT_ASSERT(target.mMode != PropertiesMode::Count);
        OCT_ASSERT(target.mObject != nullptr);
        mInspectPast.pop_back();

        switch (target.mMode)
        {
        case PropertiesMode::Actor:
            SetSelectedActor(target.mObject->As<Actor>());
            break;
        case PropertiesMode::Component:
            SetSelectedComponent(target.mObject->As<Component>());
            break;
        case PropertiesMode::Asset:
            InspectAsset(target.mObject->As<Asset>());
            break;
        case PropertiesMode::Widget:
            SetSelectedWidget(target.mObject->As<Widget>());
            break;
        }

        SetMode(target.mMode);

        // Update these so we don't detect an inspection change.
        mPrevInspectMode = mMode;
        mPrevInspectObject = GetCurrentObject();
    }
}


#endif
