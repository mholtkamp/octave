#pragma once

#include "Nodes/Node.h"

#include <string>
#include <vector>
#include <unordered_map>

class Widget;
class UIStyleSheet;

//------------------------------------------------------------
// Event binding from XML on-* attributes
//------------------------------------------------------------

struct UIEventBinding
{
    std::string mElementId;
    std::string mEventName;     // "click", "hover", "press", "release"
    std::string mHandlerName;   // Lua global function name
};

//------------------------------------------------------------
// Data binding from {{variable}} in text content
//------------------------------------------------------------

struct UIDataBinding
{
    Widget* mWidget = nullptr;
    std::string mTemplateText;  // Original text with {{var}} placeholders
};

//------------------------------------------------------------
// Load result
//------------------------------------------------------------

struct UILoadResult
{
    NodePtr mRootNodePtr;               // Owns the root widget (keeps tree alive)
    Widget* mRootWidget = nullptr;      // Raw pointer for convenience
    std::unordered_map<std::string, Widget*> mIdMap;
    std::unordered_map<std::string, std::vector<Widget*>> mClassMap;
    std::vector<UIEventBinding> mEventBindings;
    std::vector<UIDataBinding> mDataBindings;
};

//------------------------------------------------------------
// UILoader - builds widget tree from XML + CSS
//------------------------------------------------------------

class UILoader
{
public:

    static bool Load(
        const std::string& xmlSource,
        const std::string& basePath,
        UILoadResult& outResult);

private:

    UILoader() = default;
};
