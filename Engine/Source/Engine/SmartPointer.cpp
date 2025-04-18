#include "SmartPointer.h"

#include "Nodes/Node.h"
#include "LuaBindings/Node_Lua.h"


void MakeNodeUserdataStrong(Node* node)
{
    // Is there only one shared ref right now? But we have userdata created?
    // Then that one ref must be the NodePtr in Node_Lua. So now that we 
    // are referencing it in C++, we need to move its weak ref to a strong ref.
    if (node != nullptr &&
        node->IsUserdataCreated())
    {
        int nodeId = (int)node->GetNodeId();

        lua_State* L = GetLua();
        int preTop = lua_gettop(L);

        // We need to move the ref from the weak table to the strong table
        // Strong/Weak tables should be created when first userdata is created in Node_Lua::Create()
        {
            lua_getfield(L, LUA_REGISTRYINDEX, OCT_NODE_WEAK_TABLE_KEY);
            OCT_ASSERT(lua_istable(L, -1));
            int weakTableIdx = lua_gettop(L);

            lua_getfield(L, LUA_REGISTRYINDEX, OCT_NODE_STRONG_TABLE_KEY);
            OCT_ASSERT(lua_istable(L, -1));
            int strongTableIdx = lua_gettop(L);

            // Add strong table ref
            lua_rawgeti(L, weakTableIdx, nodeId);
            lua_rawseti(L, strongTableIdx, nodeId);

            // Pop strong table
            lua_pop(L, 1);

            // Clear weak table ref
            lua_pushnil(L);
            lua_rawseti(L, weakTableIdx, nodeId);

            // Pop weak table
            lua_pop(L, 1);
        }

        int postTop = lua_gettop(L);

        OCT_ASSERT(preTop == postTop);
    }
}

void MakeNodeUserdataWeak(Node* node)
{
    // Are we about to drop to the last SharedPtr?
    // And is it the userdata shared ptr? If so, unref the 
    // userdata in the registry so that it will be garbage
    // collected when there are no more Lua references to it.
    if (node != nullptr &&
        node->IsUserdataCreated())
    {
        int nodeId = (int)node->GetNodeId();

        lua_State* L = GetLua();
        int preTop = lua_gettop(L);

        // We need to move the ref from the strong table to the weak table
        // Strong/Weak tables should be created when first userdata is created in Node_Lua::Create()
        {
            lua_getfield(L, LUA_REGISTRYINDEX, OCT_NODE_STRONG_TABLE_KEY);
            OCT_ASSERT(lua_istable(L, -1));
            int strongTableIdx = lua_gettop(L);

            lua_getfield(L, LUA_REGISTRYINDEX, OCT_NODE_WEAK_TABLE_KEY);
            OCT_ASSERT(lua_istable(L, -1));
            int weakTableIdx = lua_gettop(L);

            // Add weak table ref
            lua_rawgeti(L, strongTableIdx, nodeId);
            lua_rawseti(L, weakTableIdx, nodeId);

            // Pop weak table
            lua_pop(L, 1);

            // Clear strong table ref
            lua_pushnil(L);
            lua_rawseti(L, strongTableIdx, nodeId);

            // Pop strong table
            lua_pop(L, 1);
        }

        int postTop = lua_gettop(L);

        OCT_ASSERT(preTop == postTop);
    }
}
