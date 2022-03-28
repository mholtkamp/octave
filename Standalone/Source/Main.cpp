#include <stdint.h>

#undef min
#undef max

#include "Engine.h"
#include "World.h"
#include "Renderer.h"
#include "InputDevices.h"
#include "Log.h"
#include "Assets/Level.h"
#include "AssetManager.h"

#include "Widgets/StatsOverlay.h"

#define EMBEDDED_ENABLED (PLATFORM_DOLPHIN || PLATFORM_3DS)

#if EMBEDDED_ENABLED
#include "../Generated/EmbeddedAssets.h"
#include "../Generated/EmbeddedScripts.h"
extern uint32_t gNumEmbeddedAssets;
#endif

extern CommandLineOptions gCommandLineOptions;

InitOptions OctPreInitialize()
{
    InitOptions initOptions;

#if 1

    initOptions.mStandalone = true;

    initOptions.mWidth = 1280;
    initOptions.mHeight = 720;
    //initOptions.mProjectName;
    initOptions.mUseAssetRegistry = false;
    initOptions.mVersion = 1;

#if EMBEDDED_ENABLED
    // Note to self:
    // If you want to run without embedded assets on these consoles, then:
    // (1) Set EMBED_ALL_ASSETS to 0 in Constants.h
    // (2) Compile and launch the editor. Ctrl+B to build.
    // (3) Move Rocket/Assets to sd:/Rocket/Assets
    // (4) Move Engine/Assets to sd:/Engine/Assets
    // (5) Move Rocket.octp and AssetRegistry.txt into the sd:/Rocket folder
    // (6) Set the initOptions.mUseAssetRegistry to true in this file.
    // (7) On Wii, you might want to set the working directory to the app's folder so 
    //     that you don't clutter the root of the SD card. I forget if on 3DS your working
    //     directory is where you launch the executable from, but on Wii it resets to sd:/
    //     You can set the working directory in the initOptions.mWorkingDirectory
    //     For example: initOptions.mWorkingDirectory = "sd:/apps/RetroLeagueGX";
    // (8) Compile Rocket and place Rocket.dol (or other exe fmt) in normal place on sd card.
    // (9) Launch the executable and hope it works!
    initOptions.mEmbeddedAssetCount = gNumEmbeddedAssets;
    initOptions.mEmbeddedAssets = gEmbeddedAssets;
    initOptions.mEmbeddedScriptCount = gNumEmbeddedScripts;
    initOptions.mEmbeddedScripts = gEmbeddedScripts;
#endif

#if PLATFORM_WII
    initOptions.mWorkingDirectory = "sd://apps/RetroLeagueSD";
#endif

#endif

    return initOptions;
}

void OctPostInitialize()
{
    // Open Engine.ini and check for project name
    bool embedded = false;
#if EMBEDDED_ENABLED
    embedded = (gNumEmbeddedAssets > 0);
#endif

    if (!embedded)
    {
        std::string projName = "";

        FILE* file = fopen("Engine.ini", "r");

        if (file != nullptr)
        {
            char key[MAX_PATH_SIZE] = {};
            char value[MAX_PATH_SIZE] = {};

            while (fscanf(file, "%[^=]=%s\n", key, value) != -1)
            {
                if (strncmp(key, "project", MAX_PATH_SIZE) == 0)
                {
                    projName = value;
                }
            }
        }

        if (projName != "")
        {
            LoadProject(projName + "/" + projName + ".octp");
        }
    }

    if (gCommandLineOptions.mDefaultLevel == "")
    {
        Level* defaultLevel = LoadAsset<Level>("L_Default");
        if (defaultLevel != nullptr)
        {
            defaultLevel->LoadIntoWorld(GetWorld());
        }
    }
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
