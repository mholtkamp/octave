#if EDITOR

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <stdint.h>
#include "Maths.h"

#include "EngineTypes.h"
struct AssetStub;
class AssetDir;
class Node;
struct aiMesh;

void EditorCenterCursor();
glm::vec3 EditorGetFocusPosition();
AssetStub* EditorAddUniqueAsset(const char* baseName, AssetDir* dir, TypeId assetType, bool autoCreate);
std::string EditorGetAssetNameFromPath(const std::string& path);

void RemoveRedundantDescendants(std::vector<Node*>& nodes);
std::string GetDevkitproPath();
std::string GetDevenvPath();

bool IsAiCollisionMesh(const aiMesh* mesh);

#endif