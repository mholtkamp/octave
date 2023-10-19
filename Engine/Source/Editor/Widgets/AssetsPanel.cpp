#if EDITOR

#include "Nodes/Widgets/AssetsPanel.h"
#include "Nodes/Widgets/AssetButton.h"
#include "Nodes/Widgets/PropertiesPanel.h"
#include "Nodes/Widgets/TextEntry.h"
#include "Nodes/Widgets/ActionList.h"
#include "AssetManager.h"
#include "AssetDir.h"
#include "Engine.h"
#include "EditorUtils.h"
#include "EditorConstants.h"
#include "Renderer.h"
#include "World.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/TextField.h"
#include "EditorState.h"
#include "ActionManager.h"
#include "PanelManager.h"
#include "InputDevices.h"
#include "Assets/ParticleSystem.h"
#include "Assets/Level.h"
#include "Assets/Blueprint.h"
#include "Assets/SoundWave.h"
#include "Assets/SkeletalMesh.h"
#include "Assets/WidgetMap.h"

static AssetStub* sActionContextAssetStub = nullptr;
static AssetDir* sActionContextAssetDir = nullptr;

void ClearContext()
{
    sActionContextAssetStub = nullptr;
    sActionContextAssetDir = nullptr;
}

void ClearModal()
{
    Renderer::Get()->SetModalWidget(nullptr);
}

void AssetsPanel::HandleCreateFolder(TextField* tf)
{
    const std::string& folderName = tf->GetTextString();

    if (folderName != "")
    {
        AssetDir* dir = PanelManager::Get()->GetAssetsPanel()->GetDirectory();

        if (SYS_CreateDirectory((dir->mPath + folderName).c_str()))
        {
            dir->CreateSubdirectory(folderName);
        }
        else
        {
            LogError("Failed to create folder");
        }
    }

    ClearContext();
    ClearModal();
}

void AssetsPanel::HandleRename(TextField* tf)
{
    const std::string& newName = tf->GetTextString();

    if (newName != "")
    {
        if (sActionContextAssetStub != nullptr)
        {
            Asset* asset = AssetManager::Get()->LoadAsset(*sActionContextAssetStub);
            AssetManager::Get()->RenameAsset(asset, newName);
            AssetManager::Get()->SaveAsset(*sActionContextAssetStub);
        }
        else if (sActionContextAssetDir != nullptr)
        {
            AssetManager::Get()->RenameDirectory(sActionContextAssetDir, newName);
        }
    }

    ClearContext();
    ClearModal();
}

void AssetsPanel::HandleChangeClass(Button* button)
{
    const std::string& className = button->GetTextString();

    const std::vector<Factory*>& actorFactories = Actor::GetFactoryList();

    TypeId newType = INVALID_TYPE_ID;
    for (uint32_t i = 0; i < actorFactories.size(); ++i)
    {
        if (className == actorFactories[i]->GetClassName())
        {
            newType = actorFactories[i]->GetType();
            break;
        }
    }

    if (newType != INVALID_TYPE_ID)
    {
        Asset* asset = AssetManager::Get()->LoadAsset(*sActionContextAssetStub);
        Blueprint* blueprint = asset ? asset->As<Blueprint>() : nullptr;

        if (blueprint)
        {
            TypeId prevType = blueprint->GetActorType();
            std::string prevName = "???";
            for (uint32_t i = 0; i < actorFactories.size(); ++i)
            {
                if (actorFactories[i]->GetType() == prevType)
                {
                    prevName = actorFactories[i]->GetClassName();
                    break;
                }
            }

            blueprint->ChangeActorType(newType);
            LogDebug("Changed actor class from [%s] to [%s]", prevName.c_str(), className.c_str());
        }
    }
    else
    {
        LogError("Failed to change class, invalid type id.");
    }

    ClearContext();
    Renderer::Get()->SetModalWidget(nullptr);
}


