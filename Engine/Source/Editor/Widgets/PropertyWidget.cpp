#if EDITOR

#include "Widgets/PropertyWidget.h"
#include "Widgets/AssetsPanel.h"
#include "Widgets/Text.h"
#include "Widgets/TextField.h"
#include "Widgets/Selector.h"
#include "Widgets/ComboBox.h"
#include "Widgets/CheckBox.h"
#include "Widgets/Panel.h"
#include "EngineTypes.h"
#include "EditorState.h"
#include "AssetManager.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "Asset.h"
#include "InputDevices.h"


static float sIndent1 = 5.0f;
static float sIndent2 = 30.0f;
static float sIndent3 = 50.0f;

static float sHeight = 20.0f;
static float sWidth = Panel::sDefaultWidth - 50;
static float sWidth2 = Panel::sDefaultWidth - 80;

static float sVerticalSpacing = sHeight + 2;

void HandleTextFieldChange(TextField* textField)
{
    assert(textField->GetParent());
    PropertyWidget* propWidget = static_cast<PropertyWidget*>(textField->GetParent());
    propWidget->Write();
}

void HandleCheckBoxPressed(Button* checkBox)
{
    assert(checkBox->GetParent());
    PropertyWidget* propWidget = static_cast<PropertyWidget*>(checkBox->GetParent());
    propWidget->Write();
}

void HandleSelectorChange(Selector* selector)
{
    assert(selector->GetParent());
    PropertyWidget* propWidget = static_cast<PropertyWidget*>(selector->GetParent());
    propWidget->Write();
}

void HandleAssignAssetPressed(Button* button)
{
    assert(button->GetParent());
    AssetProp* propWidget = static_cast<AssetProp*>(button->GetParent());

    // Check if there is a selected asset, if so, assign it to this property.

    if (IsControlDown())
    {
        Asset* propAsset = propWidget->GetProperty().GetAsset();
        if (propAsset != nullptr)
        {
            PanelManager::Get()->GetAssetsPanel()->BrowseToAsset(propAsset->GetName());
        }
    }
    else 
    {
        Asset* selectedAsset = GetSelectedAsset();

        if (selectedAsset != nullptr)
        {
            propWidget->AssignAsset(selectedAsset);
        }
    }
}

void HandlePopVectorPressed(Button* button)
{
    PropertyArrayWidget* arrayWidget = (PropertyArrayWidget*) button->GetParent();

    Property& prop = arrayWidget->GetProperty();
    if (prop.IsVector() && prop.GetCount() > 0)
    {
        prop.EraseVector(prop.GetCount() - 1);
    }
}

void HandlePushVectorPressed(Button* button)
{
    PropertyArrayWidget* arrayWidget = (PropertyArrayWidget*)button->GetParent();

    Property& prop = arrayWidget->GetProperty();
    if (prop.IsVector() && prop.GetCount() < 255)
    {
        prop.PushBackVector();

        std::vector<PropertyWidget*>& elements = arrayWidget->GetElementWidgets();
        for (uint32_t i = 0; i < elements.size(); ++i)
        {
            elements[i]->SetProperty(prop, i);
        }
    }
}

void HandleAssetTextFieldChange(TextField* textField)
{
    // Search for an asset by name, otherwise reset it to what it was.
    assert(textField->GetParent());
    AssetProp* propWidget = static_cast<AssetProp*>(textField->GetParent());
    propWidget->AssignAsset(textField->GetTextString());
}

