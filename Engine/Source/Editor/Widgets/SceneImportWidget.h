#pragma once

#include "Widgets/Widget.h"
#include "Widgets/Button.h"
#include "Widgets/TextField.h"
#include "Widgets/Text.h"
#include "Widgets/Quad.h"
#include "Widgets/CheckBox.h"
#include "EditorTypes.h"

class SceneImportWidget : public Widget
{
public:

    SceneImportWidget();

    static void HandleFilePressed(Button* button);
    static void HandleImportPressed(Button* button);
    static void HandleCancelPressed(Button* button);

    virtual void Update() override;

    Button* mFileButton = nullptr;
    Text* mFileText = nullptr;

    Text* mPrefixLabel = nullptr;
    TextField* mPrefixField = nullptr;

    Text* mCleanDirectoryLabel = nullptr;
    CheckBox* mCleanDirectoryBox = nullptr;

    Text* mClearWorldLabel = nullptr;
    CheckBox* mClearWorldBox = nullptr;

    Text* mImportObjectsLabel = nullptr;
    CheckBox* mImportObjectsBox = nullptr;

    Text* mImportMeshesLabel = nullptr;
    CheckBox* mImportMeshesBox = nullptr;

    Text* mImportMaterialsLabel = nullptr;
    CheckBox* mImportMaterialsBox = nullptr;

    Text* mImportTexturesLabel = nullptr;
    CheckBox* mImportTexturesBox = nullptr;

    Text* mImportLightsLabel = nullptr;
    CheckBox* mImportLightsBox = nullptr;

    Text* mDefaultShadingLabel = nullptr;
    TextField* mDefaultShadingField = nullptr;

    Button* mImportButton = nullptr;
    Button* mCancelButton = nullptr;
};