void AssetsPanel::ActionListHandler(Button* button)
{
    bool clearContext = true;
    bool clearModal = true;
    const std::string& buttonText = button->GetTextString();
    AssetsPanel* assetsPanel = PanelManager::Get()->GetAssetsPanel();

    if (buttonText == "Properties")
    {
        if (sActionContextAssetStub != nullptr)
        {
            if (sActionContextAssetStub->mAsset == nullptr)
            {
                AssetManager::Get()->LoadAsset(*sActionContextAssetStub);
            }

            PanelManager::Get()->GetPropertiesPanel()->InspectAsset(sActionContextAssetStub->mAsset);
        }
    }
    else if (buttonText == "Save")
    {
        if (sActionContextAssetStub != nullptr)
        {
            if (sActionContextAssetStub->mAsset == nullptr)
            {
                AssetManager::Get()->LoadAsset(*sActionContextAssetStub);
            }

            AssetManager::Get()->SaveAsset(*sActionContextAssetStub);
        }
    }
    else if (buttonText == "Delete")
    {
        if (sActionContextAssetStub != nullptr)
        {
            ActionManager::Get()->DeleteAsset(sActionContextAssetStub);
        }
        else if (sActionContextAssetDir != nullptr)
        {
            ActionManager::Get()->DeleteAssetDir(sActionContextAssetDir);
            assetsPanel->ClearDirHistory();
        }
    }
    else if (buttonText == "Save Level")
    {
        AssetStub* saveStub = nullptr;
        if (sActionContextAssetStub != nullptr &&
            sActionContextAssetStub->mType == Level::GetStaticType())
        {
            saveStub = sActionContextAssetStub;
        }

        assetsPanel->SaveLevel(saveStub);
    }
    else if (buttonText == "Save Blueprint")
    {
        Actor* bpActor = nullptr;
        if (GetEditorMode() == EditorMode::Blueprint)
        {
            bpActor = GetEditBlueprintActor();
        }
        else
        {
            bpActor = GetSelectedActor();
        }

        if (bpActor != nullptr)
        {
            AssetStub* saveStub = nullptr;
            if (sActionContextAssetStub != nullptr &&
                sActionContextAssetStub->mType == Blueprint::GetStaticType())
            {
                saveStub = sActionContextAssetStub;
            }

            assetsPanel->SaveBlueprint(saveStub, bpActor);
        }
    }
    else if (buttonText == "Duplicate")
    {
        assetsPanel->DuplicateAsset(sActionContextAssetStub);
    }
    else if (buttonText == "New Folder")
    {
        ShowTextPrompt("Create Folder", HandleCreateFolder);
        clearContext = false;
        clearModal = false;
    }
    else if (buttonText == "Rename")
    {
        const char* defaultText = nullptr;

        if (sActionContextAssetStub != nullptr)
        {
            defaultText = sActionContextAssetStub->mName.c_str();
        }
        else if (sActionContextAssetDir != nullptr)
        {
            defaultText = sActionContextAssetDir->mName.c_str();
        }

        ShowTextPrompt("Rename", HandleRename, defaultText);
        clearContext = false;
        clearModal = false;
    }
    else if (buttonText == "Load Level")
    {
        assetsPanel->LoadLevel(sActionContextAssetStub);
    }
    else if (buttonText == "Set Startup Level")
    {
        GetEditorState()->mStartupLevelName = sActionContextAssetStub->mName;
        WriteEditorSave();
    }
    else if (buttonText == "Import Asset")
    {
        ActionManager::Get()->ImportAsset();
    }
    else if (buttonText == "Create Asset")
    {
        ActionList* actionList = GetActionList();
        std::vector<std::string> actions;
        actions.push_back("Material");
        actions.push_back("Particle System");
        actions.push_back("Widget Map");
        actions.push_back("Blueprint");
        actionList->SetActions(actions, HandleCreateAsset);

        clearModal = false;
        clearContext = false;
    }
    else if (buttonText == "Edit")
    {
        if (!IsPlayingInEditor() && sActionContextAssetStub)
        {
            AssetStub* stub = sActionContextAssetStub;
            Asset* asset = AssetManager::Get()->LoadAsset(*stub);

            if (stub->mType == WidgetMap::GetStaticType())
            {
                WidgetMap* widgetMap = asset->As<WidgetMap>();

                OCT_ASSERT(widgetMap);
                SetActiveWidgetMap(widgetMap);

                SetEditorMode(EditorMode::Widget);
            }
            else if (stub->mType == Blueprint::GetStaticType())
            {
                Blueprint* bp = asset->As<Blueprint>();

                OCT_ASSERT(bp);
                SetActiveBlueprint(bp);

                SetEditorMode(EditorMode::Blueprint);
            }
        }

    }
    else if (buttonText == "Instantiate")
    {
        if (sActionContextAssetStub &&
            GetEditorMode() == EditorMode::Widget)
        {
            AssetStub* stub = sActionContextAssetStub;
            Asset* asset = AssetManager::Get()->LoadAsset(*stub);
            WidgetMap* widgetMap = asset->As<WidgetMap>();
            OCT_ASSERT(widgetMap);

            EditorInstantiateMappedWidget(widgetMap);
        }
    }
    else if (buttonText == "Save Widget Map")
    {
        if (GetEditorMode() == EditorMode::Widget)
        {
            AssetStub* saveStub = nullptr;
            if (sActionContextAssetStub != nullptr &&
                sActionContextAssetStub->mType == WidgetMap::GetStaticType())
            {
                saveStub = sActionContextAssetStub;
            }

            assetsPanel->SaveWidgetMap(saveStub, GetEditRootWidget());
        }
    }
    else if (buttonText == "Spawn Actor")
    {
        if (sActionContextAssetStub)
        {
            ActionManager* am = ActionManager::Get();
            AssetStub* stub = sActionContextAssetStub;
            Asset* asset = AssetManager::Get()->LoadAsset(*stub);

            glm::vec3 spawnPos = EditorGetFocusPosition();
            
            if (stub->mType == Blueprint::GetStaticType())
                am->SpawnBasicActor(BASIC_BLUEPRINT, spawnPos, asset);
            else if (stub->mType == SoundWave::GetStaticType())
                am->SpawnBasicActor(BASIC_AUDIO, spawnPos, asset);
            else if (stub->mType == StaticMesh::GetStaticType())
                am->SpawnBasicActor(BASIC_STATIC_MESH, spawnPos, asset);
            else if (stub->mType == SkeletalMesh::GetStaticType())
                am->SpawnBasicActor(BASIC_SKELETAL_MESH, spawnPos, asset);
            else if (stub->mType == ParticleSystem::GetStaticType())
                am->SpawnBasicActor(BASIC_PARTICLE, spawnPos, asset);
        }
    }
    else if (buttonText == "Change Class")
    {
        std::vector<std::string> actions;

        GatherAllClassNames(actions);
        GetActionList()->SetActions(actions, HandleChangeClass);

        clearContext = false;
        clearModal = false;
    }

    if (clearContext)
    {
        ClearContext();
    }

    if (clearModal)
    {
        Renderer::Get()->SetModalWidget(nullptr);
    }
}

