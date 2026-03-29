#include "ToolTipManager.h"
#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/ToolTipWidget.h"
#include "World.h"
#include "Engine.h"
#include "InputDevices.h"
#include "Renderer.h"
#include "Log.h"

#if LUA_ENABLED
#include "LuaBindings/LuaUtils.h"
#include "LuaBindings/Node_Lua.h"

static void PushWidgetToLua(lua_State* L, Widget* widget)
{
    if (!widget)
    {
        lua_pushnil(L);
        return;
    }

    widget->RegisterScriptFuncs(L);

    Node** ud = (Node**)lua_newuserdata(L, sizeof(Node*));
    *ud = widget;

    luaL_getmetatable(L, widget->GetClassName());
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        luaL_getmetatable(L, NODE_LUA_NAME);
    }
    lua_setmetatable(L, -2);
}

static void CallLuaGlobalFunc(lua_State* L, const char* funcName, Widget* widget)
{
    lua_getglobal(L, funcName);
    if (lua_isfunction(L, -1))
    {
        PushWidgetToLua(L, widget);
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            const char* err = lua_tostring(L, -1);
            LogError("ToolTip callback '%s' error: %s", funcName, err ? err : "unknown");
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
}

static void CallLuaRefFunc(lua_State* L, int ref, Widget* widget)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
    if (lua_isfunction(L, -1))
    {
        PushWidgetToLua(L, widget);
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            const char* err = lua_tostring(L, -1);
            LogError("ToolTip callback error: %s", err ? err : "unknown");
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
}
#endif

ToolTipManager* ToolTipManager::sInstance = nullptr;

void ToolTipManager::Create()
{
    OCT_ASSERT(sInstance == nullptr);
    sInstance = new ToolTipManager();
}

void ToolTipManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

ToolTipManager* ToolTipManager::Get()
{
    return sInstance;
}

ToolTipManager::ToolTipManager()
{
}

ToolTipManager::~ToolTipManager()
{
    Shutdown();
}

void ToolTipManager::Initialize()
{
    // ToolTipWidget is created lazily when needed
}

void ToolTipManager::Shutdown()
{
    if (mDefaultToolTipWidget != nullptr)
    {
        mDefaultToolTipWidget->Destroy();
        mDefaultToolTipWidget.Reset();
    }

    mHoveredWidget = nullptr;
    mActiveWidget = nullptr;
    mTooltipVisible = false;
    mHoverTime = 0.0f;

#if LUA_ENABLED
    // Clear Lua references
    if (mOnShowLuaRef >= 0)
    {
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, mOnShowLuaRef);
        }
        mOnShowLuaRef = -1;
    }
    if (mOnHideLuaRef >= 0)
    {
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, mOnHideLuaRef);
        }
        mOnHideLuaRef = -1;
    }
#endif
    mOnShowFuncName.clear();
    mOnHideFuncName.clear();
}

void ToolTipManager::Tick(float deltaTime)
{
    if (!mEnabled)
    {
        if (mTooltipVisible)
        {
            HideTooltip();
        }
        return;
    }

    UpdateHoverTracking();

    // Check if we have a widget with tooltip
    if (mHoveredWidget != nullptr && mHoveredWidget->HasTooltip())
    {
        mHoverTime += deltaTime;

        // Show tooltip after delay
        if (!mTooltipVisible && mHoverTime >= mShowDelay)
        {
            ShowTooltip(mHoveredWidget);
        }
        else if (mTooltipVisible)
        {
            // Update position while visible
            PositionTooltip();
        }
    }
    else
    {
        // No hovered widget with tooltip - hide
        if (mTooltipVisible)
        {
            HideTooltip();
        }
        mHoverTime = 0.0f;
    }
}

void ToolTipManager::UpdateHoverTracking()
{
    Widget* newHovered = FindHoveredWidgetWithTooltip();

    if (newHovered != mHoveredWidget)
    {
        // Hovered widget changed
        mHoveredWidget = newHovered;
        mHoverTime = 0.0f;

        // Hide tooltip when hovering over a different widget
        if (mTooltipVisible && mActiveWidget != newHovered)
        {
            HideTooltip();
        }
    }
}

Widget* ToolTipManager::FindHoveredWidgetWithTooltip()
{
    Widget* result = nullptr;

    // Check all worlds
    for (int32_t i = 0; i < GetNumWorlds(); ++i)
    {
        World* world = GetWorld(i);
        if (world != nullptr && world->GetRootNode() != nullptr)
        {
            Node* root = world->GetRootNode();
            if (root->IsWidget())
            {
                TraverseWidgetsForHover(static_cast<Widget*>(root), result);
            }
            else
            {
                // Traverse children
                for (uint32_t c = 0; c < root->GetNumChildren(); ++c)
                {
                    Node* child = root->GetChild(c);
                    if (child != nullptr && child->IsWidget())
                    {
                        TraverseWidgetsForHover(static_cast<Widget*>(child), result);
                    }
                }
            }
        }
    }

    return result;
}

