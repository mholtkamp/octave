#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <stdint.h>
#include "Maths.h"

#include "EngineTypes.h"
struct AssetStub;
class AssetDir;
class WidgetMap;
class Node;

void EditorCenterCursor();
glm::vec3 EditorGetFocusPosition();
AssetStub* EditorAddUniqueAsset(const char* baseName, AssetDir* dir, TypeId assetType, bool autoCreate);
std::string EditorGetAssetNameFromPath(const std::string& path);

void RemoveRedundantDescendants(std::vector<Node*>& nodes);