void AssetsPanel::HandleCreateAsset(Button* button)
{
    const std::string& buttonText = button->GetTextString();
    AssetsPanel* assetsPanel = PanelManager::Get()->GetAssetsPanel();

    if (buttonText == "Material")
    {
        assetsPanel->CreateMaterial();
    }
    else if (buttonText == "Particle System")
    {
        assetsPanel->CreateParticleSystem();
    }
    else if (buttonText == "Widget Map")
    {
        assetsPanel->CreateWidgetMap();
    }
    else if (buttonText == "Blueprint")
    {
        assetsPanel->CreateBlueprint();
    }

    ClearContext();
    Renderer::Get()->SetModalWidget(nullptr);
}

void OnAssetButtonPressed(Button* button)
{
    if (button != nullptr)
    {
        AssetStub* selectedAssetStub = GetSelectedAssetStub();

        AssetButton* assetButton = static_cast<AssetButton*>(button);
        AssetStub* stub = assetButton->GetAssetStub();
        AssetDir* dir = assetButton->GetAssetDir();

        if (dir != nullptr)
        {
            assetButton->GetPanel()->SetDirectory(dir);
            SetSelectedAssetStub(nullptr);
        }
        else if (selectedAssetStub != stub)
        {
            SetSelectedAssetStub(stub);
        }
        else if (!IsControlDown())
        {
            SetSelectedAssetStub(nullptr);
        }

        if (IsControlDown() &&
            stub != nullptr &&
            stub->mAsset != nullptr)
        {
            PanelManager::Get()->GetPropertiesPanel()->InspectAsset(stub->mAsset);
        }
    }
}

