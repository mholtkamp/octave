#include "EngineTypes.h"
#include "Log.h"
#include "Engine.h"

#include "Nodes/Node.h"
#include "Assets/Level.h"
#include "Nodes/Node.h"
#include "Nodes/3D/Primitive3d.h"

#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Actor_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Node3d_Lua.h"

#if LUA_ENABLED

int Actor_Lua::GetScript(lua_State* L)
{
    Actor* actor = CHECK_ACTOR(L, 1);
    const char* name = nullptr;
    
    // If no script name is supplied, just get the first script.
    if (!lua_isnone(L, 2)) { name = CHECK_STRING(L, 2); }
    bool found = false;

    const std::vector<Component*>& comps = actor->GetComponents();
    for (uint32_t i = 0; i < comps.size(); ++i)
    {
        if (comps[i]->GetType() == ScriptComponent::GetStaticType())
        {
            ScriptComponent* sc = static_cast<ScriptComponent*>(comps[i]);

            if (name == nullptr || sc->GetScriptClassName() == name)
            {
                lua_getglobal(L, sc->GetTableName().c_str());
                if (lua_istable(L, -1))
                {
                    found = true;
                    break;
                }
                else
                {
                    // Should not happen? The component has a table registered
                    // but it doesn't exist in the lua state.
                    OCT_ASSERT(0);
                    lua_pop(L, 1);
                }
            }
        }
    }

    if (!found)
    {
        lua_pushnil(L);
    }

    return 1;
}


#endif