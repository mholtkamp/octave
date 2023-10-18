#include "Actor.h"
#include "Renderer.h"
#include "Clock.h"
#include "World.h"
#include "Nodes/3D/CameraComponent.h"
#include "Enums.h"
#include "Log.h"
#include "Maths.h"
#include "Utilities.h"
#include "Engine.h"
#include "ObjectRef.h"
#include "NetworkManager.h"
#include "LuaBindings/Actor_Lua.h"
#include "Assets/Blueprint.h"

#include "Nodes/3D/PrimitiveComponent.h"
#include "Nodes/3D/LightComponent.h"

#include "Graphics/Graphics.h"

#if LUA_ENABLED
#include "LuaBindings/LuaUtils.h"
#endif

#if EDITOR
#include "EditorState.h"
#endif

#include <functional>
#include <algorithm>

using namespace std;

void Actor::DestroyAllComponents()
{
    // Destroy components first, then delete the memory, in case a component
    // references a sibling component during its destruction.
    for (int32_t i = int32_t(mComponents.size()) - 1; i >= 0; --i)
    {
        mComponents[i]->Destroy();
        mComponents[i]->SetOwner(nullptr);
    }

    for (int32_t i = int32_t(mComponents.size()) - 1; i >= 0; --i)
    {
        delete mComponents[i];
    }

    mComponents.clear();
}

FORCE_LINK_DEF(Actor);
DEFINE_FACTORY_MANAGER(Actor);
DEFINE_FACTORY(Actor, Actor);
DEFINE_RTTI(Actor);