AssetsPanel::AssetsPanel() :
    mCurrentDir(nullptr)
{
    SetTitle("Assets");
    SetAnchorMode(AnchorMode::LeftStretch);
    SetYRatio(0.5f);
    SetHeightRatio(0.5f);

    mScrollMultiplier = 5;
}

AssetsPanel::~AssetsPanel()
{

}

void AssetsPanel::Update()
{
    float canvasHeight = mBodyCanvas->GetHeight();
    float buttonHeight = AssetButton::sAssetButtonHeight;
    int32_t numAvailableButtons = int32_t(canvasHeight / buttonHeight);
    OCT_ASSERT(numAvailableButtons < 100); // Something might be wrong?

    while (mAssetButtons.size() < numAvailableButtons)
    {
        int32_t i = (int32_t)mAssetButtons.size();

        AssetButton* button = new AssetButton();
        mAssetButtons.push_back(button);
        mBodyCanvas->AddChild(button);
        button->SetAssetStub(nullptr);
        button->SetPosition(0, i * mAssetButtons[i]->GetHeight());
        button->SetPressedHandler(OnAssetButtonPressed);
        button->SetPanel(this);
    }

    if (mCurrentDir)
    {
        const std::vector<AssetDir*>& dirs = mCurrentDir->mChildDirs;
        const std::vector<AssetStub*>& assets = mCurrentDir->mAssetStubs;
        const int32_t parentDirCount = (mCurrentDir->mParentDir != nullptr) ? 1 : 0;
        const int32_t numDirs = int32_t(dirs.size());
        const int32_t numAssets = int32_t(assets.size());

        SetMaxScroll(int32_t(parentDirCount + numDirs + numAssets) - 1);
        int32_t listOffset = mScroll;


        for (int32_t i = 0; i < mAssetButtons.size(); ++i)
        {
            int32_t index = listOffset + i;

            if (index < parentDirCount)
            {
                mAssetButtons[i]->SetAssetDir(mCurrentDir->mParentDir, true);
            }
            else if (index < parentDirCount + numDirs)
            {
                // This asset button is really a directory.
                mAssetButtons[i]->SetAssetDir(dirs[index - parentDirCount]);
            }
            else if (index < parentDirCount + numDirs + numAssets)
            {
                mAssetButtons[i]->SetAssetStub(assets[index - numDirs - parentDirCount]);
            }
            else
            {
                mAssetButtons[i]->SetAssetStub(nullptr);
            }
        }
    }
    else
    {
        for (int32_t i = 0; i < mAssetButtons.size(); ++i)
        {
            mAssetButtons[i]->SetAssetStub(nullptr);
        }
    }

    Panel::Update();
}

