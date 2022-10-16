#pragma once

#if EDITOR

#include "EngineTypes.h"

struct SceneImportOptions
{
    std::string mFilePath;
    std::string mPrefix;
    bool mCleanDirectory = false;
    bool mClearWorld = true;
    bool mImportMeshes = true;
    bool mImportMaterials = true;
    bool mImportTextures = true;
    bool mImportLights = false;
    bool mImportObjects = true;
    ShadingModel mDefaultShadingModel = ShadingModel::Lit;
};

#endif
