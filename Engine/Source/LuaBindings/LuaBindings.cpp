#include "Constants.h"

#if LUA_ENABLED

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Engine_Lua.h"
#include "LuaBindings/Input_Lua.h"
#include "LuaBindings/Audio_Lua.h"
#include "LuaBindings/Maths_Lua.h"
#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/Renderer_Lua.h"
#include "LuaBindings/Log_Lua.h"
#include "LuaBindings/World_Lua.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Component_Lua.h"
#include "LuaBindings/TransformComponent_Lua.h"
#include "LuaBindings/PrimitiveComponent_Lua.h"
#include "LuaBindings/MeshComponent_Lua.h"
#include "LuaBindings/StaticMeshComponent_Lua.h"
#include "LuaBindings/SkeletalMeshComponent_Lua.h"
#include "LuaBindings/CameraComponent_Lua.h"
#include "LuaBindings/LightComponent_Lua.h"
#include "LuaBindings/DirectionalLightComponent_Lua.h"
#include "LuaBindings/PointLightComponent_Lua.h"
#include "LuaBindings/AudioComponent_Lua.h"
#include "LuaBindings/BoxComponent_Lua.h"
#include "LuaBindings/CapsuleComponent_Lua.h"
#include "LuaBindings/ParticleComponent_Lua.h"
#include "LuaBindings/ScriptComponent_Lua.h"
#include "LuaBindings/ShadowMeshComponent_Lua.h"
#include "LuaBindings/SphereComponent_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Blueprint_Lua.h"
#include "LuaBindings/Level_Lua.h"
#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/MaterialInstance_Lua.h"
#include "LuaBindings/ParticleSystem_Lua.h"
#include "LuaBindings/ParticleSystemInstance_Lua.h"
#include "LuaBindings/StaticMesh_Lua.h"
#include "LuaBindings/SoundWave_Lua.h"
#include "LuaBindings/SkeletalMesh_Lua.h"
#include "LuaBindings/Texture_Lua.h"
#include "LuaBindings/Font_Lua.h"
#include "LuaBindings/AssetManager_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Canvas_Lua.h"
#include "LuaBindings/CheckBox_Lua.h"
#include "LuaBindings/ComboBox_Lua.h"
#include "LuaBindings/Selector_Lua.h"
#include "LuaBindings/TextField_Lua.h"
#include "LuaBindings/VerticalList_Lua.h"
#include "LuaBindings/ActorRef_Lua.h"
#include "LuaBindings/ComponentRef_Lua.h"

#include "LuaBindings/Misc_Lua.h"

static std::string sOriginalPath;

void BindLuaInterface()
{
    Vector_Lua::Bind();
    Rect_Lua::Bind();
    Engine_Lua::Bind();
    Input_Lua::Bind();
    Audio_Lua::Bind();
    Log_Lua::Bind();
    Maths_Lua::Bind();
    Network_Lua::Bind();
    Renderer_Lua::Bind();
    World_Lua::Bind();
    AssetManager_Lua::Bind();
    Actor_Lua::Bind();
    ActorRef_Lua::Bind();
    ComponentRef_Lua::Bind();

    // Components need to be bound in hierarchy order.
    // Derived classes need to come after parent classes.
    Component_Lua::Bind();
    TransformComponent_Lua::Bind();
    PrimitiveComponent_Lua::Bind();
    MeshComponent_Lua::Bind();
    StaticMeshComponent_Lua::Bind();
    SkeletalMeshComponent_Lua::Bind();
    CameraComponent_Lua::Bind();
    LightComponent_Lua::Bind();
    DirectionalLightComponent_Lua::Bind();
    PointLightComponent_Lua::Bind();
    AudioComponent_Lua::Bind();
    BoxComponent_Lua::Bind();
    CapsuleComponent_Lua::Bind();
    ParticleComponent_Lua::Bind();
    ScriptComponent_Lua::Bind();
    ShadowMeshComponent_Lua::Bind();
    SphereComponent_Lua::Bind();

    // Assets need to be bound in hierarchy order.
    Asset_Lua::Bind();
    Blueprint_Lua::Bind();
    Level_Lua::Bind();
    Material_Lua::Bind();
    MaterialInstance_Lua::Bind();
    ParticleSystem_Lua::Bind();
    ParticleSystemInstance_Lua::Bind();
    StaticMesh_Lua::Bind();
    SoundWave_Lua::Bind();
    SkeletalMesh_Lua::Bind();
    Texture_Lua::Bind();
    Font_Lua::Bind();

    // Widgets need to be bound in hierarchy order
    Widget_Lua::Bind();
    Quad_Lua::Bind();
    Text_Lua::Bind();
    Button_Lua::Bind();
    Selector_Lua::Bind();
    Canvas_Lua::Bind();
    CheckBox_Lua::Bind();
    ComboBox_Lua::Bind();
    TextField_Lua::Bind();
    VerticalList_Lua::Bind();

    Misc_Lua::BindMisc();

    // Setup a global "world" variable.
    lua_State* L = GetLua();
    World_Lua::Create(L, GetWorld());
    lua_setglobal(L, "world");
}

void UpdateLuaPath()
{
    std::string projectScriptPath = GetEngineState()->mProjectDirectory + "Scripts/?.lua;";
    std::string engineScriptPath = "Engine/Scripts/?.lua;";

    lua_State* L = GetLua();
    GetLua();

    lua_getglobal(L, "package"); // 1
    lua_getfield(L, -1, "path"); // 2
    lua_pushstring(L, projectScriptPath.c_str()); // 3
    lua_pushstring(L, engineScriptPath.c_str()); // 4

    lua_concat(L, 3); // 2
    lua_setfield(L, -2, "path"); // 1

    lua_pop(L, 1); // pop package table
}

void SetupLuaPath()
{
    // Grab and save the initial path so we can update it if the project changes.
    lua_State* L = GetLua();

    lua_getglobal(L, "package"); // 1
    lua_getfield(L, -1, "path"); // 2

    sOriginalPath = lua_tostring(L, -1);

    lua_pop(L, 2); // pop package table and path string

    // Then update the package.path variable to include our script folders.
    UpdateLuaPath();
}

#endif
