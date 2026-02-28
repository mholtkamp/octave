#if EDITOR

// WinSock2 must be included before Windows.h to avoid winsock.h conflict with ASIO
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <Windows.h>
#endif

#include "ControllerServerRoutes.h"
#include "ControllerServer.h"

#include "Engine.h"
#include "World.h"
#include "Log.h"
#include "AssetManager.h"
#include "Assets/Scene.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Node3d.h"
#include "Property.h"
#include "Script.h"
#include "EditorState.h"
#include "ActionManager.h"
#include "EditorUIHookManager.h"
#include "Addons/NativeAddonManager.h"

#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif

#include "crow.h"

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// JSON Helpers
// ---------------------------------------------------------------------------

static crow::json::wvalue ErrorJson(const std::string& msg)
{
    crow::json::wvalue j;
    j["error"] = msg;
    return j;
}

static crow::json::wvalue Vec3ToJson(const glm::vec3& v)
{
    crow::json::wvalue j;
    j[0] = v.x;
    j[1] = v.y;
    j[2] = v.z;
    return j;
}

static glm::vec3 JsonToVec3(const crow::json::rvalue& j)
{
    glm::vec3 v(0.0f);
    if (j.size() >= 3)
    {
        v.x = static_cast<float>(j[0].d());
        v.y = static_cast<float>(j[1].d());
        v.z = static_cast<float>(j[2].d());
    }
    return v;
}

static crow::json::wvalue DatumToJson(const Datum& datum, uint32_t index = 0)
{
    crow::json::wvalue j;
    switch (datum.GetType())
    {
    case DatumType::Integer:
    case DatumType::Short:
    case DatumType::Byte:
        j = datum.GetInteger(index);
        break;
    case DatumType::Float:
        j = datum.GetFloat(index);
        break;
    case DatumType::Bool:
        j = datum.GetBool(index);
        break;
    case DatumType::String:
        j = datum.GetString(index);
        break;
    case DatumType::Vector2D:
    {
        glm::vec2 v2 = datum.GetVector2D(index);
        j[0] = v2.x;
        j[1] = v2.y;
        break;
    }
    case DatumType::Vector:
    {
        glm::vec3 v3 = datum.GetVector(index);
        j[0] = v3.x;
        j[1] = v3.y;
        j[2] = v3.z;
        break;
    }
    case DatumType::Color:
    {
        glm::vec4 c = datum.GetColor(index);
        j[0] = c.r;
        j[1] = c.g;
        j[2] = c.b;
        j[3] = c.a;
        break;
    }
    case DatumType::Asset:
    {
        Asset* a = datum.GetAsset();
        j = a ? a->GetName() : "";
        break;
    }
    default:
        j = "(unsupported type)";
        break;
    }
    return j;
}

static crow::json::wvalue PropertyToJson(const Property& prop)
{
    crow::json::wvalue j;
    j["name"] = prop.mName;
    j["type"] = static_cast<int>(prop.GetType());

    uint32_t count = prop.GetCount();
    if (count == 1)
    {
        j["value"] = DatumToJson(prop, 0);
    }
    else
    {
        crow::json::wvalue arr;
        for (uint32_t i = 0; i < count; ++i)
        {
            arr[i] = DatumToJson(prop, i);
        }
        j["value"] = std::move(arr);
    }
    return j;
}

static crow::json::wvalue NodeToJson(Node* node)
{
    crow::json::wvalue j;
    if (node == nullptr)
    {
        return j;
    }

    j["name"] = node->GetName();
    j["type"] = node->GetTypeName();
    j["active"] = node->IsActive();
    j["visible"] = node->IsVisible();

    if (node->IsNode3D())
    {
        Node3D* n3d = static_cast<Node3D*>(node);
        j["position"] = Vec3ToJson(n3d->GetWorldPosition());
        j["rotation"] = Vec3ToJson(n3d->GetWorldRotationEuler());
        j["scale"] = Vec3ToJson(n3d->GetWorldScale());
    }

    return j;
}

