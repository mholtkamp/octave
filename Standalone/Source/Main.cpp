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

void OctPreInitialize(EngineConfig& config)
{
    config.mStandalone = true;

#if !EDITOR

    config.mWindowWidth = 1280;
    config.mWindowHeight = 720;
    config.mUseAssetRegistry = false;
    config.mVersion = 0;

#if EMBEDDED_ENABLED
    config.mEmbeddedAssetCount = gNumEmbeddedAssets;
    config.mEmbeddedAssets = gEmbeddedAssets;
    config.mEmbeddedScriptCount = gNumEmbeddedScripts;
    config.mEmbeddedScripts = gEmbeddedScripts;
#endif

#endif
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
