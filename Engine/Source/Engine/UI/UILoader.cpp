#include "UI/UILoader.h"
#include "UI/UITypes.h"
#include "UI/UIStyleSheet.h"

#include "Nodes/Node.h"
#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/Canvas.h"
#include "Nodes/Widgets/Quad.h"
#include "Nodes/Widgets/Text.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/ArrayWidget.h"
#include "Nodes/Widgets/Poly.h"
#include "Nodes/Widgets/PolyRect.h"
#include "AssetManager.h"
#include "Assets/Texture.h"
#include "Assets/Font.h"
#include "Log.h"

#include "IrrXML/irrXML.h"

#include <cstdio>
#include <cstring>
#include <stack>
#include <algorithm>

using namespace irr;
using namespace io;

//------------------------------------------------------------
// String-based IrrXML reader callback
//------------------------------------------------------------

class StringReadCallback : public IFileReadCallBack
{
public:
    StringReadCallback(const char* data, int size)
        : mData(data), mSize(size), mPos(0) {}

    int read(void* buffer, int sizeToRead) override
    {
        int remaining = mSize - mPos;
        int toRead = (sizeToRead < remaining) ? sizeToRead : remaining;
        if (toRead > 0)
        {
            memcpy(buffer, mData + mPos, toRead);
            mPos += toRead;
        }
        return toRead;
    }

    int getSize() override { return mSize; }

private:
    const char* mData;
    int mSize;
    int mPos;
};

//------------------------------------------------------------
// File reading utility
//------------------------------------------------------------

static std::string ReadTextFile(const std::string& path)
{
    std::string result;
    FILE* file = fopen(path.c_str(), "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (size > 0)
        {
            result.resize((size_t)size);
            fread(&result[0], 1, (size_t)size, file);
        }
        fclose(file);
    }
    return result;
}

//------------------------------------------------------------
// Extract directory from path
//------------------------------------------------------------

static std::string GetDirectory(const std::string& path)
{
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos)
        return path.substr(0, pos + 1);
    return "";
}

//------------------------------------------------------------
// Resolve a relative path against a base directory
//------------------------------------------------------------

static std::string ResolvePath(const std::string& basePath, const std::string& relativePath)
{
    if (relativePath.empty()) return "";

    // If relative path looks absolute, return as-is
    if (relativePath[0] == '/' || relativePath[0] == '\\')
        return relativePath;
    if (relativePath.size() > 1 && relativePath[1] == ':')
        return relativePath;

    // Strip leading "./" prefix
    std::string rel = relativePath;
    while (rel.size() >= 2 && rel[0] == '.' && (rel[1] == '/' || rel[1] == '\\'))
        rel = rel.substr(2);

    return basePath + rel;
}

//------------------------------------------------------------
// Check for data binding placeholders: {{variableName}}
//------------------------------------------------------------

static bool HasDataBinding(const std::string& text)
{
    return text.find("{{") != std::string::npos;
}

//------------------------------------------------------------
// Apply direct XML attributes to a widget
//------------------------------------------------------------

