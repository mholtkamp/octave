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

    void OnProjectDirectorySet();

    AssetStub* AddUniqueAsset(const char* baseName, TypeId assetType, bool autoCreate);

    void CreateMaterial();
    void CreateParticleSystem();
    void SaveLevel(AssetStub* levelStub);
    void SaveBlueprint(AssetStub* bpStub, Actor* srcActor);
    void DuplicateAsset(AssetStub* srcStub);
    void LoadLevel(AssetStub* stub);

protected:

    static void HandleCreateFolder(TextField* tf);
    static void HandleCreateAsset(Button* button);
    static void HandleRename(TextField* tf);
    static void ActionListHandler(Button* button);


    AssetDir* mCurrentDir;
    std::vector<AssetButton*> mAssetButtons;
    TextEntry* mTextEntryWidget = nullptr;
};