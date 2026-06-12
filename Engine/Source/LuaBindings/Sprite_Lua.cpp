#include "LuaBindings/Sprite_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Widget_Lua.h"

#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Rect_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#include "Assets/Texture.h"
#include "AssetManager.h"

#if LUA_ENABLED

int Sprite_Lua::AddAnimation(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    std::string animationName = "";
    if (!lua_isnone(L, 2)) { animationName = CHECK_STRING(L, 2); };

    sprite->AddAnimation(animationName);

    return 0;
}

int Sprite_Lua::AddFrame(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnone(L, 2)) { texture = CHECK_TEXTURE(L, 2); };
    int32_t frameIndex = -1;
    if (!lua_isnone(L, 3)) { frameIndex = CHECK_INDEX(L, 3); };
    bool insert = false;
    if (!lua_isnone(L, 4)) { insert = CHECK_BOOLEAN(L, 4); };
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 5)) { animationIndex = CHECK_INDEX(L, 5); };

    sprite->AddFrame(texture, frameIndex, insert, animationIndex);

    return 0;
}

int Sprite_Lua::AddEmptyFrame(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    int32_t frameIndex = -1;
    if (!lua_isnone(L, 2)) { frameIndex = CHECK_INDEX(L, 2); };
    bool insert = false;
    if (!lua_isnone(L, 3)) { insert = CHECK_BOOLEAN(L, 3); };
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 4)) { animationIndex = CHECK_INDEX(L, 4); };

    sprite->AddEmptyFrame(frameIndex, insert, animationIndex);

    return 0;
}

int Sprite_Lua::RemoveAnimation(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    uint32_t animationIndex = CHECK_INDEX(L, 2);

    sprite->RemoveAnimation(animationIndex);

    return 0;
}

int Sprite_Lua::RemoveFrame(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    uint32_t frameIndex = CHECK_INDEX(L, 2);
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 3)) { animationIndex = CHECK_INDEX(L, 3); };

    sprite->RemoveFrame(frameIndex, animationIndex);

    return 0;
}

int Sprite_Lua::SetAnimation(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    if (lua_isnumber(L, 2))
    {
        uint32_t animationIndex = CHECK_INDEX(L, 2);
        sprite->SetAnimation(animationIndex);
    }
    else
    {
        std::string animationName = CHECK_STRING(L, 2);
        sprite->SetAnimation(animationName);
    }

    return 0;
}

int Sprite_Lua::SetFrame(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    uint32_t frameIndex = CHECK_INDEX(L, 2);

    sprite->SetFrame(frameIndex);

    return 0;
}

int Sprite_Lua::SetFPS(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    float fps = CHECK_NUMBER(L, 2);

    sprite->SetFPS(fps);

    return 0;
}

int Sprite_Lua::SetPlay(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    bool enablePlay = CHECK_BOOLEAN(L, 2);

    sprite->SetPlay(enablePlay);

    return 0;
}

int Sprite_Lua::SetLoop(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    bool enableLoop = CHECK_BOOLEAN(L, 2);

    sprite->SetLoop(enableLoop);

    return 0;
}

int Sprite_Lua::SetAnimationName(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    std::string animationName = CHECK_STRING(L, 2);
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 3)) { animationIndex = CHECK_INDEX(L, 3); };

    sprite->SetAnimationName(animationName, animationIndex);

    return 0;
}

int Sprite_Lua::GetFrame(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    uint32_t frameIndex = CHECK_INDEX(L, 2);
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 3)) { animationIndex = CHECK_INDEX(L, 3); };

    Texture* ret = sprite->GetFrame(frameIndex, animationIndex);

    Asset_Lua::Create(L, ret);
    return 1;
}

int Sprite_Lua::GetAnimationLength(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 2)) { animationIndex = CHECK_INDEX(L, 2); };

    uint32_t ret = sprite->GetAnimationLength(animationIndex);

    lua_pushinteger(L, ret);


    return 1;
}

int Sprite_Lua::GetAnimationName(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 2)) { animationIndex = CHECK_INDEX(L, 2); };

    std::string ret = sprite->GetAnimationName(animationIndex);

    lua_pushstring(L, ret.c_str());

    return 1;
}

int Sprite_Lua::IsPlaying(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);

    bool ret = sprite->IsPlaying();

    lua_pushboolean(L, ret);

    return 1;
}

int Sprite_Lua::GetLoop(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);

    bool ret = sprite->GetLoop();

    lua_pushboolean(L, ret);


    return 1;
}

int Sprite_Lua::GetFrameIndex(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    int32_t ret = sprite->GetFrameIndex();

    lua_pushnumber(L, ret);

    return 1;
}

int Sprite_Lua::GetNumAnimations(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);

    uint32_t ret = sprite->GetNumAnimations();

    lua_pushnumber(L, ret);

    return 1;
}

int Sprite_Lua::GetAnimationIndex(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);

    int32_t ret = sprite->GetAnimationIndex();

    ++ret;

    lua_pushnumber(L, ret);

    return 1;
}

int Sprite_Lua::GetNumFrames(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);
    int32_t animationIndex = -1;
    if (!lua_isnone(L, 2)) { animationIndex = CHECK_INDEX(L, 2); };

    uint32_t ret = sprite->GetNumFrames(animationIndex);

    lua_pushnumber(L, ret);

    return 1;
}

int Sprite_Lua::GetFPS(lua_State* L)
{
    Sprite* sprite = CHECK_SPRITE(L, 1);

    uint32_t ret = sprite->GetFPS();

    lua_pushnumber(L, ret);

    return 1;
}

void Sprite_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        SPRITE_LUA_NAME,
        SPRITE_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, AddAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, AddFrame);

    REGISTER_TABLE_FUNC(L, mtIndex, AddEmptyFrame);

    REGISTER_TABLE_FUNC(L, mtIndex, RemoveAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, RemoveFrame);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAnimation);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFrame);

    REGISTER_TABLE_FUNC(L, mtIndex, SetFPS);

    REGISTER_TABLE_FUNC(L, mtIndex, SetPlay);

    REGISTER_TABLE_FUNC(L, mtIndex, SetAnimationName);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFrame);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAnimationLength);

    REGISTER_TABLE_FUNC(L, mtIndex, SetLoop);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAnimationName);

    REGISTER_TABLE_FUNC(L, mtIndex, IsPlaying);

    REGISTER_TABLE_FUNC(L, mtIndex, GetLoop);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFrameIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumAnimations);

    REGISTER_TABLE_FUNC(L, mtIndex, GetAnimationIndex);

    REGISTER_TABLE_FUNC(L, mtIndex, GetNumFrames);

    REGISTER_TABLE_FUNC(L, mtIndex, GetFPS);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
