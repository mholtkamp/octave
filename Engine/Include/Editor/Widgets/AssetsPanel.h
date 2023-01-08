#pragma once

#include "Panel.h"

class AssetButton;
class AssetDir;
class TextEntry;
class TextField;
struct AssetStub;
class Actor;
class Button;

class AssetsPanel : public Panel
{
public:

    AssetsPanel();
    virtual ~AssetsPanel();

    virtual void Update() override;
    virtual void HandleInput() override;

    void SetDirectory(AssetDir* dir);
    AssetDir* GetDirectory();

    void BrowseToAsset(const std::string& name);

    void OnProjectDirectorySet();

    void CreateMaterial();
    void CreateParticleSystem();
    void CreateWidgetMap();
    void SaveLevel(AssetStub* levelStub);
    void SaveBlueprint(AssetStub* bpStub, Actor* srcActor);
    void SaveWidgetMap(AssetStub* stub, Widget* srcWidget);
    void DuplicateAsset(AssetStub* srcStub);
    void LoadLevel(AssetStub* stub);

protected:

    static void HandleCreateFolder(TextField* tf);
    static void HandleCreateAsset(Button* button);
    static void HandleRename(TextField* tf);
    static void ActionListHandler(Button* button);


    AssetDir* mCurrentDir;
    std::vector<AssetButton*> mAssetButtons;
};