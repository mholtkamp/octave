#pragma once

#include <stdint.h>
#include <string>

#include "EngineTypes.h"
#include "Property.h"

extern InitOptions OctPreInitialize();
extern void OctPostInitialize();
extern void OctPreUpdate();
extern void OctPostUpdate();
extern void OctPreShutdown();
extern void OctPostShutdown();

bool Initialize(InitOptions& initOptions);

bool Update();

void Shutdown();

void Quit();

class World* GetWorld();

struct EngineState* GetEngineState();

const class Clock* GetAppClock();

bool IsShuttingDown();

void LoadProject(const std::string& path, bool discoverAssets = true);

void EnableConsole(bool enable);

void ResizeWindow(uint32_t width, uint32_t height);

bool IsPlayingInEditor();
bool IsPlaying();

bool IsGameTickEnabled();

void ReloadAllScripts(bool restartComponents = true);

void SetPaused(bool paused);
bool IsPaused();
void FrameStep();

void SetTimeDilation(float timeDilation);
float GetTimeDilation();

void GarbageCollect();

void GatherGlobalProperties(std::vector<Property>& props);

#if LUA_ENABLED
lua_State* GetLua();
#endif
