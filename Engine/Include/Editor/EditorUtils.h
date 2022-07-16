#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <stdint.h>
#include "Maths.h"

#include "EngineTypes.h"
struct AssetStub;
class AssetDir;

void EditorShowCursor(bool show);
void EditorCenterCursor();
void EditorSetCursorPos(int32_t x, int32_t y);
void EditorGetWindowCenter(int32_t& x, int32_t& y);
glm::vec3 EditorGetFocusPosition();
AssetStub* EditorAddUniqueAsset(const char* baseName, AssetDir* dir, TypeId assetType, bool autoCreate);
