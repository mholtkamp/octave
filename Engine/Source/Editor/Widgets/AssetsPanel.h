#pragma once

#include "Panel.h"

class AssetButton;
class AssetDir;
class TextEntry;
class TextField;
struct AssetStub;
class Button;

class AssetsPanel : public Panel
{
public:

    AssetsPanel();
    virtual ~AssetsPanel();

    virtual void Tick(float deltaTime) override;
    virtual void HandleInput() override;

    void SetDirectory(AssetDir* dir, bool recordHistory = true);
    AssetDir* GetDirectory();

    void BrowseToAsset(const std::string& name);

    void OnProjectDirectorySet();

    void CreateMaterial();
    void CreateParticleSystem();
    void CreateWidgetMap();
    void CreateBlueprint();
    void SaveLevel(AssetStub* levelStub);
    void SaveBlueprint(AssetStub* bpStub, Actor* srcActor);
    void SaveWidgetMap(AssetStub* stub, Widget* srcWidget);
    void DuplicateAsset(AssetStub* srcStub);
    void LoadLevel(AssetStub* stub);

    void ClearDirHistory();

protected:

    static void HandleCreateFolder(TextField* tf);
    static void HandleCreateAsset(Button* button);
    static void HandleRename(TextField* tf);
    static void HandleChangeClass(Button* button);
    static void ActionListHandler(Button* button);

    void ProgressDirFuture();
    void RegressDirPast();


    AssetDir* mCurrentDir;
    std::vector<AssetButton*> mAssetButtons;

    std::vector<AssetDir*> mDirPast;
    std::vector<AssetDir*> mDirFuture;
};