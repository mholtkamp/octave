#pragma once

#include <stdint.h>
#include <string>

#include "EngineTypes.h"
#include "Property.h"

extern void OctPreInitialize(EngineConfig& config);
extern void OctPostInitialize();
extern void OctPreUpdate();
extern void OctPostUpdate();
extern void OctPreShutdown();
extern void OctPostShutdown();

bool Initialize();

bool Update();

void Shutdown();

void Quit();

class World* GetWorld(int32_t index);
int32_t GetNumWorlds();

struct EngineState* GetEngineState();
const struct EngineConfig* GetEngineConfig();
struct EngineConfig* GetMutableEngineConfig();

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

void SetScreenOrientation(ScreenOrientation mode);
ScreenOrientation GetScreenOrientation();

void WriteEngineConfig(std::string path = "");
void ReadEngineConfig(std::string path);
void ResetEngineConfig();

void ReadCommandLineArgs(int32_t argc, char** argv);


#if LUA_ENABLED
lua_State* GetLua();
#endif
