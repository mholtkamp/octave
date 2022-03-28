#if EDITOR

#include "Widgets/PropertyWidget.h"
#include "Widgets/Text.h"
#include "Widgets/TextField.h"
#include "Widgets/Selector.h"
#include "Widgets/ComboBox.h"
#include "Widgets/CheckBox.h"
#include "Widgets/Panel.h"
#include "EngineTypes.h"
#include "EditorState.h"
#include "AssetManager.h"
#include "Asset.h"

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
    // Check if there is a selected asset, if so, assign it to this property.
    Asset* selectedAsset = GetSelectedAsset();

    if (selectedAsset != nullptr)
    {
        assert(button->GetParent());
        AssetProp* propWidget = static_cast<AssetProp*>(button->GetParent());
        propWidget->AssignAsset(selectedAsset);
    }
}

void HandleAssetTextFieldChange(TextField* textField)
{
    // Search for an asset by name, otherwise reset it to what it was.
    assert(textField->GetParent());
    AssetProp* propWidget = static_cast<AssetProp*>(textField->GetParent());
    propWidget->AssignAsset(textField->GetTextString());
}

PropertyWidget::PropertyWidget() :
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
    mNameText->SetText(mProperty.mName);
}

float PropertyWidget::GetHeight()
{
    return sVerticalSpacing * 2;
}

void PropertyWidget::SetProperty(const Property& prop)
{
    mProperty = prop;
    MarkDirty();
}

const Property& PropertyWidget::GetProperty()
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
    mTextField->SetTextString(std::to_string(mProperty.GetFloat()));
}

void FloatProp::Write()
{
    try
    {
        mProperty.SetFloat(std::stof(mTextField->GetTextString()));
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
    mTextField->SetTextString(std::to_string(mProperty.GetInteger()));
}

void IntegerProp::Write()
{
    try
    {
        mProperty.SetInteger(std::stoi(mTextField->GetTextString()));
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
    const glm::vec3& vect = mProperty.GetVector();
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
        mProperty.SetVector(vect);
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
    const glm::vec4& color = mProperty.GetColor();
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
        mProperty.SetColor(color);
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
    mTextField->SetTextString(mProperty.GetString());
}

void StringProp::Write()
{
    mProperty.SetString(mTextField->GetTextString());
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
    mCheckBox->SetChecked(mProperty.GetBool());
}

void BoolProp::Write()
{
    mProperty.SetBool(mCheckBox->IsChecked());
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

    Asset* asset = mProperty.GetAsset();
    glm::vec4 assetColor = AssetManager::Get()->GetEditorAssetColor((TypeId)mProperty.mExtra);
    mTextField->SetTextString(asset ? mProperty.GetAsset()->GetName() : "");
    mTextField->GetText()->SetColor(assetColor);
    mAssignAssetButton->GetText()->SetColor(assetColor);
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
        mProperty.SetAsset(asset);
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

void EnumProp::SetProperty(const Property& prop)
{
    PropertyWidget::SetProperty(prop);

    mSelector->RemoveAllSelections();

    assert(mProperty.mEnumCount > 0);
    assert(mProperty.mEnumStrings != nullptr);
    for (uint32_t i = 0; i < uint32_t(mProperty.mEnumCount); ++i)
    {
        mSelector->AddSelection(mProperty.mEnumStrings[i]);
    }
}

void EnumProp::Update()
{
    PropertyWidget::Update();
    mSelector->SetSelectionIndex(mProperty.GetEnum());
}

void EnumProp::Write()
{
    try
    {
        mProperty.SetEnum(uint32_t(mSelector->GetSelectionIndex()));
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
    mTextField->SetTextString(std::to_string(mProperty.GetByte()));
}

void ByteProp::Write()
{
    try
    {
        mProperty.SetByte((uint8_t)std::stoi(mTextField->GetTextString()));
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

    uint8_t value = mProperty.GetByte();
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

    mProperty.SetByte(value);
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
    const glm::vec2& vect = mProperty.GetVector2D();
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
        mProperty.SetVector2D(vect);
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
