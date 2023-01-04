#if EDITOR

#include "Widgets/PropertiesPanel.h"
#include "EditorState.h"
#include "Property.h"
#include "PanelManager.h"
#include "Components/Component.h"
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
    mComponentCanvas(nullptr),
    mActorCanvas(nullptr),
    mAssetCanvas(nullptr),
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
    mModeBox->GetText()->SetSize(14.0f);
    mModeBox->SetTextSize(14.0f);
    mModeBox->AddSelection("Component");
    mModeBox->AddSelection("Actor");
    mModeBox->AddSelection("Asset");
    mHeaderCanvas->AddChild(mModeBox);
}

PropertiesPanel::~PropertiesPanel()
{

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

void PropertiesPanel::InspectAsset(Asset* asset)
{
    mCurrentAsset = asset;

    if (mCurrentAsset != nullptr)
    {
        mAssetCanvas = mAssetCanvasCache.FindOrCreateCanvas(asset->GetType());

        std::vector<Property> props;
        mCurrentAsset->GatherProperties(props);
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

void PropertiesPanel::RefreshProperties()
{
    if (GetSelectedComponent() != nullptr)
    {
        Component* selComp = GetSelectedComponent();
        mCurrentComponent = nullptr;
        OnSelectedComponentChanged();
    }

    if (mCurrentAsset != nullptr)
    {
        Asset* asset = mCurrentAsset;
        mCurrentAsset = nullptr;
        InspectAsset(asset);
    }
}

void PropertiesPanel::Update()
{
    Panel::Update();

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

    for (int32_t i = 0; i < int32_t(props.size()); ++i)
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

#endif