void AssetsPanel::HandleInput()
{
    Panel::HandleInput();

    if (IsMouseInsidePanel() &&
        ShouldHandleInput())
    {
        if (IsKeyJustDown(KEY_M) &&
            mCurrentDir != nullptr)
        {
            CreateMaterial();
        }

        if (IsKeyJustDown(KEY_P) &&
            mCurrentDir != nullptr)
        {
            CreateParticleSystem();
        }

        if (IsKeyJustDown(KEY_L) &&
            mCurrentDir != nullptr)
        {
            if (IsControlDown())
            {
                // Load Level
                if (GetSelectedAsset() != nullptr &&
                    GetSelectedAsset()->GetType() == Level::GetStaticType())
                {
                    LoadLevel(GetSelectedAssetStub());
                }
            }
            else
            {
                // Save Level
                AssetStub* stub = nullptr;

                if (IsShiftDown() &&
                    GetSelectedAsset() != nullptr &&
                    GetSelectedAsset()->GetType() == Level::GetStaticType())
                {
                    stub = GetSelectedAssetStub();
                }

                SaveLevel(stub);
            }
        }

        if (IsKeyJustDown(KEY_B) &&
            mCurrentDir != nullptr)
        {
            Actor* srcActor = GetSelectedActor();
            if (srcActor != nullptr)
            {
                AssetStub* stub = nullptr;

                if (IsShiftDown() && 
                    GetSelectedAsset() != nullptr &&
                    GetSelectedAsset()->GetType() == Blueprint::GetStaticType())
                {
                    stub = GetSelectedAssetStub();
                }

                SaveBlueprint(stub, srcActor);
            }
        }

        if (IsKeyDown(KEY_D) && IsControlDown())
        {
            AssetStub* srcStub = GetSelectedAssetStub();

            if (srcStub != nullptr)
            {
                DuplicateAsset(srcStub);
            }
        }

        if (IsKeyJustDown(KEY_DELETE) || IsKeyJustDown(KEY_X))
        {
            AssetStub* selAsset = GetSelectedAssetStub();

            if (selAsset != nullptr)
            {
                ActionManager::Get()->DeleteAsset(selAsset);
            }
        }

        if (IsKeyJustDown(KEY_F))
        {
            ShowTextPrompt("Create Folder", HandleCreateFolder);
        }

        if (IsMouseButtonJustUp(MOUSE_RIGHT) &&
            ContainsMouse() &&
            mCurrentDir != nullptr &&
            mCurrentDir != AssetManager::Get()->GetRootDirectory())
        {
            bool assetButton = false;
            AssetStub* stub = nullptr;
            AssetDir* dir = nullptr;

            // Check if asset button is selected
            for (uint32_t i = 0; i < mAssetButtons.size(); ++i)
            {
                if (mAssetButtons[i]->IsVisible() &&
                    mAssetButtons[i]->ContainsMouse() &&
                    !mAssetButtons[i]->IsParentDir())
                {
                    assetButton = true;
                    stub = mAssetButtons[i]->GetAssetStub();
                    dir = mAssetButtons[i]->GetAssetDir();
                    OCT_ASSERT(stub != nullptr || dir != nullptr);
                    break;
                }
            }

            bool engineFile = false;
            if ((stub && stub->mEngineAsset) ||
                (dir && dir->mEngineDir))
            {
                engineFile = true;
            }

            bool canInstantiate = false;
            if (stub &&
                (stub->mType == Blueprint::GetStaticType() ||
                    stub->mType == SoundWave::GetStaticType() ||
                    stub->mType == StaticMesh::GetStaticType() ||
                    stub->mType == SkeletalMesh::GetStaticType() ||
                    stub->mType == ParticleSystem::GetStaticType()))
            {
                canInstantiate = true;
            }

            if (GetEditorMode() == EditorMode::Blueprint)
            {
                canInstantiate = false;
            }

            sActionContextAssetStub = stub;
            sActionContextAssetDir = dir;

            ActionList* actionList = GetActionList();
            std::vector<std::string> actions;

            if (assetButton)
            {
                if (sActionContextAssetStub)
                {
                    actions.push_back("Properties");
                }

                if (sActionContextAssetStub &&
                    (sActionContextAssetStub->mType == WidgetMap::GetStaticType() ||
                    sActionContextAssetStub->mType == Blueprint::GetStaticType()))
                {
                    actions.push_back("Edit");

                    if (GetEditorMode() == EditorMode::Widget && 
                        sActionContextAssetStub->mType == WidgetMap::GetStaticType())
                    {
                        actions.push_back("Instantiate");
                    }
                }

                if (!engineFile)
                {
                    actions.push_back("Save");
                    actions.push_back("Rename");
                    actions.push_back("Delete");

                    if (sActionContextAssetStub)
                    {
                        actions.push_back("Duplicate");
                    }
                }

                if (canInstantiate)
                {
                    actions.push_back("Spawn Actor");
                }

                if (sActionContextAssetStub &&
                    sActionContextAssetStub->mType == Level::GetStaticType())
                {
                    actions.push_back("Load Level");
                    actions.push_back("Set Startup Level");
                }

                if (sActionContextAssetStub &&
                    sActionContextAssetStub->mType == Blueprint::GetStaticType())
                {
                    actions.push_back("Change Class");
                }
            }

            if (!mCurrentDir->mEngineDir)
            {
                actions.push_back("Import Asset");
                actions.push_back("Create Asset");
                actions.push_back("New Folder");

                EditorMode editorMode = GetEditorMode();

                switch (editorMode)
                {
                case EditorMode::Level:
                    actions.push_back("Save Level");

                    if (GetSelectedActor() != nullptr)
                    {
                        actions.push_back("Save Blueprint");
                    }
                    break;
                case EditorMode::Widget:
                    actions.push_back("Save Widget Map");
                    break;

                case EditorMode::Blueprint:
                    actions.push_back("Save Blueprint");
                    break;
                default: break;
                }
            }

            if (actions.size() > 0)
            {
                actionList->SetActions(actions, ActionListHandler);
            }
        }

        if (IsMouseButtonJustDown(MOUSE_X1))
        {
            RegressDirPast();
        }
        else if (IsMouseButtonJustDown(MOUSE_X2))
        {
            ProgressDirFuture();
        }
    }
}

