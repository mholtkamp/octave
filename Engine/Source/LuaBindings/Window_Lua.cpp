#include "LuaBindings/Window_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"
#include "LuaBindings/Canvas_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Button_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/ScrollContainer_Lua.h"
#include "LuaBindings/Vector_Lua.h"
#include "LuaBindings/Asset_Lua.h"
#include "LuaBindings/Texture_Lua.h"

#if LUA_ENABLED

int Window_Lua::SetWindowId(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    const char* id = CHECK_STRING(L, 2);

    window->SetWindowId(id);

    return 0;
}

int Window_Lua::GetWindowId(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    const std::string& id = window->GetWindowId();

    lua_pushstring(L, id.c_str());
    return 1;
}

int Window_Lua::SetTitle(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    const char* title = CHECK_STRING(L, 2);

    window->SetTitle(title);

    return 0;
}

int Window_Lua::GetTitle(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    const std::string& title = window->GetTitle();

    lua_pushstring(L, title.c_str());
    return 1;
}

int Window_Lua::SetTitleAlignment(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    TitleAlignment alignment = (TitleAlignment)CHECK_INTEGER(L, 2);

    window->SetTitleAlignment(alignment);

    return 0;
}

int Window_Lua::GetTitleAlignment(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    int ret = (int)window->GetTitleAlignment();

    lua_pushinteger(L, ret);
    return 1;
}

int Window_Lua::SetTitleFontSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float size = CHECK_NUMBER(L, 2);

    window->SetTitleFontSize(size);

    return 0;
}

int Window_Lua::GetTitleFontSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetTitleFontSize();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::SetTitleOffset(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec2 offset = CHECK_VECTOR(L, 2);

    window->SetTitleOffset(offset);

    return 0;
}

