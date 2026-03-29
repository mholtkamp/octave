#pragma once

#include "EngineTypes.h"
#include "Maths.h"
#include "SmartPointer.h"

#include <string>
#include <unordered_map>

class Widget;
class ToolTipWidget;

class ToolTipManager
{
public:

    static void Create();
    static void Destroy();
    static ToolTipManager* Get();

    void Initialize();
    void Shutdown();
    void Tick(float deltaTime);

    // Configuration
    void SetShowDelay(float seconds);
    float GetShowDelay() const;
    void SetHideDelay(float seconds);
    float GetHideDelay() const;
    void SetCursorOffset(glm::vec2 offset);
    glm::vec2 GetCursorOffset() const;
    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    // Tooltip widget access
    ToolTipWidget* GetToolTipWidget();

    // Template registration
    void RegisterTemplate(const std::string& name, ToolTipWidget* tooltipWidget);
    void UnregisterTemplate(const std::string& name);
    void UnregisterTemplate(ToolTipWidget* tooltipWidget);
    ToolTipWidget* GetTemplate(const std::string& name);
    bool HasTemplate(const std::string& name) const;

    // Manual control
    void ShowTooltip(Widget* forWidget);
    void HideTooltip();

    // Current state
    Widget* GetHoveredWidget() const;
    bool IsTooltipVisible() const;

    // Lua callback registration
    void SetOnShowCallback(const std::string& funcName);
    void SetOnShowCallbackRef(int luaRef);
    void ClearOnShowCallback();
    void SetOnHideCallback(const std::string& funcName);
    void SetOnHideCallbackRef(int luaRef);
    void ClearOnHideCallback();

private:

    static ToolTipManager* sInstance;

    ToolTipManager();
    ~ToolTipManager();

    void UpdateHoverTracking();
    void PositionTooltip();
    void FireShowCallback(Widget* widget);
    void FireHideCallback(Widget* widget);
    Widget* FindHoveredWidgetWithTooltip();
    void TraverseWidgetsForHover(Widget* widget, Widget*& outHovered);

    // State
    SharedPtr<ToolTipWidget> mDefaultToolTipWidget;
    ToolTipWidget* mActiveTooltipWidget = nullptr;  // Currently shown tooltip (default or template)
    Widget* mHoveredWidget = nullptr;
    Widget* mActiveWidget = nullptr;  // Widget whose tooltip is currently shown
    float mHoverTime = 0.0f;
    bool mTooltipVisible = false;
    bool mEnabled = true;

    // Configuration
    float mShowDelay = 0.5f;
    float mHideDelay = 0.0f;
    glm::vec2 mCursorOffset = glm::vec2(16.0f, 16.0f);

    // Callbacks
    std::string mOnShowFuncName;
    int mOnShowLuaRef = -1;
    std::string mOnHideFuncName;
    int mOnHideLuaRef = -1;

    // Registered tooltip templates (name -> widget)
    std::unordered_map<std::string, ToolTipWidget*> mTemplates;
};