static void ApplyDirectAttributes(
    Widget* widget,
    IrrXMLReader* xml,
    const char* elementName,
    UILoadResult& result)
{
    int attrCount = xml->getAttributeCount();

    for (int i = 0; i < attrCount; ++i)
    {
        const char* name = xml->getAttributeName(i);
        const char* val  = xml->getAttributeValue(i);

        if (!name || !val) continue;

        // Skip meta-attributes handled elsewhere
        if (strcmp(name, "id") == 0) continue;
        if (strcmp(name, "class") == 0) continue;
        if (strcmp(name, "style") == 0) continue;
        if (strncmp(name, "on-", 3) == 0) continue;

        //--- Direct widget property attributes ---

        if (strcmp(name, "name") == 0)
        {
            widget->SetName(val);
            continue;
        }

        // Image source
        if (strcmp(name, "src") == 0)
        {
            if (widget->Is("Quad"))
            {
                Texture* tex = LoadAsset<Texture>(val);
                if (tex) static_cast<Quad*>(widget)->SetTexture(tex);
            }
            else if (widget->Is("Button"))
            {
                Texture* tex = LoadAsset<Texture>(val);
                if (tex) static_cast<Button*>(widget)->SetNormalTexture(tex);
            }
            continue;
        }

        // Font
        if (strcmp(name, "font") == 0)
        {
            Font* font = LoadAsset<Font>(val);
            if (font)
            {
                if (widget->Is("Text"))
                    static_cast<Text*>(widget)->SetFont(font);
                else if (widget->Is("Button"))
                {
                    Text* text = static_cast<Button*>(widget)->GetText();
                    if (text) text->SetFont(font);
                }
            }
            continue;
        }

        // ArrayWidget-specific
        if (strcmp(name, "direction") == 0 && widget->Is("ArrayWidget"))
        {
            // ArrayWidget direction is set via properties/GatherProperties
            // We'll handle this as a CSS property
            CSSValue v = CSSValue::MakeEnum(val);
            ApplyCSSProperty(widget, elementName, "flex-direction", v);
            continue;
        }

        if (strcmp(name, "spacing") == 0 && widget->Is("ArrayWidget"))
        {
            // Spacing - parse as number, apply via property
            CSSValue v = ParseCSSLength(val);
            ApplyCSSProperty(widget, elementName, "gap", v);
            continue;
        }

        // Width / height shorthand
        if (strcmp(name, "width") == 0)
        {
            CSSValue v = ParseCSSLength(val);
            ApplyCSSProperty(widget, elementName, "width", v);
            continue;
        }
        if (strcmp(name, "height") == 0)
        {
            CSSValue v = ParseCSSLength(val);
            ApplyCSSProperty(widget, elementName, "height", v);
            continue;
        }

        // Anchor
        if (strcmp(name, "anchor") == 0)
        {
            CSSValue v = CSSValue::MakeEnum(val);
            ApplyCSSProperty(widget, elementName, "anchor", v);
            continue;
        }

        // Text content via attribute
        if (strcmp(name, "text") == 0)
        {
            if (widget->Is("Text"))
                static_cast<Text*>(widget)->SetText(val);
            else if (widget->Is("Button"))
                static_cast<Button*>(widget)->SetTextString(val);
            continue;
        }

        // Opacity
        if (strcmp(name, "opacity") == 0)
        {
            CSSValue v = ParseCSSLength(val);
            ApplyCSSProperty(widget, elementName, "opacity", v);
            continue;
        }

        // Color
        if (strcmp(name, "color") == 0)
        {
            CSSValue v = ParseCSSColor(val);
            ApplyCSSProperty(widget, elementName, "color", v);
            continue;
        }

        // Background color
        if (strcmp(name, "background-color") == 0 || strcmp(name, "bg-color") == 0)
        {
            CSSValue v = ParseCSSColor(val);
            ApplyCSSProperty(widget, elementName, "background-color", v);
            continue;
        }

        // Font size
        if (strcmp(name, "font-size") == 0)
        {
            CSSValue v = ParseCSSLength(val);
            ApplyCSSProperty(widget, elementName, "font-size", v);
            continue;
        }

        // Word wrap
        if (strcmp(name, "word-wrap") == 0)
        {
            if (widget->Is("Text"))
            {
                bool wrap = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0 || strcmp(val, "break-word") == 0);
                static_cast<Text*>(widget)->EnableWordWrap(wrap);
            }
            continue;
        }

        // Visible
        if (strcmp(name, "visible") == 0)
        {
            bool vis = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
            widget->SetVisible(vis);
            continue;
        }

        // Active
        if (strcmp(name, "active") == 0)
        {
            bool act = (strcmp(val, "true") == 0 || strcmp(val, "1") == 0);
            widget->SetActive(act);
            continue;
        }
    }
}

//------------------------------------------------------------
// Parse event attributes (on-click, on-hover, etc.)
//------------------------------------------------------------