void ToolTipManager::TraverseWidgetsForHover(Widget* widget, Widget*& outHovered)
{
    if (widget == nullptr || !widget->IsVisible())
        return;

    // Skip the tooltip widget itself
    if (widget == mDefaultToolTipWidget.Get())
        return;

    // Depth-first: check children first (children are rendered on top)
    for (uint32_t i = 0; i < widget->GetNumChildren(); ++i)
    {
        Node* child = widget->GetChild(i);
        if (child != nullptr && child->IsWidget())
        {
            TraverseWidgetsForHover(static_cast<Widget*>(child), outHovered);
        }
    }

    // Then check this widget
    if (widget->ContainsMouse() && widget->HasTooltip())
    {
        // Last checked (deepest visible) widget wins
        outHovered = widget;
    }
}

void ToolTipManager::PositionTooltip()
{
    if (mActiveTooltipWidget == nullptr)
        return;

    int32_t mouseX, mouseY;
    GetMousePosition(mouseX, mouseY);

    float tooltipX = float(mouseX) + mCursorOffset.x;
    float tooltipY = float(mouseY) + mCursorOffset.y;

    // Get viewport dimensions
    glm::uvec4 vp = Renderer::Get()->GetViewport();
    float viewWidth = float(vp.z);
    float viewHeight = float(vp.w);

    // Get tooltip dimensions
    float tooltipWidth = mActiveTooltipWidget->GetWidth();
    float tooltipHeight = mActiveTooltipWidget->GetHeight();

    // Clamp to screen bounds
    if (tooltipX + tooltipWidth > viewWidth)
    {
        tooltipX = viewWidth - tooltipWidth;
    }
    if (tooltipY + tooltipHeight > viewHeight)
    {
        // Try positioning above cursor instead
        tooltipY = float(mouseY) - tooltipHeight - 4.0f;
        if (tooltipY < 0.0f)
        {
            tooltipY = 0.0f;
        }
    }
    if (tooltipX < 0.0f)
    {
        tooltipX = 0.0f;
    }

    mActiveTooltipWidget->SetPosition(tooltipX, tooltipY);
}

void ToolTipManager::ShowTooltip(Widget* forWidget)
{
    if (forWidget == nullptr || !forWidget->HasTooltip())
        return;

    // Check if there's a registered template for this tooltip name
    const std::string& tooltipName = forWidget->GetTooltipName();
    ToolTipWidget* tooltipToUse = nullptr;

    auto it = mTemplates.find(tooltipName);
    if (it != mTemplates.end() && it->second != nullptr)
    {
        // Use the registered template
        tooltipToUse = it->second;
    }
    else
    {
        // Use the default tooltip widget
        if (mDefaultToolTipWidget == nullptr)
        {
            mDefaultToolTipWidget = Node::Construct<ToolTipWidget>();
            mDefaultToolTipWidget->Create();
            mDefaultToolTipWidget->SetTransient(true);
        }
        tooltipToUse = mDefaultToolTipWidget.Get();

        // Configure default tooltip content from widget
        tooltipToUse->ConfigureFromWidget(forWidget);
    }

    // Attach to the widget's world root so it renders
    // Re-attaching moves it to the end of the children list (renders on top)
    World* world = forWidget->GetWorld();
    if (world != nullptr && world->GetRootNode() != nullptr)
    {
        tooltipToUse->Attach(world->GetRootNode());
    }

    mActiveTooltipWidget = tooltipToUse;

    // Fire show callback (before showing, so callback can modify content)
    FireShowCallback(forWidget);

    // Position and show
    PositionTooltip();
    mActiveTooltipWidget->SetVisible(true);

    mActiveWidget = forWidget;
    mTooltipVisible = true;
}

void ToolTipManager::HideTooltip()
{
    if (!mTooltipVisible)
        return;

    // Fire hide callback
    FireHideCallback(mActiveWidget);

    if (mActiveTooltipWidget != nullptr)
    {
        mActiveTooltipWidget->SetVisible(false);
    }

    mActiveTooltipWidget = nullptr;
    mActiveWidget = nullptr;
    mTooltipVisible = false;
}

void ToolTipManager::FireShowCallback(Widget* widget)
{
#if LUA_ENABLED
    lua_State* L = GetLua();
    if (L == nullptr)
        return;

    if (mOnShowLuaRef >= 0)
    {
        CallLuaRefFunc(L, mOnShowLuaRef, widget);
    }
    else if (!mOnShowFuncName.empty())
    {
        CallLuaGlobalFunc(L, mOnShowFuncName.c_str(), widget);
    }
#endif
}

