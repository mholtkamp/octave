#pragma once

#include "Nodes/Widgets/Button.h"

class Asset;
class AssetDir;
class AssetsPanel;
struct AssetStub;

class AssetButton : public Button
{
public:

    AssetButton();
    ~AssetButton();

    virtual void Tick(float deltaTime) override;

    void SetAssetStub(AssetStub* asset);
    AssetStub* GetAssetStub();

    void SetAssetDir(AssetDir* assetDir, bool parentDir = false);
    AssetDir* GetAssetDir();

    void SetPanel(AssetsPanel* panel);
    AssetsPanel* GetPanel();

    bool IsParentDir() const;

    static float sAssetButtonHeight;

protected:

    void UpdateAppearance();

    // Can either point to AssetDir or Asset, not both.
    AssetStub* mAssetStub;
    AssetDir* mAssetDir;

    AssetsPanel* mPanel;

    bool mPrevSelectedStub;
    bool mParentDir;
};