PropertyWidget* CreatePropWidget(const Property& prop, bool arrayElement)
{
    PropertyWidget* widget = nullptr;

    if (prop.IsArray() && !arrayElement)
    {
        widget = new PropertyArrayWidget();
    }
    else
    {
        // Create a new property widget
        switch (prop.mType)
        {
        case DatumType::Float: widget = new FloatProp(); break;
        case DatumType::Integer: widget = new IntegerProp(); break;
        case DatumType::Vector: widget = new VectorProp(); break;
        case DatumType::Color: widget = new ColorProp(); break;
        case DatumType::String: widget = new StringProp(); break;
        case DatumType::Bool: widget = new BoolProp(); break;
        case DatumType::Asset: widget = new AssetProp(); break;
        case DatumType::Enum: widget = new EnumProp(); break;
        case DatumType::Byte:
        {
            if (prop.mExtra == int32_t(ByteExtra::FlagWidget) ||
                prop.mExtra == int32_t(ByteExtra::ExclusiveFlagWidget))
            {
                ByteFlagProp* flagWidget = new ByteFlagProp();
                // TODO: Handle exclusive flag mode
                widget = flagWidget;
            }
            else
            {
                widget = new ByteProp();
            }
            break;
        }
        case DatumType::Vector2D: widget = new Vector2DProp(); break;

        case DatumType::Count: break;
        }
    }

    return widget;
}

PropertyWidget::PropertyWidget() :
    mOwnerType(PropertyOwnerType::Count),
    mIndex(0),
    mArrayElement(false),
    mNameText(nullptr)
{
    mNameText = new Text();
    mNameText->SetText("Property");
    mNameText->SetPosition(sIndent1, 0);
    AddChild(mNameText);

    SetDimensions(Panel::sDefaultWidth, GetHeight());
}

void PropertyWidget::Update()
{
    Widget::Update();

    if (mArrayElement)
    {
        mNameText->SetText(std::string("  [") + std::to_string(mIndex) + "]");
    }
    else
    {
        mNameText->SetText(mProperty.mName);
    }
}

float PropertyWidget::GetHeight()
{
    return sVerticalSpacing * 2;
}

void PropertyWidget::SetProperty(const Property& prop, uint32_t index)
{
    mProperty = prop;
    mIndex = index;

    // Determine owner type
    mOwnerType = PropertyOwnerType::Count;
    RTTI* rtti = (RTTI*)prop.mOwner;
    if (rtti)
    {
        if (rtti->As<Actor>())
            mOwnerType = PropertyOwnerType::Actor;
        else if (rtti->As<Component>())
            mOwnerType = PropertyOwnerType::Component;
        else if (rtti->As<Asset>())
            mOwnerType = PropertyOwnerType::Asset;
    }

    MarkDirty();
}

const Property& PropertyWidget::GetProperty() const
{
    return mProperty;
}

Property& PropertyWidget::GetProperty()
{
    return mProperty;
}

void PropertyWidget::TabTextField()
{
    TextField* selTf = TextField::GetSelectedTextField();
    bool foundTf = false;
    bool switchedTf = false;

    for (uint32_t i = 0; i < mChildren.size(); ++i)
    {
        if (mChildren[i] == selTf)
        {
            foundTf = true;
        }
        else if (foundTf && mChildren[i]->GetType() == TextField::GetStaticType())
        {
            TextField* newTf = (TextField*)mChildren[i];
            newTf->SetState(ButtonState::Pressed);

            switchedTf = true;
            break;
        }
    }

    if (!switchedTf)
    {
        // If couldn't tab to the next textfield, just unselect the current textfield as if hitting Enter.
        TextField::SetSelectedTextField(nullptr);
    }
}

PropertyArrayWidget::PropertyArrayWidget()
{
    mPopButton = new Button();
    mPopButton->SetPosition(sIndent1, sVerticalSpacing);
    mPopButton->SetDimensions(sHeight, sHeight);
    mPopButton->SetPressedHandler(HandlePopVectorPressed);
    mPopButton->SetTextString("  -");
    AddChild(mPopButton);

    mPushButton = new Button();
    mPushButton->SetPosition(sIndent1 + sHeight + 3, sVerticalSpacing);
    mPushButton->SetDimensions(sHeight, sHeight);
    mPushButton->SetPressedHandler(HandlePushVectorPressed);
    mPushButton->SetTextString("  +");
    AddChild(mPushButton);
}

