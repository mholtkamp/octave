#pragma once

#include <stdint.h>
#include <string>

#include "EngineTypes.h"

bool Initialize(InitOptions& initOptions);

bool Update();

void Shutdown();

class World* GetWorld();

struct EngineState* GetEngineState();

const class Clock* GetAppClock();

bool IsShuttingDown();

void LoadProject(const std::string& path, bool discoverAssets = true);

void EnableConsole(bool enable);

void ResizeWindow(uint32_t width, uint32_t height);

#if LUA_ENABLED
lua_State* GetLua();
#endif
