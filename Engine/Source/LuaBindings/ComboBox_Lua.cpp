#include "LuaBindings/ComboBox_Lua.h"
#include "LuaBindings/Quad_Lua.h"
#include "LuaBindings/Text_Lua.h"
#include "LuaBindings/Widget_Lua.h"
#include "LuaBindings/Node_Lua.h"

#include "LuaBindings/Vector_Lua.h"

#if LUA_ENABLED

int ComboBox_Lua::AddOption(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    const char* option = CHECK_STRING(L, 2);

    comboBox->AddOption(option);

    return 0;
}

int ComboBox_Lua::RemoveOption(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    comboBox->RemoveOption(index);

    return 0;
}

int ComboBox_Lua::ClearOptions(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    comboBox->ClearOptions();

    return 0;
}

int ComboBox_Lua::SetOptions(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    luaL_checktype(L, 2, LUA_TTABLE);
    std::vector<std::string> options;

    int tableLen = (int)lua_rawlen(L, 2);
    for (int i = 1; i <= tableLen; i++)
    {
        lua_rawgeti(L, 2, i);
        if (lua_isstring(L, -1))
        {
            options.push_back(lua_tostring(L, -1));
        }
        lua_pop(L, 1);
    }

    comboBox->SetOptions(options);

    return 0;
}

int ComboBox_Lua::GetOptions(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    const std::vector<std::string>& options = comboBox->GetOptions();

    lua_newtable(L);
    for (size_t i = 0; i < options.size(); i++)
    {
        lua_pushstring(L, options[i].c_str());
        lua_rawseti(L, -2, (int)(i + 1));
    }

    return 1;
}

int ComboBox_Lua::GetOptionCount(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    int32_t ret = comboBox->GetOptionCount();

    lua_pushinteger(L, ret);
    return 1;
}

int ComboBox_Lua::SetSelectedIndex(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    int32_t index = CHECK_INTEGER(L, 2);

    comboBox->SetSelectedIndex(index);

    return 0;
}

int ComboBox_Lua::GetSelectedIndex(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    int32_t ret = comboBox->GetSelectedIndex();

    lua_pushinteger(L, ret);
    return 1;
}

int ComboBox_Lua::GetSelectedOption(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    std::string ret = comboBox->GetSelectedOption();

    lua_pushstring(L, ret.c_str());
    return 1;
}

int ComboBox_Lua::IsOpen(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    bool ret = comboBox->IsOpen();

    lua_pushboolean(L, ret);
    return 1;
}

int ComboBox_Lua::Open(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    comboBox->Open();

    return 0;
}

int ComboBox_Lua::Close(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    comboBox->Close();

    return 0;
}

int ComboBox_Lua::Toggle(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    comboBox->Toggle();

    return 0;
}

int ComboBox_Lua::SetBackgroundColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    comboBox->SetBackgroundColor(color);

    return 0;
}

int ComboBox_Lua::GetBackgroundColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    glm::vec4 ret = comboBox->GetBackgroundColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ComboBox_Lua::SetTextColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    comboBox->SetTextColor(color);

    return 0;
}

int ComboBox_Lua::GetTextColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    glm::vec4 ret = comboBox->GetTextColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ComboBox_Lua::SetDropdownColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    comboBox->SetDropdownColor(color);

    return 0;
}

int ComboBox_Lua::GetDropdownColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    glm::vec4 ret = comboBox->GetDropdownColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ComboBox_Lua::SetHoveredColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    glm::vec4 color = CHECK_VECTOR(L, 2);

    comboBox->SetHoveredColor(color);

    return 0;
}

int ComboBox_Lua::GetHoveredColor(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    glm::vec4 ret = comboBox->GetHoveredColor();

    Vector_Lua::Create(L, ret);
    return 1;
}

int ComboBox_Lua::SetMaxVisibleItems(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);
    int32_t count = CHECK_INTEGER(L, 2);

    comboBox->SetMaxVisibleItems(count);

    return 0;
}

int ComboBox_Lua::GetMaxVisibleItems(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    int32_t ret = comboBox->GetMaxVisibleItems();

    lua_pushinteger(L, ret);
    return 1;
}

int ComboBox_Lua::GetBackground(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    Quad* ret = comboBox->GetBackground();

    Node_Lua::Create(L, ret);
    return 1;
}

int ComboBox_Lua::GetTextWidget(lua_State* L)
{
    ComboBox* comboBox = CHECK_COMBOBOX(L, 1);

    Text* ret = comboBox->GetTextWidget();

    Node_Lua::Create(L, ret);
    return 1;
}

void ComboBox_Lua::Bind()
{
    lua_State* L = GetLua();
    int mtIndex = CreateClassMetatable(
        COMBOBOX_LUA_NAME,
        COMBOBOX_LUA_FLAG,
        WIDGET_LUA_NAME);

    Node_Lua::BindCommon(L, mtIndex);

    // Options
    REGISTER_TABLE_FUNC(L, mtIndex, AddOption);
    REGISTER_TABLE_FUNC(L, mtIndex, RemoveOption);
    REGISTER_TABLE_FUNC(L, mtIndex, ClearOptions);
    REGISTER_TABLE_FUNC(L, mtIndex, SetOptions);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOptions);
    REGISTER_TABLE_FUNC(L, mtIndex, GetOptionCount);

    // Selection
    REGISTER_TABLE_FUNC(L, mtIndex, SetSelectedIndex);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectedIndex);
    REGISTER_TABLE_FUNC(L, mtIndex, GetSelectedOption);

    // State
    REGISTER_TABLE_FUNC(L, mtIndex, IsOpen);
    REGISTER_TABLE_FUNC(L, mtIndex, Open);
    REGISTER_TABLE_FUNC(L, mtIndex, Close);
    REGISTER_TABLE_FUNC(L, mtIndex, Toggle);

    // Visual
    REGISTER_TABLE_FUNC(L, mtIndex, SetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackgroundColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetDropdownColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetDropdownColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, GetHoveredColor);
    REGISTER_TABLE_FUNC(L, mtIndex, SetMaxVisibleItems);
    REGISTER_TABLE_FUNC(L, mtIndex, GetMaxVisibleItems);

    // Children
    REGISTER_TABLE_FUNC(L, mtIndex, GetBackground);
    REGISTER_TABLE_FUNC(L, mtIndex, GetTextWidget);

    lua_pop(L, 1);
    OCT_ASSERT(lua_gettop(L) == 0);
}

#endif
