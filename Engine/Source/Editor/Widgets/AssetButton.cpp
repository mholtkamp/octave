#if EDITOR

#include "Nodes/Widgets/AssetButton.h"
#include "Asset.h"
#include "AssetDir.h"
#include "Renderer.h"
#include "AssetManager.h"
#include "ActionManager.h"
#include "Nodes/Widgets/Panel.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/ActionList.h"
#include "PanelManager.h"
#include "Nodes/Widgets/PropertiesPanel.h"
#include "EditorState.h"

#include "InputDevices.h"

float AssetButton::sAssetButtonHeight = 22;

AssetButton::AssetButton() :
    mAssetStub(nullptr),
    mAssetDir(nullptr),
    mPanel(nullptr),
    mPrevSelectedStub(false)
{
    SetDimensions(Panel::sDefaultWidth, sAssetButtonHeight);
    UpdateAppearance();
}

AssetButton::~AssetButton()
{

}

void AssetButton::Update()
{
    AssetStub* selectedStub = GetSelectedAssetStub();
    if (mAssetStub != nullptr &&
        mAssetStub == selectedStub)
    {
        if (!mPrevSelectedStub)
        {
            UpdateAppearance();

        }
        mPrevSelectedStub = true;
    }
    else
    {
        if (mPrevSelectedStub)
        {
            UpdateAppearance();
        }
        mPrevSelectedStub = false;
    }

    Button::Update();
}

void AssetButton::SetAssetStub(AssetStub* stub)
{
    mAssetDir = nullptr;
    mParentDir = false;
    mAssetStub = stub;
    SetVisible(mAssetStub != nullptr);
    UpdateAppearance();
}

AssetStub* AssetButton::GetAssetStub()
{
    return mAssetStub;
}

void AssetButton::SetAssetDir(AssetDir* assetDir, bool parentDir)
{
    mAssetStub = nullptr;
    mAssetDir = assetDir;
    mParentDir = parentDir;
    SetVisible(mAssetDir != nullptr);
    UpdateAppearance();
}

AssetDir* AssetButton::GetAssetDir()
{
    return mAssetDir;
}

void AssetButton::SetPanel(AssetsPanel* panel)
{
    mPanel = panel;
}

AssetsPanel* AssetButton::GetPanel()
{
    return mPanel;
}

bool AssetButton::IsParentDir() const
{
    return mParentDir;
}

void AssetButton::UpdateAppearance()
{
    const bool visible = (mAssetStub != nullptr) || (mAssetDir != nullptr);
    SetVisible(visible);

    glm::vec4 fgColor = glm::vec4(1);
    glm::vec4 bgColor = glm::vec4(1);

    if (mAssetStub)
    {
        fgColor = AssetManager::Get()->GetEditorAssetColor(mAssetStub->mType);
        bgColor = fgColor * glm::vec4(1.0f, 1.0f, 1.0f, 0.3f);
        SetTextString(mAssetStub->mName);
    }
    else if (mAssetDir)
    {
        fgColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        bgColor = glm::vec4(0.2f, 0.0f, 0.5f, 0.8f),
        SetTextString(mParentDir ? ".." : mAssetDir->mName);
    }

    if (mAssetStub != nullptr &&
        GetSelectedAssetStub() == mAssetStub)
    {
        GetQuad()->SetColor({ 0.8f, 0.2f, 0.9f, 1.0f },
            { 0.8f, 0.2f, 0.9f, 1.0f },
            { 1.0f, 0.2f, 0.9f, 1.0f },
            { 1.0f, 0.2f, 0.9f, 1.0f });
        SetHoveredColor({ 1, 1, 0.5f, 1 });
        mText->SetColor({ 1, 1, 1, 1 });
    }
    else
    {
        if (mAssetDir != nullptr)
        {
            GetQuad()->SetColor({ 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.2f, 0.2f, 0.8f, 1.0f },
                { 0.5f, 0.2f, 0.8f, 0.5f },
                { 0.5f, 0.2f, 0.8f, 0.5f });
            SetHoveredColor({ 1, 1, 0.5f, 1 });
        }
        else
        {
            GetQuad()->SetColor({ 0, 0, 0, 0 },
                bgColor,
                { 0, 0, 0, 0 },
                bgColor);
            SetHoveredColor(fgColor * 2.0f);
        }

        mText->SetColor(fgColor);
    }
}

#endif
