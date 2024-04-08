#include <stdint.h>

#undef min
#undef max

#include "Engine.h"
#include "World.h"
#include "Renderer.h"
#include "InputDevices.h"
#include "Log.h"
#include "DefaultFonts.h"
#include "AssetManager.h"

#include "Widgets/StatsOverlay.h"

#if !EDITOR
#include "../Generated/EmbeddedAssets.h"
#endif

InitOptions OctPreInitialize()
{
    InitOptions initOptions;
    initOptions.mWidth = 1280;
    initOptions.mHeight = 720;
    initOptions.mProjectName = "OctTemplate";
    initOptions.mUseAssetRegistry = false;
    initOptions.mVersion = 1;

#if PLATFORM_DOLPHIN || PLATFORM_3DS
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
#endif

#if PLATFORM_WII
    initOptions.mWorkingDirectory = "sd://apps/DuskSD";
#endif

    return initOptions;
}

void OctPostInitialize()
{
    //FogSettings fogSettings;
    //fogSettings.mColor = { 0.0f, 1.0f, 0.7f, 1.0f };
    //fogSettings.mNear = 20.0f;
    //fogSettings.mFar = 500.0f;
    //fogSettings.mDensityFunc = FogDensityFunc::Linear;
    //fogSettings.mEnabled = true;
    //GetWorld()->SetFogSettings(fogSettings);
    GetWorld()->SetAmbientLightColor({ 0.5f, 0.5f, 0.5f, 1.0f });
}

void OctPreUpdate()
{

}

void OctPostUpdate()
{
    // Exit the game if home is pressed or the Smash Bros Melee reset combo is pressed.
    if (IsGamepadButtonDown(GAMEPAD_HOME, 0) ||
        (IsGamepadButtonDown(GAMEPAD_A, 0) &&
            IsGamepadButtonDown(GAMEPAD_L1, 0) &&
            IsGamepadButtonDown(GAMEPAD_R1, 0) &&
            IsGamepadButtonDown(GAMEPAD_START, 0)))
    {
        exit(0);
    }

    if (IsGamepadButtonDown(GAMEPAD_Z, 0) ||
        IsGamepadButtonDown(GAMEPAD_SELECT, 0))
    {
        StatsOverlay* stats = Renderer::Get()->GetStatsWidget();
        if (IsGamepadButtonJustDown(GAMEPAD_DOWN, 0))
        {
            stats->SetDisplayMode(StatDisplayMode::None);
        }
        else if (IsGamepadButtonJustDown(GAMEPAD_LEFT, 0))
        {
            stats->SetDisplayMode(StatDisplayMode::Network);
        }
        else if (IsGamepadButtonJustDown(GAMEPAD_UP, 0))
        {
            stats->SetDisplayMode(StatDisplayMode::CpuStatText);
        }
        else if (IsGamepadButtonJustDown(GAMEPAD_RIGHT, 0))
        {
            stats->SetDisplayMode(StatDisplayMode::Memory);
        }
    }
}

void OctPreShutdown()
{

}

void OctPostShutdown()
{
    
}