void AssetsPanel::SetDirectory(AssetDir* dir, bool recordHistory)
{
    if (mCurrentDir != dir)
    {
        if (recordHistory && mCurrentDir != nullptr)
        {
            mDirPast.push_back(mCurrentDir);
            mDirFuture.clear();
        }

        mCurrentDir = dir;
    }
}

AssetDir* AssetsPanel::GetDirectory()
{
    return mCurrentDir;
}

void AssetsPanel::BrowseToAsset(const std::string& name)
{
    AssetStub* stub = AssetManager::Get()->GetAssetStub(name);

    if (stub != nullptr)
    {
        SetDirectory(stub->mDirectory);
        SetSelectedAssetStub(stub);

        const std::vector<AssetDir*>& dirs = mCurrentDir->mChildDirs;
        const std::vector<AssetStub*>& assets = mCurrentDir->mAssetStubs;
        const int32_t parentDirCount = (mCurrentDir->mParentDir != nullptr) ? 1 : 0;
        const int32_t numDirs = int32_t(dirs.size());
        const int32_t numAssets = int32_t(assets.size());
        SetMaxScroll(int32_t(parentDirCount + numDirs + numAssets) - 1);

        for (uint32_t i = 0; i < assets.size(); ++i)
        {
            if (assets[i] == stub)
            {
                SetScroll(i + parentDirCount + numDirs);
                break;
            }
        }
    }
}

void AssetsPanel::OnProjectDirectorySet()
{
    // Set the current asset directory to the root directory.
    ClearDirHistory();
    SetDirectory(AssetManager::Get()->FindProjectDirectory());
    SetSelectedAssetStub(nullptr);
}

void AssetsPanel::CreateMaterial()
{
    AssetStub* stub = EditorAddUniqueAsset("M_Material", mCurrentDir, Material::GetStaticType(), true);

    if (stub != nullptr &&
        stub->mAsset != nullptr &&
        GetSelectedAsset() != nullptr &&
        GetSelectedAsset()->GetType() == Texture::GetStaticType())
    {
        Material* material = stub->mAsset->As<Material>();
        Texture* texture = GetSelectedAsset()->As<Texture>();

        // Auto assign the selected texture to Texture_0
        material->SetTexture(TEXTURE_0, texture);

        std::string newMatName = texture->GetName();

        if (newMatName.length() >= 2 && newMatName[0] == 'T' && newMatName[1] == '_')
        {
            newMatName[0] = 'M';
        }
        else
        {
            newMatName = std::string("M_") + newMatName;
        }

        AssetManager::Get()->RenameAsset(material, newMatName);
    }

    AssetManager::Get()->SaveAsset(*stub);
}

void AssetsPanel::CreateParticleSystem()
{
    AssetStub* stub = EditorAddUniqueAsset("P_Particle", mCurrentDir, ParticleSystem::GetStaticType(), true);
    AssetManager::Get()->SaveAsset(*stub);
}

void AssetsPanel::CreateWidgetMap()
{
    AssetStub* stub = EditorAddUniqueAsset("W_Widget", mCurrentDir, WidgetMap::GetStaticType(), true);
    AssetManager::Get()->SaveAsset(*stub);
}

void AssetsPanel::CreateBlueprint()
{
    AssetStub* stub = EditorAddUniqueAsset("BP_Blueprint", mCurrentDir, Blueprint::GetStaticType(), true);
    AssetManager::Get()->SaveAsset(*stub);
}