int Window_Lua::GetTitleOffset(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec2 ret = window->GetTitleOffset();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetContentPadding(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float left = CHECK_NUMBER(L, 2);
    float top = CHECK_NUMBER(L, 3);
    float right = CHECK_NUMBER(L, 4);
    float bottom = CHECK_NUMBER(L, 5);

    window->SetContentPadding(left, top, right, bottom);

    return 0;
}

int Window_Lua::SetContentPaddingLeft(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float left = CHECK_NUMBER(L, 2);

    window->SetContentPaddingLeft(left);

    return 0;
}

int Window_Lua::SetContentPaddingTop(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float top = CHECK_NUMBER(L, 2);

    window->SetContentPaddingTop(top);

    return 0;
}

int Window_Lua::SetContentPaddingRight(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float right = CHECK_NUMBER(L, 2);

    window->SetContentPaddingRight(right);

    return 0;
}

int Window_Lua::SetContentPaddingBottom(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float bottom = CHECK_NUMBER(L, 2);

    window->SetContentPaddingBottom(bottom);

    return 0;
}

int Window_Lua::GetContentPaddingLeft(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetContentPaddingLeft();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::GetContentPaddingTop(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetContentPaddingTop();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::GetContentPaddingRight(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetContentPaddingRight();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::GetContentPaddingBottom(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetContentPaddingBottom();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::Show(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    window->Show();

    return 0;
}

int Window_Lua::Hide(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    window->Hide();

    return 0;
}

int Window_Lua::Close(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    window->Close();

    return 0;
}

int Window_Lua::SetDraggable(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    bool draggable = CHECK_BOOLEAN(L, 2);

    window->SetDraggable(draggable);

    return 0;
}

int Window_Lua::IsDraggable(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    bool ret = window->IsDraggable();

    lua_pushboolean(L, ret);
    return 1;
}

int Window_Lua::SetResizable(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    bool resizable = CHECK_BOOLEAN(L, 2);

    window->SetResizable(resizable);

    return 0;
}

int Window_Lua::IsResizable(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    bool ret = window->IsResizable();

    lua_pushboolean(L, ret);
    return 1;
}

int Window_Lua::SetShowCloseButton(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    bool show = CHECK_BOOLEAN(L, 2);

    window->SetShowCloseButton(show);

    return 0;
}

int Window_Lua::GetShowCloseButton(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    bool ret = window->GetShowCloseButton();

    lua_pushboolean(L, ret);
    return 1;
}

int Window_Lua::SetTitleBarHeight(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float height = CHECK_NUMBER(L, 2);

    window->SetTitleBarHeight(height);

    return 0;
}

int Window_Lua::GetTitleBarHeight(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetTitleBarHeight();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::SetMinSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec2 minSize = CHECK_VECTOR(L, 2);

    window->SetMinSize(minSize);

    return 0;
}

int Window_Lua::GetMinSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec2 ret = window->GetMinSize();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetMaxSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec2 maxSize = CHECK_VECTOR(L, 2);

    window->SetMaxSize(maxSize);

    return 0;
}

int Window_Lua::GetMaxSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec2 ret = window->GetMaxSize();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetResizeHandleSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    float size = CHECK_NUMBER(L, 2);

    window->SetResizeHandleSize(size);

    return 0;
}

int Window_Lua::GetResizeHandleSize(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    float ret = window->GetResizeHandleSize();

    lua_pushnumber(L, ret);
    return 1;
}

int Window_Lua::SetContentWidget(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    Widget* widget = CHECK_WIDGET(L, 2);

    window->SetContentWidget(widget);

    return 0;
}

int Window_Lua::GetContentContainer(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    ScrollContainer* ret = window->GetContentContainer();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::GetContentWidget(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Widget* ret = window->GetContentWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetTitleBarColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    window->SetTitleBarColor(color);

    return 0;
}

int Window_Lua::GetTitleBarColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec4 ret = window->GetTitleBarColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetBackgroundColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    window->SetBackgroundColor(color);

    return 0;
}

int Window_Lua::GetBackgroundColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec4 ret = window->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::GetTitleBar(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Canvas* ret = window->GetTitleBar();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::GetTitleText(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Text* ret = window->GetTitleText();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::GetCloseButton(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Button* ret = window->GetCloseButton();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::GetResizeHandle(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Quad* ret = window->GetResizeHandle();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::GetBackground(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Quad* ret = window->GetBackground();

    Node_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetBackgroundTexture(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    window->SetBackgroundTexture(texture);

    return 0;
}

int Window_Lua::GetBackgroundTexture(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Texture* ret = window->GetBackgroundTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetTitleBarTexture(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    window->SetTitleBarTexture(texture);

    return 0;
}

int Window_Lua::GetTitleBarTexture(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Texture* ret = window->GetTitleBarTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetCloseButtonTexture(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    Texture* texture = nullptr;
    if (!lua_isnil(L, 2))
    {
        texture = CHECK_TEXTURE(L, 2);
    }

    window->SetCloseButtonTexture(texture);

    return 0;
}

int Window_Lua::GetCloseButtonTexture(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    Texture* ret = window->GetCloseButtonTexture();

    Asset_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetCloseButtonNormalColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    window->SetCloseButtonNormalColor(color);

    return 0;
}

int Window_Lua::GetCloseButtonNormalColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec4 ret = window->GetCloseButtonNormalColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetCloseButtonHoveredColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    window->SetCloseButtonHoveredColor(color);

    return 0;
}

int Window_Lua::GetCloseButtonHoveredColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec4 ret = window->GetCloseButtonHoveredColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int Window_Lua::SetCloseButtonPressedColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    window->SetCloseButtonPressedColor(color);

    return 0;
}

int Window_Lua::GetCloseButtonPressedColor(lua_State* L)
{
    Window* window = CHECK_WINDOW(L, 1);

    glm::vec4 ret = window->GetCloseButtonPressedColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

void Window_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        WINDOW_LUA_NAME,
        WINDOW_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Window ID
    REGISTER_TABLE_FUNC(L, mtIndex, SetWindowId);
    REGISTER_TABLE_FUNC(L, mtIndex, GetWindowId);

    // Title
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitle);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitle);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleAlignment);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleAlignment);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleFontSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleFontSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleOffset);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleOffset);

    // Content Padding
    REGISTER_TABLE_FUNC(L, mtIndex, SetContentPadding);
    REGISTER_TABLE_FUNC(L, mtIndex, SetContentPaddingLeft);
    REGISTER_TABLE_FUNC(L, mtIndex, SetContentPaddingTop);
    REGISTER_TABLE_FUNC(L, mtIndex, SetContentPaddingRight);
    REGISTER_TABLE_FUNC(L, mtIndex, SetContentPaddingBottom);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentPaddingLeft);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentPaddingTop);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentPaddingRight);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentPaddingBottom);

    // Visibility
    REGISTER_TABLE_FUNC(L, mtIndex, Show);
    REGISTER_TABLE_FUNC(L, mtIndex, Hide);
    REGISTER_TABLE_FUNC(L, mtIndex, Close);

    // Properties
    REGISTER_TABLE_FUNC(L, mtIndex, SetDraggable);
    REGISTER_TABLE_FUNC(L, mtIndex, IsDraggable);
    REGISTER_TABLE_FUNC(L, mtIndex, SetResizable);
    REGISTER_TABLE_FUNC(L, mtIndex, IsResizable);
    REGISTER_TABLE_FUNC(L, mtIndex, SetShowCloseButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetShowCloseButton);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleBarHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleBarHeight);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMinSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMinSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxSize);
    REGISTER_TABLE_FUNC(L, mtIndex, SetResizeHandleSize);
    REGISTER_TABLE_FUNC(L, mtIndex, GetResizeHandleSize);

    // Content
    REGISTER_TABLE_FUNC(L, mtIndex, SetContentWidget);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentContainer);
    REGISTER_TABLE_FUNC(L, mtIndex, GetContentWidget);

    // Colors
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleBarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleBarColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);

    // Textures
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTitleBarTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleBarTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, SetCloseButtonTexture);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCloseButtonTexture);

    // Close Button Colors
    REGISTER_TABLE_FUNC(L, mtIndex, SetCloseButtonNormalColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCloseButtonNormalColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetCloseButtonHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCloseButtonHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetCloseButtonPressedColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCloseButtonPressedColor);

    // Internal widget access
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleBar);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTitleText);
    REGISTER_TABLE_FUNC(L, mtIndex, GetCloseButton);
    REGISTER_TABLE_FUNC(L, mtIndex, GetResizeHandle);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackground);

    lua_pop(L, 1);
}

#endif