void PropertyArrayWidget::Update()
{
    PropertyWidget::Update();

    bool resized = false;

    bool vector = mProperty.IsVector();
    mPopButton->SetVisible(vector);
    mPushButton->SetVisible(vector);

    if (mElementWidgets.size() < mProperty.GetCount())
    {
        mElementWidgets.resize(mProperty.GetCount());
        resized = true;
    }

    for (uint32_t i = 0; i < mProperty.GetCount(); ++i)
    {
        if (mElementWidgets[i] == nullptr)
        {
            mElementWidgets[i] = CreatePropWidget(mProperty, true);
            mElementWidgets[i]->SetPosition(glm::vec2(sIndent1, sVerticalSpacing * 2 + mElementWidgets[i]->GetHeight() * i));
            mElementWidgets[i]->MarkArrayElement();
            AddChild(mElementWidgets[i]);
        }

        mElementWidgets[i]->SetProperty(mProperty, i);
    }

    // Remove stale widgets
    if (mProperty.GetCount() < mElementWidgets.size())
    {
        for (uint32_t i = mProperty.GetCount(); i < mElementWidgets.size(); ++i)
        {
            mElementWidgets[i]->DetachFromParent();
            delete mElementWidgets[i];
            mElementWidgets[i] = nullptr;
        }

        mElementWidgets.resize(mProperty.GetCount());
        resized = true;
    }

    if (resized)
    {
        SetDimensions(Panel::sDefaultWidth, GetHeight());
    }
}

float PropertyArrayWidget::GetHeight()
{
    float totalHeight = 0.0f;

    bool isVector = mProperty.IsVector();
    float headerHeight = isVector ? (sVerticalSpacing * 2) : sVerticalSpacing;

    float bodyHeight = 0.0f;
    for (uint32_t i = 0; i < mElementWidgets.size(); ++i)
    {
        bodyHeight += mElementWidgets[i]->GetHeight();
    }

    totalHeight = headerHeight + bodyHeight;
    return totalHeight;
}

void PropertyArrayWidget::SetProperty(const Property& prop, uint32_t index)
{
    assert(index == 0);

    if (mProperty.mType != prop.mType)
    {
        // If we change prop types, we gotta kill all of our children.
        // In the future, I think maybe the optimal thing is to use a widget pool.
        // And also use that pool for non-array widgets.
        for (uint32_t i = 0; i < mElementWidgets.size(); ++i)
        {
            mElementWidgets[i]->DetachFromParent();
            delete mElementWidgets[i];
            mElementWidgets[i] = nullptr;
        }

        mElementWidgets.clear();
    }

    PropertyWidget::SetProperty(prop, index);
}


FloatProp::FloatProp() :
    mTextField(nullptr)
{
    mTextField = new TextField();
    mTextField->SetPosition(sIndent2, sVerticalSpacing);
    mTextField->SetDimensions(sWidth, sHeight);
    mTextField->SetTextConfirmHandler(HandleTextFieldChange);
    AddChild(mTextField);
}

void FloatProp::Update()
{
    PropertyWidget::Update();
    mTextField->SetTextString(std::to_string(mProperty.GetFloat(mIndex)));
}

void FloatProp::Write()
{
    try
    {
        float newValue = std::stof(mTextField->GetTextString());

        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            newValue);
    }
    catch (...)
    {

    }
}

IntegerProp::IntegerProp() :
    mTextField(nullptr)
{
    mTextField = new TextField();
    mTextField->SetPosition(sIndent2, sVerticalSpacing);
    mTextField->SetDimensions(sWidth, sHeight);
    mTextField->SetTextConfirmHandler(HandleTextFieldChange);
    AddChild(mTextField);
}

void IntegerProp::Update()
{
    PropertyWidget::Update();
    mTextField->SetTextString(std::to_string(mProperty.GetInteger(mIndex)));
}

void IntegerProp::Write()
{
    try
    {
        int32_t newValue = std::stoi(mTextField->GetTextString());

        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            newValue);
    }
    catch (...)
    {

    }
}

