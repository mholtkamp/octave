#include "Constants.h"

#if LUA_ENABLED

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Engine_Lua.h"
#include "LuaBindings/Script_Lua.h"
#include "LuaBindings/Input_Lua.h"
#include "LuaBindings/Audio_Lua.h"
#include "LuaBindings/Maths_Lua.h"
#include "LuaBindings/Network_Lua.h"
#include "LuaBindings/Renderer_Lua.h"
#include "LuaBindings/System_Lua.h"
#include "LuaBindings/Log_Lua.h"
#include "LuaBindings/World_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Node3d_Lua.h"
#include "LuaBindings/Primitive3d_Lua.h"
#include "LuaBindings/Mesh3d_Lua.h"
#include "LuaBindings/StaticMesh3d_Lua.h"
#include "LuaBindings/SkeletalMesh3d_Lua.h"
#include "LuaBindings/Camera3d_Lua.h"
#include "LuaBindings/Light3d_Lua.h"
#include "LuaBindings/DirectionalLight3d_Lua.h"
#include "LuaBindings/PointLight3d_Lua.h"
#include "LuaBindings/Audio3d_Lua.h"
#include "LuaBindings/Box3d_Lua.h"
#include "LuaBindings/Capsule3d_Lua.h"
#include "LuaBindings/Particle3d_Lua.h"
#include "LuaBindings/ShadowMesh3d_Lua.h"
#include "LuaBindings/InstancedMesh3d_Lua.h"
#include "LuaBindings/TextMesh3d_Lua.h"
#include "LuaBindings/Sphere3d_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Scene_Lua.h"
#include "LuaBindings/Material_Lua.h"
#include "LuaBindings/MaterialInstance_Lua.h"
#include "LuaBindings/MaterialLite_Lua.h"
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
#include "LuaBindings/Poly_Lua.h"
#include "LuaBindings/PolyRect_Lua.h"
#include "LuaBindings/NodeRef_Lua.h"
#include "LuaBindings/Stream_Lua.h"
#include "LuaBindings/TimerManager_Lua.h"

#include "LuaBindings/Misc_Lua.h"

static std::string sOriginalPath;

void BindLuaInterface()
{
    Vector_Lua::Bind();
    Rect_Lua::Bind();
    Engine_Lua::Bind();
    Script_Lua::Bind();
    Input_Lua::Bind();
    Audio_Lua::Bind();
    Log_Lua::Bind();
    Maths_Lua::Bind();
    Network_Lua::Bind();
    Renderer_Lua::Bind();
    System_Lua::Bind();
    World_Lua::Bind();
    AssetManager_Lua::Bind();
    NodeRef_Lua::Bind();
    Stream_Lua::Bind();
    TimerManager_Lua::Bind();

    // Components need to be bound in hierarchy order.
    // Derived classes need to come after parent classes.
    Node_Lua::Bind();
    Node3D_Lua::Bind();
    Primitive3D_Lua::Bind();
    Mesh3D_Lua::Bind();
    StaticMesh3D_Lua::Bind();
    SkeletalMesh3D_Lua::Bind();
    Camera3D_Lua::Bind();
    Light3D_Lua::Bind();
    DirectionalLight3D_Lua::Bind();
    PointLight3D_Lua::Bind();
    Audio3D_Lua::Bind();
    Box3D_Lua::Bind();
    Capsule3D_Lua::Bind();
    Particle3D_Lua::Bind();
    ShadowMesh3D_Lua::Bind();
    InstancedMesh3D_Lua::Bind();
    TextMesh3D_Lua::Bind();
    Sphere3D_Lua::Bind();

    // Assets need to be bound in hierarchy order.
    Asset_Lua::Bind();
    Scene_Lua::Bind();
    Material_Lua::Bind();
    MaterialInstance_Lua::Bind();
    MaterialLite_Lua::Bind();
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
    Poly_Lua::Bind();
    PolyRect_Lua::Bind();

    Misc_Lua::BindMisc();
}

void UpdateLuaPath()
{
    lua_State* L = GetLua();

    if (L != nullptr)
    {
        std::string projectScriptPath = GetEngineState()->mProjectDirectory + "Scripts/?.lua;";
        std::string engineScriptPath = "Engine/Scripts/?.lua;";

        lua_getglobal(L, "package"); // 1
        lua_pushstring(L, sOriginalPath.c_str()); // 2
        lua_pushstring(L, projectScriptPath.c_str()); // 3
        lua_pushstring(L, engineScriptPath.c_str()); // 4

        lua_concat(L, 3); // 2
        lua_setfield(L, -2, "path"); // 1

        lua_pop(L, 1); // pop package table
    }
}

void SetupLuaPath()
{
    // Grab and save the initial path so we can update it if the project changes.
    lua_State* L = GetLua();

    lua_getglobal(L, "package"); // 1
    lua_getfield(L, -1, "path"); // 2

    sOriginalPath = lua_tostring(L, -1);
    sOriginalPath += ";";

    lua_pop(L, 2); // pop package table and path string

    // Then update the package.path variable to include our script folders.
    UpdateLuaPath();
}

#endif
