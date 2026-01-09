#if EDITOR

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include "Maths.h"
#include "EngineTypes.h"

#include <stdint.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

const aiNode* FindMeshNode(const aiScene* scene, const aiNode* node, const aiMesh* mesh);
std::string GetFileNameFromPath(const std::string& path);
std::string GenerateUniqueMeshName(const std::string& baseName, const aiScene* scene, int32_t meshIndex);

#endif