VectorProp::VectorProp()
{
    mTextX = new Text();
    mTextY = new Text();
    mTextZ = new Text();
    mTextFieldX = new TextField();
    mTextFieldY = new TextField();
    mTextFieldZ = new TextField();

    mTextX->SetText("X");
    mTextY->SetText("Y");
    mTextZ->SetText("Z");

    mTextX->SetPosition(sIndent2, sVerticalSpacing * 1);
    mTextFieldX->SetPosition(sIndent3, sVerticalSpacing * 1);
    mTextY->SetPosition(sIndent2, sVerticalSpacing * 2);
    mTextFieldY->SetPosition(sIndent3, sVerticalSpacing * 2);
    mTextZ->SetPosition(sIndent2, sVerticalSpacing * 3);
    mTextFieldZ->SetPosition(sIndent3, sVerticalSpacing * 3);

    mTextFieldX->SetDimensions(sWidth2, sHeight);
    mTextFieldY->SetDimensions(sWidth2, sHeight);
    mTextFieldZ->SetDimensions(sWidth2, sHeight);

    mTextFieldX->SetTextConfirmHandler(HandleTextFieldChange);
    mTextFieldY->SetTextConfirmHandler(HandleTextFieldChange);
    mTextFieldZ->SetTextConfirmHandler(HandleTextFieldChange);

    AddChild(mTextX);
    AddChild(mTextY);
    AddChild(mTextZ);
    AddChild(mTextFieldX);
    AddChild(mTextFieldY);
    AddChild(mTextFieldZ);

    SetDimensions(Panel::sDefaultWidth, GetHeight());
}

void VectorProp::Update()
{
    PropertyWidget::Update();
    const glm::vec3& vect = mProperty.GetVector(mIndex);
    mTextFieldX->SetTextString(std::to_string(vect.x));
    mTextFieldY->SetTextString(std::to_string(vect.y));
    mTextFieldZ->SetTextString(std::to_string(vect.z));
}

void VectorProp::Write()
{
    try
    {
        glm::vec3 vect;
        vect.x = std::stof(mTextFieldX->GetTextString());
        vect.y = std::stof(mTextFieldY->GetTextString());
        vect.z = std::stof(mTextFieldZ->GetTextString());
        
        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            vect);
    }
    catch (...)
    {

    }
}

float VectorProp::GetHeight()
{
    return sVerticalSpacing * 4;
}

ColorProp::ColorProp() :
    mTextW(nullptr),
    mTextFieldW(nullptr)
{
    mTextW = new Text();
    mTextFieldW = new TextField();

    mTextX->SetText("R");
    mTextY->SetText("G");
    mTextZ->SetText("B");
    mTextW->SetText("A");

    mTextW->SetPosition(sIndent2, sVerticalSpacing * 4);
    mTextFieldW->SetPosition(sIndent3, sVerticalSpacing * 4);
    mTextFieldW->SetDimensions(sWidth2, sHeight);
    mTextFieldW->SetTextConfirmHandler(HandleTextFieldChange);

    AddChild(mTextW);
    AddChild(mTextFieldW);

    SetDimensions(Panel::sDefaultWidth, GetHeight());
}

void ColorProp::Update()
{
    PropertyWidget::Update();
    const glm::vec4& color = mProperty.GetColor(mIndex);
    mTextFieldX->SetTextString(std::to_string(color.x));
    mTextFieldY->SetTextString(std::to_string(color.y));
    mTextFieldZ->SetTextString(std::to_string(color.z));
    mTextFieldW->SetTextString(std::to_string(color.w));
}

void ColorProp::Write()
{
    try
    {
        glm::vec4 color;
        color.x = std::stof(mTextFieldX->GetTextString());
        color.y = std::stof(mTextFieldY->GetTextString());
        color.z = std::stof(mTextFieldZ->GetTextString());
        color.w = std::stof(mTextFieldW->GetTextString());
        
        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            color);
    }
    catch (...)
    {

    }
}

float ColorProp::GetHeight()
{
    return sVerticalSpacing * 5;
}

StringProp::StringProp()
{
    mTextField = new TextField();
    mTextField->SetPosition(sIndent2, sVerticalSpacing);
    mTextField->SetDimensions(sWidth, sHeight);
    mTextField->SetTextConfirmHandler(HandleTextFieldChange);
    AddChild(mTextField);
}

void StringProp::Update()
{
    PropertyWidget::Update();
    mTextField->SetTextString(mProperty.GetString(mIndex));
}

void StringProp::Write()
{
    std::string newValue = mTextField->GetTextString();

    ActionManager::Get()->EXE_EditProperty(
        mProperty.mOwner,
        mOwnerType,
        mProperty.mName,
        mIndex,
        newValue);
}