void AssetsPanel::SaveLevel(AssetStub* levelStub)
{
    if (levelStub == nullptr)
    {
        levelStub = EditorAddUniqueAsset("L_Level", mCurrentDir, Level::GetStaticType(), true);
    }

    if (levelStub->mAsset == nullptr)
    {
        AssetManager::Get()->LoadAsset(*levelStub);
    }

    Level* newLevel = (Level*)levelStub->mAsset;
    newLevel->CaptureWorld(GetWorld());
    AssetManager::Get()->SaveAsset(*levelStub);

    SetActiveLevel(newLevel);
}

void AssetsPanel::SaveBlueprint(AssetStub* bpStub, Actor* srcActor)
{
    if (srcActor == nullptr)
    {
        LogError("Invalid actor.");
        return;
    }

    if (!srcActor->DoComponentsHaveUniqueNames())
    {
        LogError("Failed to save Blueprint: Components must have unique names.");
        return;
    }

    if (bpStub == nullptr)
    {
        bpStub = EditorAddUniqueAsset("BP_Blueprint", mCurrentDir, Blueprint::GetStaticType(), true);
    }

    if (bpStub->mAsset == nullptr)
    {
        AssetManager::Get()->LoadAsset(*bpStub);
    }

    Blueprint* bp = (Blueprint*)bpStub->mAsset;

    // Create will overwrite existing data
    bp->Create(srcActor);
    AssetManager::Get()->SaveAsset(*bpStub);

    if (GetEditorMode() == EditorMode::Blueprint)
    {
        SetActiveBlueprint(bp);
    }
}

void AssetsPanel::SaveWidgetMap(AssetStub* stub, Widget* srcWidget)
{
    if (stub == nullptr)
    {
        stub = EditorAddUniqueAsset("W_Widget", mCurrentDir, WidgetMap::GetStaticType(), true);
    }

    if (stub->mAsset == nullptr)
    {
        AssetManager::Get()->LoadAsset(*stub);
    }

    WidgetMap* newMap = (WidgetMap*)stub->mAsset;
    newMap->Create(srcWidget);
    AssetManager::Get()->SaveAsset(*stub);

    SetActiveWidgetMap(newMap);
}

void AssetsPanel::DuplicateAsset(AssetStub* srcStub)
{
    if (srcStub != nullptr)
    {
        Asset* srcAsset = nullptr;
        if (srcStub->mAsset == nullptr)
        {
            AssetManager::Get()->LoadAsset(*srcStub);
        }

        srcAsset = srcStub->mAsset;

        if (srcAsset != nullptr)
        {
            AssetStub* stub = EditorAddUniqueAsset(srcAsset->GetName().c_str(), mCurrentDir, srcAsset->GetType(), false);

            if (stub != nullptr)
            {
                stub->mAsset->Copy(srcAsset);
                stub->mAsset->SetName(stub->mName);
                stub->mAsset->Create();
                AssetManager::Get()->SaveAsset(*stub);
            }
        }
    }
}

void AssetsPanel::LoadLevel(AssetStub* stub)
{
    if (stub != nullptr &&
        stub->mType == Level::GetStaticType())
    {
        ActionManager::Get()->ClearWorld();
        Level* level = (Level*) AssetManager::Get()->LoadAsset(*stub);
        level->LoadIntoWorld(GetWorld());
        SetActiveLevel(level);
    }
}

void AssetsPanel::ClearDirHistory()
{
    mDirFuture.clear();
    mDirPast.clear();
}

void AssetsPanel::ProgressDirFuture()
{
    if (mDirFuture.size() > 0)
    {
        if (mCurrentDir != nullptr)
        {
            mDirPast.push_back(mCurrentDir);
        }

        AssetDir* dir = mDirFuture.back();
        OCT_ASSERT(dir);
        mDirFuture.pop_back();

        SetDirectory(dir, false);
    }
}

void AssetsPanel::RegressDirPast()
{
    if (mDirPast.size() > 0)
    {
        if (mCurrentDir != nullptr)
        {
            // Record the current dir.
            mDirFuture.push_back(mCurrentDir);
        }

        AssetDir* dir = mDirPast.back();
        OCT_ASSERT(dir);
        mDirPast.pop_back();

        SetDirectory(dir, false);
    }
}

#endif
