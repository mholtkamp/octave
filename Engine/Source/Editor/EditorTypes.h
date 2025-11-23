#pragma once

#if EDITOR

#include "EngineTypes.h"

struct SceneImportOptions
{
    std::string mFilePath;
    std::string mPrefix;
    bool mCleanDirectory = true;
    bool mClearWorld = true;
    bool mImportMeshes = true;
    bool mImportMaterials = true;
    bool mImportTextures = true;
    bool mImportLights = true;
    bool mImportNodes = true;
    ShadingModel mDefaultShadingModel = ShadingModel::Lit;
    VertexColorMode mDefaultVertexColorMode = VertexColorMode::None;
};

#endif