BoolProp::BoolProp()
{
    mCheckBox = new CheckBox();
    mCheckBox->SetPosition(sIndent1, 0);
    mCheckBox->SetDimensions(sHeight, sHeight);
    mCheckBox->SetPressedHandler(HandleCheckBoxPressed);
    AddChild(mCheckBox);

    mNameText->SetPosition(sIndent2, 0);

    SetDimensions(Panel::sDefaultWidth, GetHeight());
}

void BoolProp::Update()
{
    PropertyWidget::Update();
    mCheckBox->SetChecked(mProperty.GetBool(mIndex));
}

void BoolProp::Write()
{
    bool newValue = mCheckBox->IsChecked();

    ActionManager::Get()->EXE_EditProperty(
        mProperty.mOwner,
        mOwnerType,
        mProperty.mName,
        mIndex,
        newValue);
}

float BoolProp::GetHeight()
{
    return sVerticalSpacing;
}

AssetProp::AssetProp()
{
    mTextField = new TextField();
    mTextField->SetPosition(sIndent2, sVerticalSpacing);
    mTextField->SetDimensions(sWidth, sHeight);
    mTextField->SetTextConfirmHandler(HandleAssetTextFieldChange);
    AddChild(mTextField);

    mAssignAssetButton = new Button();
    mAssignAssetButton->SetPosition(sIndent1, sVerticalSpacing);
    mAssignAssetButton->SetDimensions(sHeight, sHeight);
    mAssignAssetButton->SetPressedHandler(HandleAssignAssetPressed);
    mAssignAssetButton->SetTextString("  >");
    AddChild(mAssignAssetButton);

    SetDimensions(Panel::sDefaultWidth, GetHeight());
}

void AssetProp::Update()
{
    PropertyWidget::Update();

    Asset* asset = mProperty.GetAsset(mIndex);
    glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor((TypeId)mProperty.mExtra);
    mTextField->SetTextString(asset ? mProperty.GetAsset(mIndex)->GetName() : "");
    mTextField->GetText()->SetColor(assetColor);
    mAssignAssetButton->GetText()->SetColor(assetColor);
    mAssignAssetButton->SetTextString(IsControlDown() ? "  <" : "  >");
}

void AssetProp::Write()
{
    // Use AssignAsset to change property value
}

void AssetProp::AssignAsset(Asset* asset)
{
    if (asset == nullptr ||
        mProperty.mExtra == 0 ||
        asset->GetType() == TypeId(mProperty.mExtra))
    {
        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            asset);
    }
}

void AssetProp::AssignAsset(const std::string& name)
{
    if (name == "NULL" ||
        name == "null" ||
        name == "Null")
    {
        AssignAsset(nullptr);
    }
    else
    {
        Asset* asset = LoadAsset(name);

        if (asset != nullptr)
        {
            AssignAsset(asset);
        }
    }
}

EnumProp::EnumProp() :
    mSelector(nullptr)
{
    mSelector = new ComboBox();
    mSelector->SetPosition(sIndent2, sVerticalSpacing);
    mSelector->SetDimensions(sWidth, sHeight);
    mSelector->SetSelectionChangeHandler(HandleSelectorChange);
    AddChild(mSelector);
}

void EnumProp::SetProperty(const Property& prop, uint32_t index)
{
    bool refreshStrings = (mProperty.mData.vp != prop.mData.vp);

    PropertyWidget::SetProperty(prop, index);

    if (refreshStrings)
    {
        mSelector->RemoveAllSelections();

        assert(mProperty.mEnumCount > 0);
        assert(mProperty.mEnumStrings != nullptr);
        for (uint32_t i = 0; i < uint32_t(mProperty.mEnumCount); ++i)
        {
            mSelector->AddSelection(mProperty.mEnumStrings[i]);
        }
    }
}

void EnumProp::Update()
{
    PropertyWidget::Update();
    mSelector->SetSelectionIndex(mProperty.GetEnum(mIndex));
}