static void ParseEventAttributes(
    IrrXMLReader* xml,
    const std::string& elementId,
    std::vector<UIEventBinding>& outBindings)
{
    int attrCount = xml->getAttributeCount();

    for (int i = 0; i < attrCount; ++i)
    {
        const char* name = xml->getAttributeName(i);
        const char* val  = xml->getAttributeValue(i);

        if (!name || !val) continue;

        if (strncmp(name, "on-", 3) == 0)
        {
            UIEventBinding binding;
            binding.mElementId = elementId;
            binding.mEventName = name + 3; // skip "on-"
            binding.mHandlerName = val;
            outBindings.push_back(binding);
        }
    }
}

//------------------------------------------------------------
// UILoader::Load
//------------------------------------------------------------

bool UILoader::Load(
    const std::string& xmlSource,
    const std::string& basePath,
    UILoadResult& outResult)
{
    if (xmlSource.empty())
    {
        LogError("UILoader: Empty XML source");
        return false;
    }

    // Create IrrXML reader from string
    StringReadCallback callback(xmlSource.c_str(), (int)xmlSource.size());
    IrrXMLReader* xml = createIrrXMLReader(&callback);

    if (!xml)
    {
        LogError("UILoader: Failed to create XML reader");
        return false;
    }

    // Style sheet (accumulated from <link> and <style> elements)
    UIStyleSheet styleSheet;

    // Parent stack for building tree hierarchy
    struct StackEntry
    {
        Widget* widget;
        std::string elementName;
        std::string id;
        std::string cssClass;
    };
    std::stack<StackEntry> parentStack;

    // Root canvas to hold everything
    NodePtr rootNodePtr = Node::Construct("Canvas");
    if (!rootNodePtr)
    {
        LogError("UILoader: Failed to construct root Canvas");
        delete xml;
        return false;
    }

    Widget* rootWidget = static_cast<Widget*>(rootNodePtr.Get());
    rootWidget->SetName("UIRoot");
    rootWidget->SetAnchorMode(AnchorMode::FullStretch);
    rootWidget->SetWidthRatio(1.0f);
    rootWidget->SetHeightRatio(1.0f);

    outResult.mRootNodePtr = rootNodePtr;  // Transfer ownership to keep tree alive
    outResult.mRootWidget = rootWidget;

    // Push root as first parent
    StackEntry rootEntry;
    rootEntry.widget = rootWidget;
    rootEntry.elementName = "ui";
    parentStack.push(rootEntry);

    bool insideStyleBlock = false;
    std::string styleBlockContent;
    std::string pendingTextContent;
    std::string pendingElementName;

    while (xml->read())
    {
        switch (xml->getNodeType())
        {
        case EXN_ELEMENT:
        {
            const char* nodeName = xml->getNodeName();
            if (!nodeName) break;

            // Handle <ui> root element
            if (strcmp(nodeName, "ui") == 0 || strcmp(nodeName, "UI") == 0)
            {
                // Apply attributes to root
                const char* idAttr = xml->getAttributeValue("id");
                const char* classAttr = xml->getAttributeValue("class");
                const char* styleAttr = xml->getAttributeValue("style");

                if (idAttr)
                {
                    rootWidget->SetName(idAttr);
                    outResult.mIdMap[idAttr] = rootWidget;
                }
                if (classAttr)
                {
                    outResult.mClassMap[classAttr].push_back(rootWidget);
                }
                if (styleAttr)
                {
                    std::vector<std::pair<std::string, CSSValue>> inlineProps;
                    ParseInlineStyle(styleAttr, inlineProps);
                    for (auto& p : inlineProps)
                        ApplyCSSProperty(rootWidget, "ui", p.first, p.second);
                }

                if (xml->isEmptyElement()) break;
                continue;
            }

            // Handle <link href="style.css" />
            if (strcmp(nodeName, "link") == 0)
            {
                const char* href = xml->getAttributeValue("href");
                if (href)
                {
                    std::string cssPath = ResolvePath(basePath, href);
                    std::string cssSource = ReadTextFile(cssPath);
                    if (!cssSource.empty())
                    {
                        styleSheet.ParseAndAppend(cssSource);
                    }
                    else
                    {
                        LogWarning("UILoader: Could not load CSS file: %s  (basePath='%s'  href='%s')", cssPath.c_str(), basePath.c_str(), href);
                    }
                }
                break; // <link> is always self-closing
            }

            // Handle <style> block
            if (strcmp(nodeName, "style") == 0)
            {
                insideStyleBlock = true;
                styleBlockContent.clear();
                if (xml->isEmptyElement())
                    insideStyleBlock = false;
                break;
            }

            // Handle <include src="other.xml" />
            if (strcmp(nodeName, "include") == 0)
            {
                const char* srcAttr = xml->getAttributeValue("src");
                if (srcAttr && !parentStack.empty())
                {
                    std::string includePath = ResolvePath(basePath, srcAttr);
                    std::string includeSource = ReadTextFile(includePath);
                    if (!includeSource.empty())
                    {
                        std::string includeDir = GetDirectory(includePath);
                        UILoadResult includeResult;
                        if (UILoader::Load(includeSource, includeDir, includeResult))
                        {
                            if (includeResult.mRootWidget)
                            {
                                parentStack.top().widget->AddChild(includeResult.mRootWidget);

                                // Merge maps
                                for (auto& kv : includeResult.mIdMap)
                                    outResult.mIdMap[kv.first] = kv.second;
                                for (auto& kv : includeResult.mClassMap)
                                {
                                    auto& vec = outResult.mClassMap[kv.first];
                                    vec.insert(vec.end(), kv.second.begin(), kv.second.end());
                                }
                                for (auto& eb : includeResult.mEventBindings)
                                    outResult.mEventBindings.push_back(eb);
                                for (auto& db : includeResult.mDataBindings)
                                    outResult.mDataBindings.push_back(db);
                            }
                        }
                        else
                        {
                            LogWarning("UILoader: Failed to load include: %s", includePath.c_str());
                        }
                    }
                }
                break;
            }

            //--- Regular widget element ---

            const char* widgetTypeName = UIElementToWidgetType(nodeName);
            if (!widgetTypeName)
            {
                // Try using the element name directly as a factory type
                widgetTypeName = nodeName;
            }

            NodePtr nodePtr = Node::Construct(widgetTypeName);
            if (!nodePtr)
            {
                LogWarning("UILoader: Unknown element type '%s', skipping", nodeName);
                // Push a null entry so we can still pop correctly on element_end
                StackEntry nullEntry;
                nullEntry.widget = nullptr;
                nullEntry.elementName = nodeName;
                if (!xml->isEmptyElement())
                    parentStack.push(nullEntry);
                break;
            }

            Widget* widget = static_cast<Widget*>(nodePtr.Get());

            // Read id and class attributes
            const char* idAttr = xml->getAttributeValueSafe("id");
            const char* classAttr = xml->getAttributeValueSafe("class");
            const char* styleAttr = xml->getAttributeValue("style");

            std::string elemId = idAttr ? idAttr : "";
            std::string elemClass = classAttr ? classAttr : "";

            // Set widget name from id
            if (!elemId.empty())
            {
                widget->SetName(elemId);
                outResult.mIdMap[elemId] = widget;
            }

            // Store class associations
            if (!elemClass.empty())
            {
                // Split by spaces and add to each class
                const char* p = elemClass.c_str();
                while (*p)
                {
                    while (*p && isspace((unsigned char)*p)) p++;
                    const char* start = p;
                    while (*p && !isspace((unsigned char)*p)) p++;
                    if (p > start)
                    {
                        std::string cls(start, p);
                        outResult.mClassMap[cls].push_back(widget);
                    }
                }
            }

            // Apply direct XML attributes
            ApplyDirectAttributes(widget, xml, nodeName, outResult);

            // Apply stylesheet rules (element, class, id selectors)
            styleSheet.ApplyStyles(widget, nodeName, elemClass.c_str(), elemId.c_str());

            // Apply inline style (highest specificity)
            if (styleAttr)
            {
                std::vector<std::pair<std::string, CSSValue>> inlineProps;
                ParseInlineStyle(styleAttr, inlineProps);
                for (auto& p : inlineProps)
                    ApplyCSSProperty(widget, nodeName, p.first, p.second);
            }

            // Parse event bindings
            if (!elemId.empty())
            {
                ParseEventAttributes(xml, elemId, outResult.mEventBindings);
            }

            // Add to parent
            if (!parentStack.empty() && parentStack.top().widget)
            {
                parentStack.top().widget->AddChild(widget);
            }

            // Push onto stack (if not self-closing)
            if (!xml->isEmptyElement())
            {
                StackEntry entry;
                entry.widget = widget;
                entry.elementName = nodeName;
                entry.id = elemId;
                entry.cssClass = elemClass;
                parentStack.push(entry);
            }

            pendingTextContent.clear();
            pendingElementName = nodeName;
            break;
        }

        case EXN_TEXT:
        {
            const char* textData = xml->getNodeData();
            if (!textData) break;

            if (insideStyleBlock)
            {
                styleBlockContent += textData;
                break;
            }

            // Trim whitespace
            std::string trimmed = textData;
            size_t start = trimmed.find_first_not_of(" \t\r\n");
            size_t end = trimmed.find_last_not_of(" \t\r\n");
            if (start == std::string::npos) break;
            trimmed = trimmed.substr(start, end - start + 1);

            if (trimmed.empty()) break;

            if (!parentStack.empty() && parentStack.top().widget)
            {
                Widget* parent = parentStack.top().widget;
                const std::string& elemName = parentStack.top().elementName;

                // Set text content on the parent widget
                if (parent->Is("Text"))
                {
                    static_cast<Text*>(parent)->SetText(trimmed);

                    // Check for data bindings
                    if (HasDataBinding(trimmed))
                    {
                        UIDataBinding db;
                        db.mWidget = parent;
                        db.mTemplateText = trimmed;
                        outResult.mDataBindings.push_back(db);
                    }
                }
                else if (parent->Is("Button"))
                {
                    static_cast<Button*>(parent)->SetTextString(trimmed);

                    // Check for data bindings on button text
                    if (HasDataBinding(trimmed))
                    {
                        Text* btnText = static_cast<Button*>(parent)->GetText();
                        if (btnText)
                        {
                            UIDataBinding db;
                            db.mWidget = btnText;
                            db.mTemplateText = trimmed;
                            outResult.mDataBindings.push_back(db);
                        }
                    }
                }
                else if (elemName == "p" || elemName == "span" || elemName == "text")
                {
                    // These are text elements, parent should be Text
                    if (parent->Is("Text"))
                    {
                        static_cast<Text*>(parent)->SetText(trimmed);
                    }
                }
            }
            break;
        }

        case EXN_ELEMENT_END:
        {
            const char* nodeName = xml->getNodeName();

            if (insideStyleBlock && nodeName && strcmp(nodeName, "style") == 0)
            {
                insideStyleBlock = false;
                if (!styleBlockContent.empty())
                {
                    styleSheet.ParseAndAppend(styleBlockContent);
                }
                styleBlockContent.clear();
                break;
            }

            if (nodeName && (strcmp(nodeName, "ui") == 0 || strcmp(nodeName, "UI") == 0))
            {
                // Pop root
                if (!parentStack.empty())
                    parentStack.pop();
                break;
            }

            // Pop from stack
            if (!parentStack.empty() && parentStack.size() > 1)
            {
                parentStack.pop();
            }
            break;
        }

        default:
            break;
        }
    }

    delete xml;

    // After building the full tree with all stylesheets loaded,
    // do a second pass to apply styles to elements that were created
    // before their stylesheet was loaded (via <link> after the element).
    // This handles the common case of <link> at the top of the document.
    // In practice, <link> should come before content, but we handle both.

    return true;
}
