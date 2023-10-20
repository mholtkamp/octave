#if EDITOR

#include "Nodes/Widgets/SceneImportWidget.h"
#include "Renderer.h"
#include "InputDevices.h"

#include "ActionManager.h"

void SceneImportWidget::HandleFilePressed(Button* button)
{
    SceneImportWidget* impWidget = (SceneImportWidget*)button->GetParent();

    std::string openPath = SYS_OpenFileDialog();

    if (openPath != "")
    {
        impWidget->mFileText->SetText(openPath);
    }
}

void SceneImportWidget::HandleImportPressed(Button* button)
{
    SceneImportWidget* impWidget = (SceneImportWidget*)button->GetParent();

    SceneImportOptions importOptions;
    importOptions.mFilePath = impWidget->mFileText->GetText();
    importOptions.mPrefix = impWidget->mPrefixField->GetTextString();
    importOptions.mCleanDirectory = impWidget->mCleanDirectoryBox->IsChecked();
    importOptions.mClearWorld = impWidget->mClearWorldBox->IsChecked();
    importOptions.mImportObjects = impWidget->mImportObjectsBox->IsChecked();
    importOptions.mImportMeshes = impWidget->mImportMeshesBox->IsChecked();
    importOptions.mImportMaterials = impWidget->mImportMaterialsBox->IsChecked();
    importOptions.mImportTextures = impWidget->mImportTexturesBox->IsChecked();
    importOptions.mImportLights = impWidget->mImportLightsBox->IsChecked();
    importOptions.mDefaultShadingModel = ShadingModel::Lit;

    const std::string& shading = impWidget->mDefaultShadingField->GetTextString();
    if (shading == "Unlit")
        importOptions.mDefaultShadingModel = ShadingModel::Unlit;
    if (shading == "Toon")
        importOptions.mDefaultShadingModel = ShadingModel::Toon;

    ActionManager::Get()->ImportScene(importOptions);

    Renderer::Get()->SetModalWidget(nullptr);
}

void SceneImportWidget::HandleCancelPressed(Button* button)
{
    if (Renderer::Get()->GetModalWidget() == button->GetParent())
    {
        Renderer::Get()->SetModalWidget(nullptr);
    }
}