void EnumProp::Write()
{
    try
    {
        Datum newValue;
        newValue.PushBack(mSelector->GetSelectionIndex());

        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            newValue);
    }
    catch (...)
    {

    }
}

ByteProp::ByteProp() :
    mTextField(nullptr)
{
    mTextField = new TextField();
    mTextField->SetPosition(sIndent2, sVerticalSpacing);
    mTextField->SetDimensions(sWidth / 2.0f, sHeight);
    mTextField->SetTextConfirmHandler(HandleTextFieldChange);
    AddChild(mTextField);
}

void ByteProp::Update()
{
    PropertyWidget::Update();
    mTextField->SetTextString(std::to_string(mProperty.GetByte(mIndex)));
}

void ByteProp::Write()
{
    try
    {
        uint8_t newValue = (uint8_t)std::stoi(mTextField->GetTextString());

        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            newValue);
    }
    catch (...)
    {

    }
}

ByteFlagProp::ByteFlagProp()
{
    for (uint32_t i = 0; i < 8; ++i)
    {
        mCheckBoxes[i] = new CheckBox();
        mCheckBoxes[i]->SetPosition(sIndent1 + i * (sHeight), sVerticalSpacing);
        mCheckBoxes[i]->SetDimensions(sHeight - 2, sHeight - 2);
        mCheckBoxes[i]->SetPressedHandler(HandleCheckBoxPressed);
        AddChild(mCheckBoxes[i]);
    }
}

void ByteFlagProp::Update()
{
    PropertyWidget::Update();

    uint8_t value = mProperty.GetByte(mIndex);
    for (int32_t i = 0; i < 8; ++i)
    {
        // Check boxes are in reverse order
        int32_t bit = 7 - i;
        mCheckBoxes[i]->SetChecked(value & (1 << bit));
    }
}

void ByteFlagProp::Write()
{
    uint8_t value = 0;
    for (int32_t i = 0; i < 8; ++i)
    {
        // Check boxes are in reverse order
        int32_t bit = 7 - i;
        value |= ((mCheckBoxes[i]->IsChecked() ? 1 : 0) << bit);
    }

    ActionManager::Get()->EXE_EditProperty(
        mProperty.mOwner,
        mOwnerType,
        mProperty.mName,
        mIndex,
        value);
}

Vector2DProp::Vector2DProp()
{
    mTextX = new Text();
    mTextY = new Text();
    mTextFieldX = new TextField();
    mTextFieldY = new TextField();

    mTextX->SetText("X");
    mTextY->SetText("Y");

    mTextX->SetPosition(sIndent2, sVerticalSpacing * 1);
    mTextFieldX->SetPosition(sIndent3, sVerticalSpacing * 1);
    mTextY->SetPosition(sIndent2, sVerticalSpacing * 2);
    mTextFieldY->SetPosition(sIndent3, sVerticalSpacing * 2);

    mTextFieldX->SetDimensions(sWidth2, sHeight);
    mTextFieldY->SetDimensions(sWidth2, sHeight);

    mTextFieldX->SetTextConfirmHandler(HandleTextFieldChange);
    mTextFieldY->SetTextConfirmHandler(HandleTextFieldChange);

    AddChild(mTextX);
    AddChild(mTextY);
    AddChild(mTextFieldX);
    AddChild(mTextFieldY);

    SetDimensions(Panel::sDefaultWidth, GetHeight());
}

void Vector2DProp::Update()
{
    PropertyWidget::Update();
    const glm::vec2& vect = mProperty.GetVector2D(mIndex);
    mTextFieldX->SetTextString(std::to_string(vect.x));
    mTextFieldY->SetTextString(std::to_string(vect.y));
}

void Vector2DProp::Write()
{
    try
    {
        glm::vec2 vect;
        vect.x = std::stof(mTextFieldX->GetTextString());
        vect.y = std::stof(mTextFieldY->GetTextString());

        ActionManager::Get()->EXE_EditProperty(
            mProperty.mOwner,
            mOwnerType,
            mProperty.mName,
            mIndex,
            vect);
    }
    catch (...)
    {

    }
}

float Vector2DProp::GetHeight()
{
    return sVerticalSpacing * 3;
}

#endif
