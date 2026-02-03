#pragma once

#if EDITOR

#include "EngineTypes.h"

struct SceneImportOptions
{
    std::string mFilePath;
    std::string mSceneName;
    std::string mPrefix;
    bool mImportMeshes = true;
    bool mImportMaterials = true;
    bool mImportTextures = true;
    bool mImportLights = true;
    bool mImportCameras = true;
    bool mEnableCollision = true;
    ShadingModel mDefaultShadingModel = ShadingModel::Lit;
    VertexColorMode mDefaultVertexColorMode = VertexColorMode::None;
};

struct CameraImportOptions
{
    std::string mFilePath;
    std::string mCameraName;
    bool mIsMainCamera = false;
    bool mOverrideCameraName = false;
};

#endif