SceneImportWidget::SceneImportWidget()
{
    SetDimensions(700, 400);

    const float kDeltaY = 26.0f;
    const float kDeltaX = 34.0f;
    const float kButtonWidth = 60.0f;
    const float kButtonHeight = 22.0f;
    const float kCheckBoxSize = kButtonHeight;
    const float kTextSize = 16.0f;
    float x = 10.0f;
    float y = 10.0f;

    Quad* bg = new Quad();
    bg->SetAnchorMode(AnchorMode::FullStretch);
    bg->SetRatios(0.0f, 0.0f, 1.0f, 1.0f);
    bg->SetColor({ 0.2f, 0.1f, 0.1f, 0.8f }, { 0.2f, 0.1f, 0.1f, 0.8f }, { 0.3f, 0.2f, 0.0f, 0.8f }, { 0.3f, 0.2f, 0.0f, 0.8f });
    bg->SetTexture(LoadAsset<Texture>("T_White"));
    AddChild(bg);

    mFileButton = new Button();
    mFileButton->SetPosition(x, y);
    mFileButton->SetDimensions(kButtonWidth, kButtonHeight);
    mFileButton->SetTextString("File...");
    mFileButton->SetPressedHandler(HandleFilePressed);
    AddChild(mFileButton);
    y += kDeltaY;

    mFileText = new Text();
    mFileText->SetPosition(x + kDeltaX, y);
    mFileText->SetTextSize(kTextSize);
    mFileText->SetText("");
    AddChild(mFileText);
    y += kDeltaY;

    mPrefixLabel = new Text();
    mPrefixLabel->SetPosition(x, y);
    mPrefixLabel->SetTextSize(kTextSize);
    mPrefixLabel->SetText("Prefix:");
    AddChild(mPrefixLabel);
    y += kDeltaY;

    mPrefixField = new TextField();
    mPrefixField->SetPosition(x + kDeltaX, y);
    mPrefixField->SetTextString("");
    mPrefixField->SetDimensions(100.0f, kButtonHeight);
    AddChild(mPrefixField);
    y += kDeltaY;

    mDefaultShadingLabel = new Text();
    mDefaultShadingLabel->SetPosition(x + kDeltaX, y);
    mDefaultShadingLabel->SetTextSize(kTextSize);
    mDefaultShadingLabel->SetText("Default Shading");
    AddChild(mDefaultShadingLabel);
    y += kDeltaY;

    mDefaultShadingField = new TextField();
    mDefaultShadingField->SetPosition(x + kDeltaX, y);
    mDefaultShadingField->SetTextString("Lit");
    mDefaultShadingField->SetDimensions(100.0f, kButtonHeight);
    AddChild(mDefaultShadingField);
    y += kDeltaY;

    mCleanDirectoryBox = new CheckBox();
    mCleanDirectoryBox->SetPosition(x, y);
    mCleanDirectoryBox->SetChecked(true);
    mCleanDirectoryBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mCleanDirectoryBox);

    mCleanDirectoryLabel = new Text();
    mCleanDirectoryLabel->SetPosition(x + kDeltaX, y);
    mCleanDirectoryLabel->SetTextSize(kTextSize);
    mCleanDirectoryLabel->SetText("Clean Directory");
    AddChild(mCleanDirectoryLabel);
    y += kDeltaY;

    mClearWorldBox = new CheckBox();
    mClearWorldBox->SetPosition(x, y);
    mClearWorldBox->SetChecked(true);
    mClearWorldBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mClearWorldBox);

    mClearWorldLabel = new Text();
    mClearWorldLabel->SetPosition(x + kDeltaX, y);
    mClearWorldLabel->SetTextSize(kTextSize);
    mClearWorldLabel->SetText("Clear World");
    AddChild(mClearWorldLabel);
    y += kDeltaY;

    mImportObjectsBox = new CheckBox();
    mImportObjectsBox->SetPosition(x, y);
    mImportObjectsBox->SetChecked(true);
    mImportObjectsBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mImportObjectsBox);

    mImportObjectsLabel = new Text();
    mImportObjectsLabel->SetPosition(x + kDeltaX, y);
    mImportObjectsLabel->SetTextSize(kTextSize);
    mImportObjectsLabel->SetText("Import Objects");
    AddChild(mImportObjectsLabel);
    y += kDeltaY;

    mImportMeshesBox = new CheckBox();
    mImportMeshesBox->SetPosition(x, y);
    mImportMeshesBox->SetChecked(true);
    mImportMeshesBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mImportMeshesBox);

    mImportMeshesLabel = new Text();
    mImportMeshesLabel->SetPosition(x + kDeltaX, y);
    mImportMeshesLabel->SetTextSize(kTextSize);
    mImportMeshesLabel->SetText("Import Meshes");
    AddChild(mImportMeshesLabel);
    y += kDeltaY;

    mImportMaterialsBox = new CheckBox();
    mImportMaterialsBox->SetPosition(x, y);
    mImportMaterialsBox->SetChecked(true);
    mImportMaterialsBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mImportMaterialsBox);

    mImportMaterialsLabel = new Text();
    mImportMaterialsLabel->SetPosition(x + kDeltaX, y);
    mImportMaterialsLabel->SetTextSize(kTextSize);
    mImportMaterialsLabel->SetText("Import Materials");
    AddChild(mImportMaterialsLabel);
    y += kDeltaY;

    mImportTexturesBox = new CheckBox();
    mImportTexturesBox->SetPosition(x, y);
    mImportTexturesBox->SetChecked(true);
    mImportTexturesBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mImportTexturesBox);

    mImportTexturesLabel = new Text();
    mImportTexturesLabel->SetPosition(x + kDeltaX, y);
    mImportTexturesLabel->SetTextSize(kTextSize);
    mImportTexturesLabel->SetText("Import Textures");
    AddChild(mImportTexturesLabel);
    y += kDeltaY;

    mImportLightsBox = new CheckBox();
    mImportLightsBox->SetPosition(x, y);
    mImportLightsBox->SetChecked(false);
    mImportLightsBox->SetDimensions(kCheckBoxSize, kCheckBoxSize);
    AddChild(mImportLightsBox);

    mImportLightsLabel = new Text();
    mImportLightsLabel->SetPosition(x + kDeltaX, y);
    mImportLightsLabel->SetTextSize(kTextSize);
    mImportLightsLabel->SetText("Import Lights");
    AddChild(mImportLightsLabel);
    y += kDeltaY;

    Button* mImportButton = new Button();
    mImportButton->SetPosition(-2.5 * kButtonWidth, - 1.25 * kButtonHeight);
    mImportButton->SetAnchorMode(AnchorMode::BottomRight);
    mImportButton->SetDimensions(kButtonWidth, kButtonHeight);
    mImportButton->SetTextString("Import");
    mImportButton->SetPressedHandler(HandleImportPressed);
    AddChild(mImportButton);

    Button* mCancelButton = new Button();
    mCancelButton->SetPosition(-1.25 * kButtonWidth, -1.25 * kButtonHeight);
    mCancelButton->SetAnchorMode(AnchorMode::BottomRight);
    mCancelButton->SetDimensions(kButtonWidth, kButtonHeight);
    mCancelButton->SetTextString("Cancel");
    mCancelButton->SetPressedHandler(HandleCancelPressed);
    AddChild(mCancelButton);
}

void SceneImportWidget::Tick(float deltaTime)
{
    Widget::Tick(deltaTime);

    if (Renderer::Get()->GetModalWidget() == this &&
        !ContainsMouse() &&
        (IsMouseButtonJustUp(MOUSE_LEFT) || IsMouseButtonJustUp(MOUSE_RIGHT)))
    {
        Renderer::Get()->SetModalWidget(nullptr);
    }
}

#endif
