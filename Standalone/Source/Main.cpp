#include <stdint.h>

#undef min
#undef max

#include "Engine.h"
#include "World.h"
#include "Renderer.h"
#include "InputDevices.h"
#include "Log.h"
#include "Assets/Scene.h"
#include "AssetManager.h"

#include "Nodes/Widgets/StatsOverlay.h"

#define EMBEDDED_ENABLED (PLATFORM_DOLPHIN || PLATFORM_3DS)

#if EMBEDDED_ENABLED
#include "../Generated/EmbeddedAssets.h"
#include "../Generated/EmbeddedScripts.h"
extern uint32_t gNumEmbeddedAssets;
#endif

static std::string sProjectName;
static std::string sDefaultScene;

void ReadIni()
{
    Stream iniStream;
    iniStream.ReadFile("Engine.ini", true);

    if (iniStream.GetSize() > 0)
    {
        char key[MAX_PATH_SIZE] = {};
        char value[MAX_PATH_SIZE] = {};

        while (iniStream.Scan("%[^=]=%s\n", key, value) != -1)
        {
            if (strncmp(key, "project", MAX_PATH_SIZE) == 0)
            {
                sProjectName = value;
            }
            else if (strncmp(key, "defaultScene", MAX_PATH_SIZE) == 0)
            {
                sDefaultScene = value;
            }
        }
    }
}

InitOptions OctPreInitialize()
{
    InitOptions initOptions;

    initOptions.mStandalone = true;

#if !EDITOR

    initOptions.mWidth = 1280;
    initOptions.mHeight = 720;
    initOptions.mUseAssetRegistry = false;
    initOptions.mVersion = 0;

#if EMBEDDED_ENABLED
    initOptions.mEmbeddedAssetCount = gNumEmbeddedAssets;
    initOptions.mEmbeddedAssets = gEmbeddedAssets;
    initOptions.mEmbeddedScriptCount = gNumEmbeddedScripts;
    initOptions.mEmbeddedScripts = gEmbeddedScripts;
#endif

    // Open Engine.ini and check for project name
    bool embedded = false;
#if EMBEDDED_ENABLED
    embedded = (gNumEmbeddedAssets > 0);
#endif

    if (!embedded)
    {
        ReadIni();
        initOptions.mProjectName = sProjectName;
    }

    initOptions.mDefaultScene = (sDefaultScene != "") ? sDefaultScene : "SC_Default";

#endif

    return initOptions;
}

void OctPostInitialize()
{

}

void OctPreUpdate()
{

}

void OctPostUpdate()
{

}

void OctPreShutdown()
{

}

void OctPostShutdown()
{

}