static crow::json::wvalue HierarchyToJson(Node* node)
{
    crow::json::wvalue j = NodeToJson(node);
    if (node == nullptr)
    {
        return j;
    }

    const auto& children = node->GetChildren();
    if (!children.empty())
    {
        crow::json::wvalue childArr;
        for (uint32_t i = 0; i < children.size(); ++i)
        {
            childArr[i] = HierarchyToJson(children[i].Get());
        }
        j["children"] = std::move(childArr);
    }

    return j;
}

static Node* FindNodeByIdentifier(World* world, const std::string& identifier)
{
    if (world == nullptr)
    {
        return nullptr;
    }

    Node* node = world->FindNode(identifier);
    return node;
}

static void LogRequest(ControllerServer* server, const char* method, const char* path)
{
    if (server && server->GetLogRequests())
    {
        LogDebug("[ControllerServer] %s %s", method, path);
    }
}

// ---------------------------------------------------------------------------
// Route Registration
// ---------------------------------------------------------------------------

void RegisterRoutes(void* appPtr, ControllerServer* server)
{
    crow::SimpleApp& app = *static_cast<crow::SimpleApp*>(appPtr);
    // ------------------------------------------------------------------
    // GET /api/scene — Current scene info
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/scene").methods("GET"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "GET", "/api/scene");

        auto future = server->QueueCommand([]() -> std::string
        {
            crow::json::wvalue j;

            EditorState* editorState = GetEditorState();
            EditScene* editScene = editorState->GetEditScene();

            if (editScene && editScene->mSceneAsset.Get() != nullptr)
            {
                j["scene"] = editScene->mSceneAsset.Get()->GetName();
            }
            else
            {
                j["scene"] = "";
            }

            j["playing"] = editorState->mPlayInEditor;
            j["paused"] = editorState->mPaused;

            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/scene/open — Open a scene by name
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/scene/open").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/scene/open");

        std::string body = req.body;
        auto future = server->QueueCommand([body]() -> std::string
        {
            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("name"))
            {
                return ErrorJson("Missing 'name' field").dump();
            }

            std::string sceneName = parsed["name"].s();
            Asset* asset = AssetManager::Get()->GetAsset(sceneName);
            Scene* scene = asset ? asset->As<Scene>() : nullptr;

            if (scene == nullptr)
            {
                return ErrorJson("Scene not found: " + sceneName).dump();
            }

            GetEditorState()->OpenEditScene(scene);

            crow::json::wvalue j;
            j["success"] = true;
            j["scene"] = sceneName;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/scene/save — Save current scene
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/scene/save").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/scene/save");

        auto future = server->QueueCommand([]() -> std::string
        {
            ActionManager::Get()->SaveScene(false);

            crow::json::wvalue j;
            j["success"] = true;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // GET /api/scene/hierarchy — Recursive node tree
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/scene/hierarchy").methods("GET"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "GET", "/api/scene/hierarchy");

        auto future = server->QueueCommand([]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* root = world->GetRootNode();
            crow::json::wvalue j = HierarchyToJson(root);
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/scene/hierarchy — Reparent a node
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/scene/hierarchy").methods("PUT"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "PUT", "/api/scene/hierarchy");

        std::string body = req.body;
        auto future = server->QueueCommand([body]() -> std::string
        {
            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("node") || !parsed.has("parent"))
            {
                return ErrorJson("Missing 'node' or 'parent' field").dump();
            }

            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            std::string nodeName = parsed["node"].s();
            std::string parentName = parsed["parent"].s();

            Node* node = FindNodeByIdentifier(world, nodeName);
            Node* parent = FindNodeByIdentifier(world, parentName);

            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }
            if (parent == nullptr)
            {
                return ErrorJson("Parent not found: " + parentName).dump();
            }

            int32_t childIndex = -1;
            if (parsed.has("index"))
            {
                childIndex = static_cast<int32_t>(parsed["index"].i());
            }

            ActionManager::Get()->EXE_AttachNode(node, parent, childIndex, -1);

            crow::json::wvalue j;
            j["success"] = true;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // GET /api/nodes/<name> — Node info
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>").methods("GET"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "GET", "/api/nodes/<name>");

        std::string nodeName = name;
        auto future = server->QueueCommand([nodeName]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            return NodeToJson(node).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/nodes — Create a node
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/nodes");

        std::string body = req.body;
        auto future = server->QueueCommand([body]() -> std::string
        {
            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("type"))
            {
                return ErrorJson("Missing 'type' field").dump();
            }

            std::string typeName = parsed["type"].s();

            Node* newNode = ActionManager::Get()->EXE_SpawnNode(typeName.c_str());
            if (newNode == nullptr)
            {
                return ErrorJson("Failed to spawn node of type: " + typeName).dump();
            }

            if (parsed.has("name"))
            {
                newNode->SetName(parsed["name"].s());
            }

            if (parsed.has("parent"))
            {
                World* world = GetWorld(0);
                std::string parentName = parsed["parent"].s();
                Node* parent = FindNodeByIdentifier(world, parentName);
                if (parent != nullptr)
                {
                    ActionManager::Get()->EXE_AttachNode(newNode, parent, -1, -1);
                }
            }

            return NodeToJson(newNode).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // DELETE /api/nodes/<name> — Delete a node
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/delete").methods("POST"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "POST", "/api/nodes/<name>/delete");

        std::string nodeName = name;
        auto future = server->QueueCommand([nodeName]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            ActionManager::Get()->EXE_DeleteNode(node);

            crow::json::wvalue j;
            j["success"] = true;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/transform — Set full transform
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/transform").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/transform");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }
            if (!node->IsNode3D())
            {
                return ErrorJson("Node is not a 3D node").dump();
            }

            Node3D* n3d = static_cast<Node3D*>(node);
            auto parsed = crow::json::load(body);
            if (!parsed)
            {
                return ErrorJson("Invalid JSON body").dump();
            }

            if (parsed.has("position"))
            {
                glm::vec3 pos = JsonToVec3(parsed["position"]);
                ActionManager::Get()->EXE_SetWorldPosition(n3d, pos);
            }
            if (parsed.has("rotation"))
            {
                glm::vec3 rot = JsonToVec3(parsed["rotation"]);
                ActionManager::Get()->EXE_SetWorldRotation(n3d, glm::quat(glm::radians(rot)));
            }
            if (parsed.has("scale"))
            {
                glm::vec3 scl = JsonToVec3(parsed["scale"]);
                ActionManager::Get()->EXE_SetWorldScale(n3d, scl);
            }

            return NodeToJson(n3d).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/move — Set position
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/move").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/move");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr || !node->IsNode3D())
            {
                return ErrorJson("3D Node not found: " + nodeName).dump();
            }

            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("position"))
            {
                return ErrorJson("Missing 'position' field").dump();
            }

            Node3D* n3d = static_cast<Node3D*>(node);
            glm::vec3 pos = JsonToVec3(parsed["position"]);
            ActionManager::Get()->EXE_SetWorldPosition(n3d, pos);

            return NodeToJson(n3d).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/rotate — Set rotation
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/rotate").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/rotate");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr || !node->IsNode3D())
            {
                return ErrorJson("3D Node not found: " + nodeName).dump();
            }

            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("rotation"))
            {
                return ErrorJson("Missing 'rotation' field").dump();
            }

            Node3D* n3d = static_cast<Node3D*>(node);
            glm::vec3 rot = JsonToVec3(parsed["rotation"]);
            ActionManager::Get()->EXE_SetWorldRotation(n3d, glm::quat(glm::radians(rot)));

            return NodeToJson(n3d).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/scale — Set scale
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/scale").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/scale");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr || !node->IsNode3D())
            {
                return ErrorJson("3D Node not found: " + nodeName).dump();
            }

            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("scale"))
            {
                return ErrorJson("Missing 'scale' field").dump();
            }

            Node3D* n3d = static_cast<Node3D*>(node);
            glm::vec3 scl = JsonToVec3(parsed["scale"]);
            ActionManager::Get()->EXE_SetWorldScale(n3d, scl);

            return NodeToJson(n3d).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/visibility — Set visibility
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/visibility").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/visibility");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("visible"))
            {
                return ErrorJson("Missing 'visible' field").dump();
            }

            node->SetVisible(parsed["visible"].b());

            crow::json::wvalue j;
            j["success"] = true;
            j["visible"] = node->IsVisible();
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // GET /api/nodes/<name>/properties — All reflected properties
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/properties").methods("GET"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "GET", "/api/nodes/<name>/properties");

        std::string nodeName = name;
        auto future = server->QueueCommand([nodeName]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            std::vector<Property> props;
            node->GatherProperties(props);

            crow::json::wvalue j;
            crow::json::wvalue propArr;
            for (uint32_t i = 0; i < props.size(); ++i)
            {
                propArr[i] = PropertyToJson(props[i]);
            }
            j["properties"] = std::move(propArr);
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/properties — Set a property by name
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/properties").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/properties");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("name") || !parsed.has("value"))
            {
                return ErrorJson("Missing 'name' or 'value' field").dump();
            }

            std::string propName = parsed["name"].s();

            std::vector<Property> props;
            node->GatherProperties(props);

            for (auto& prop : props)
            {
                if (prop.mName == propName)
                {
                    Datum newValue;
                    newValue.SetType(prop.GetType());

                    switch (prop.GetType())
                    {
                    case DatumType::Integer:
                    case DatumType::Short:
                    case DatumType::Byte:
                        newValue.SetInteger(static_cast<int32_t>(parsed["value"].i()));
                        break;
                    case DatumType::Float:
                        newValue.SetFloat(static_cast<float>(parsed["value"].d()));
                        break;
                    case DatumType::Bool:
                        newValue.SetBool(parsed["value"].b());
                        break;
                    case DatumType::String:
                        newValue.SetString(parsed["value"].s());
                        break;
                    case DatumType::Vector:
                        newValue.SetVector(JsonToVec3(parsed["value"]));
                        break;
                    default:
                        return ErrorJson("Unsupported property type for set").dump();
                    }

                    ActionManager::Get()->EXE_EditProperty(
                        node, PropertyOwnerType::Node, propName, 0, newValue);

                    crow::json::wvalue j;
                    j["success"] = true;
                    return j.dump();
                }
            }

            return ErrorJson("Property not found: " + propName).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // GET /api/nodes/<name>/script-properties — Script fields
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/script-properties").methods("GET"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "GET", "/api/nodes/<name>/script-properties");

        std::string nodeName = name;
        auto future = server->QueueCommand([nodeName]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            Script* script = node->GetScript();
            if (script == nullptr)
            {
                crow::json::wvalue j;
                crow::json::wvalue emptyArr;
                j["properties"] = std::move(emptyArr);
                return j.dump();
            }

            const std::vector<Property>& scriptProps = script->GetScriptProperties();

            crow::json::wvalue j;
            crow::json::wvalue propArr;
            for (uint32_t i = 0; i < scriptProps.size(); ++i)
            {
                propArr[i] = PropertyToJson(scriptProps[i]);
            }
            j["properties"] = std::move(propArr);
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // PUT /api/nodes/<name>/script-properties — Set script field
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/nodes/<string>/script-properties").methods("PUT"_method)
    ([server](const crow::request& req, const std::string& name)
    {
        LogRequest(server, "PUT", "/api/nodes/<name>/script-properties");

        std::string nodeName = name;
        std::string body = req.body;
        auto future = server->QueueCommand([nodeName, body]() -> std::string
        {
            World* world = GetWorld(0);
            if (world == nullptr)
            {
                return ErrorJson("No active world").dump();
            }

            Node* node = FindNodeByIdentifier(world, nodeName);
            if (node == nullptr)
            {
                return ErrorJson("Node not found: " + nodeName).dump();
            }

            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("name") || !parsed.has("value"))
            {
                return ErrorJson("Missing 'name' or 'value' field").dump();
            }

            std::string fieldName = parsed["name"].s();

            Script* script = node->GetScript();
            if (script == nullptr)
            {
                return ErrorJson("Node has no script").dump();
            }

            // Find the script property to determine type
            const std::vector<Property>& scriptProps = script->GetScriptProperties();
            for (const auto& prop : scriptProps)
            {
                if (prop.mName == fieldName)
                {
                    Datum newValue;
                    newValue.SetType(prop.GetType());

                    switch (prop.GetType())
                    {
                    case DatumType::Integer:
                        newValue.SetInteger(static_cast<int32_t>(parsed["value"].i()));
                        break;
                    case DatumType::Float:
                        newValue.SetFloat(static_cast<float>(parsed["value"].d()));
                        break;
                    case DatumType::Bool:
                        newValue.SetBool(parsed["value"].b());
                        break;
                    case DatumType::String:
                        newValue.SetString(parsed["value"].s());
                        break;
                    case DatumType::Vector:
                        newValue.SetVector(JsonToVec3(parsed["value"]));
                        break;
                    default:
                        return ErrorJson("Unsupported script property type").dump();
                    }

                    script->SetField(fieldName.c_str(), newValue);

                    crow::json::wvalue j;
                    j["success"] = true;
                    return j.dump();
                }
            }

            return ErrorJson("Script property not found: " + fieldName).dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/play/start — Begin play in editor
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/play/start").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/play/start");

        auto future = server->QueueCommand([]() -> std::string
        {
            EditorState* state = GetEditorState();
            if (!state->mPlayInEditor)
            {
                state->BeginPlayInEditor();
            }

            crow::json::wvalue j;
            j["success"] = true;
            j["playing"] = true;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/play/stop — End play in editor
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/play/stop").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/play/stop");

        auto future = server->QueueCommand([]() -> std::string
        {
            EditorState* state = GetEditorState();
            if (state->mPlayInEditor)
            {
                state->EndPlayInEditor();
            }

            crow::json::wvalue j;
            j["success"] = true;
            j["playing"] = false;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/play/pause — Pause play
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/play/pause").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/play/pause");

        auto future = server->QueueCommand([]() -> std::string
        {
            GetEditorState()->SetPlayInEditorPaused(true);

            crow::json::wvalue j;
            j["success"] = true;
            j["paused"] = true;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/play/resume — Resume play
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/play/resume").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/play/resume");

        auto future = server->QueueCommand([]() -> std::string
        {
            GetEditorState()->SetPlayInEditorPaused(false);

            crow::json::wvalue j;
            j["success"] = true;
            j["paused"] = false;
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // POST /api/assets/import — Import an asset from disk path
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/assets/import").methods("POST"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "POST", "/api/assets/import");

        std::string body = req.body;
        auto future = server->QueueCommand([body]() -> std::string
        {
            auto parsed = crow::json::load(body);
            if (!parsed || !parsed.has("path"))
            {
                return ErrorJson("Missing 'path' field").dump();
            }

            std::string path = parsed["path"].s();
            Asset* imported = ActionManager::Get()->ImportAsset(path);

            if (imported == nullptr)
            {
                return ErrorJson("Failed to import asset: " + path).dump();
            }

            crow::json::wvalue j;
            j["success"] = true;
            j["name"] = imported->GetName();
            j["type"] = imported->GetTypeName();
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // GET /api/preferences — Dump all preferences
    // ------------------------------------------------------------------
    CROW_ROUTE(app, "/api/preferences").methods("GET"_method)
    ([server](const crow::request& req)
    {
        LogRequest(server, "GET", "/api/preferences");

        auto future = server->QueueCommand([]() -> std::string
        {
            crow::json::wvalue j;
            j["controllerServer"]["enabled"] = true;
            j["controllerServer"]["running"] = ControllerServer::Get()->IsRunning();
            return j.dump();
        });

        std::string result = future.get();
        return crow::response(200, "application/json", result);
    });

    // ------------------------------------------------------------------
    // Addon routes — Forward to registered addon callbacks
    // ------------------------------------------------------------------
    CROW_CATCHALL_ROUTE(app)
    ([server](const crow::request& req)
    {
        // Check addon routes
        EditorUIHookManager* hookMgr = EditorUIHookManager::Get();
        if (hookMgr != nullptr)
        {
            const auto& addonRoutes = hookMgr->GetControllerRoutes();
            for (const auto& route : addonRoutes)
            {
                if (route.mPath == req.url && route.mMethod == crow::method_name(req.method))
                {
                    LogRequest(server, route.mMethod.c_str(), route.mPath.c_str());

                    char responseBuffer[4096] = {};
                    route.mCallback(
                        route.mMethod.c_str(),
                        route.mPath.c_str(),
                        req.body.c_str(),
                        responseBuffer,
                        sizeof(responseBuffer),
                        route.mUserData);

                    return crow::response(200, "application/json", std::string(responseBuffer));
                }
            }
        }

        return crow::response(404, "application/json",
            ErrorJson("Unknown endpoint: " + req.url).dump());
    });
}

#endif