void ToolTipManager::FireHideCallback(Widget* widget)
{
#if LUA_ENABLED
    lua_State* L = GetLua();
    if (L == nullptr)
        return;

    if (mOnHideLuaRef >= 0)
    {
        CallLuaRefFunc(L, mOnHideLuaRef, widget);
    }
    else if (!mOnHideFuncName.empty())
    {
        CallLuaGlobalFunc(L, mOnHideFuncName.c_str(), widget);
    }
#endif
}

// Configuration
void ToolTipManager::SetShowDelay(float seconds)
{
    mShowDelay = seconds;
}

float ToolTipManager::GetShowDelay() const
{
    return mShowDelay;
}

void ToolTipManager::SetHideDelay(float seconds)
{
    mHideDelay = seconds;
}

float ToolTipManager::GetHideDelay() const
{
    return mHideDelay;
}

void ToolTipManager::SetCursorOffset(glm::vec2 offset)
{
    mCursorOffset = offset;
}

glm::vec2 ToolTipManager::GetCursorOffset() const
{
    return mCursorOffset;
}

void ToolTipManager::SetEnabled(bool enabled)
{
    mEnabled = enabled;
    if (!mEnabled && mTooltipVisible)
    {
        HideTooltip();
    }
}

bool ToolTipManager::IsEnabled() const
{
    return mEnabled;
}

ToolTipWidget* ToolTipManager::GetToolTipWidget()
{
    // Create tooltip widget if needed
    if (mDefaultToolTipWidget == nullptr)
    {
        mDefaultToolTipWidget = Node::Construct<ToolTipWidget>();
        mDefaultToolTipWidget->Create();
        mDefaultToolTipWidget->SetTransient(true);
    }
    return mDefaultToolTipWidget.Get();
}

void ToolTipManager::RegisterTemplate(const std::string& name, ToolTipWidget* tooltipWidget)
{
    if (name.empty() || tooltipWidget == nullptr)
    {
        return;
    }

    auto it = mTemplates.find(name);
    if (it != mTemplates.end())
    {
        LogWarning("ToolTipManager: Template '%s' already registered. Overwriting.", name.c_str());
    }

    mTemplates[name] = tooltipWidget;
}

void ToolTipManager::UnregisterTemplate(const std::string& name)
{
    auto it = mTemplates.find(name);
    if (it != mTemplates.end())
    {
        // If this template is currently active, hide it
        if (mActiveTooltipWidget == it->second)
        {
            HideTooltip();
        }
        mTemplates.erase(it);
    }
}

void ToolTipManager::UnregisterTemplate(ToolTipWidget* tooltipWidget)
{
    if (tooltipWidget == nullptr)
    {
        return;
    }

    for (auto it = mTemplates.begin(); it != mTemplates.end(); ++it)
    {
        if (it->second == tooltipWidget)
        {
            // If this template is currently active, hide it
            if (mActiveTooltipWidget == tooltipWidget)
            {
                HideTooltip();
            }
            mTemplates.erase(it);
            return;
        }
    }
}

ToolTipWidget* ToolTipManager::GetTemplate(const std::string& name)
{
    auto it = mTemplates.find(name);
    if (it != mTemplates.end())
    {
        return it->second;
    }
    return nullptr;
}

bool ToolTipManager::HasTemplate(const std::string& name) const
{
    return mTemplates.find(name) != mTemplates.end();
}

Widget* ToolTipManager::GetHoveredWidget() const
{
    return mHoveredWidget;
}

bool ToolTipManager::IsTooltipVisible() const
{
    return mTooltipVisible;
}

// Lua callbacks
void ToolTipManager::SetOnShowCallback(const std::string& funcName)
{
    ClearOnShowCallback();
    mOnShowFuncName = funcName;
}

void ToolTipManager::SetOnShowCallbackRef(int luaRef)
{
    ClearOnShowCallback();
    mOnShowLuaRef = luaRef;
}

void ToolTipManager::ClearOnShowCallback()
{
#if LUA_ENABLED
    if (mOnShowLuaRef >= 0)
    {
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, mOnShowLuaRef);
        }
        mOnShowLuaRef = -1;
    }
#endif
    mOnShowFuncName.clear();
}

void ToolTipManager::SetOnHideCallback(const std::string& funcName)
{
    ClearOnHideCallback();
    mOnHideFuncName = funcName;
}

void ToolTipManager::SetOnHideCallbackRef(int luaRef)
{
    ClearOnHideCallback();
    mOnHideLuaRef = luaRef;
}

void ToolTipManager::ClearOnHideCallback()
{
#if LUA_ENABLED
    if (mOnHideLuaRef >= 0)
    {
        lua_State* L = GetLua();
        if (L != nullptr)
        {
            luaL_unref(L, LUA_REGISTRYINDEX, mOnHideLuaRef);
        }
        mOnHideLuaRef = -1;
    }
#endif
    mOnHideFuncName.clear();
}
