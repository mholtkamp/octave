#include "UI/UIDocument.h"
#include "UI/UILoader.h"
#include "UI/UITypes.h"

#include "Nodes/Widgets/Widget.h"
#include "Nodes/Widgets/Button.h"
#include "Nodes/Widgets/Text.h"
#include "Stream.h"
#include "Log.h"
#include "Utilities.h"

#include <cstdio>
#include <algorithm>

FORCE_LINK_DEF(UIDocument);
DEFINE_ASSET(UIDocument);

static const char* sDefaultXml =
    "<ui>\n"
    "  <style>\n"
    "    .root {\n"
    "      anchor: full-stretch;\n"
    "    }\n"
    "  </style>\n"
    "\n"
    "  <div id=\"root\" class=\"root\">\n"
    "  </div>\n"
    "</ui>\n";

//------------------------------------------------------------
// Constructor / Destructor
//------------------------------------------------------------

UIDocument::UIDocument()
{
    mType = UIDocument::GetStaticType();
}

UIDocument::~UIDocument()
{
    DestroyWidgetTree();
}

//------------------------------------------------------------
// Asset Interface
//------------------------------------------------------------

void UIDocument::Create()
{
    Asset::Create();

    if (mXmlSource.empty())
    {
        mXmlSource = sDefaultXml;
    }
}

void UIDocument::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    stream.ReadString(mXmlSource);
    stream.ReadString(mBasePath);
    stream.ReadString(mSourceFilePath);
}

void UIDocument::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

    stream.WriteString(mXmlSource);
    stream.WriteString(mBasePath);
    stream.WriteString(mSourceFilePath);
}

bool UIDocument::Import(const std::string& path, ImportOptions* options)
{
    // Read the XML file
    FILE* file = fopen(path.c_str(), "rb");
    if (!file)
    {
        LogError("UIDocument: Failed to open file: %s", path.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    mXmlSource.resize((size_t)size);
    fread(&mXmlSource[0], 1, (size_t)size, file);
    fclose(file);

    // Extract base path for resolving relative references
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos)
        mBasePath = path.substr(0, lastSlash + 1);
    else
        mBasePath = "";

    mSourceFilePath = path;
    mName = Asset::GetNameFromPath(path);

    return true;
}

void UIDocument::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);
    outProps.push_back(Property(DatumType::String, "Source File", this, &mSourceFilePath));
}

glm::vec4 UIDocument::GetTypeColor()
{
    return { 0.2f, 0.8f, 0.6f, 1.0f };
}

const char* UIDocument::GetTypeName()
{
    return "UIDocument";
}

const char* UIDocument::GetTypeImportExt()
{
    return "xml";
}

//------------------------------------------------------------
// Instantiate
//------------------------------------------------------------

Widget* UIDocument::Instantiate()
{
    DestroyWidgetTree();

    // In editor builds, re-read the XML from the source file if available.
    // This allows iterating on XML/CSS without needing to reimport.
    if (!mSourceFilePath.empty())
    {
        FILE* srcFile = fopen(mSourceFilePath.c_str(), "rb");
        if (srcFile)
        {
            fseek(srcFile, 0, SEEK_END);
            long size = ftell(srcFile);
            fseek(srcFile, 0, SEEK_SET);
            if (size > 0)
            {
                mXmlSource.resize((size_t)size);
                fread(&mXmlSource[0], 1, (size_t)size, srcFile);

                // Update base path from source file path
                size_t lastSlash = mSourceFilePath.find_last_of("/\\");
                if (lastSlash != std::string::npos)
                    mBasePath = mSourceFilePath.substr(0, lastSlash + 1);

                LogDebug("UIDocument: Reloaded XML from source file: %s", mSourceFilePath.c_str());
            }
            fclose(srcFile);
        }
    }

    if (mXmlSource.empty())
    {
        LogError("UIDocument::Instantiate: No XML source loaded");
        return nullptr;
    }

    mLoadResult = UILoadResult();

    bool success = UILoader::Load(mXmlSource, mBasePath, mLoadResult);
    if (!success || !mLoadResult.mRootWidget)
    {
        LogError("UIDocument::Instantiate: Failed to parse XML");
        return nullptr;
    }

    mRootWidget = mLoadResult.mRootWidget;

    // Wire up event bindings from XML on-* attributes
    for (const UIEventBinding& eb : mLoadResult.mEventBindings)
    {
        std::string key = eb.mElementId + ":" + eb.mEventName;
        EventCallback& cb = mCallbacks[key];
        cb.mFuncName = eb.mHandlerName;
    }

    // Set up button trackers for event detection
    mButtonTrackers.clear();
    for (auto& kv : mLoadResult.mIdMap)
    {
        Widget* w = kv.second;
        if (w && w->Is("Button"))
        {
            ButtonTracker tracker;
            tracker.mButton = static_cast<Button*>(w);
            tracker.mPrevState = ButtonState::Normal;
            tracker.mElementId = kv.first;
            mButtonTrackers.push_back(tracker);
        }
    }

    // Apply initial data bindings
    UpdateDataBindings();

    return mRootWidget;
}

//------------------------------------------------------------
// Mount / Unmount
//------------------------------------------------------------

void UIDocument::Mount(Widget* parent)
{
    if (!parent)
    {
        LogError("UIDocument::Mount: Null parent");
        return;
    }

    if (!mRootWidget)
    {
        Instantiate();
    }

    if (mRootWidget)
    {
        mMountParent = parent;
        parent->AddChild(mRootWidget);

        // Propagate the parent's target screen to the entire UI widget tree
        uint8_t targetScreen = parent->GetTargetScreen();
        mRootWidget->Traverse([targetScreen](Node* node) -> bool {
            node->SetTargetScreen(targetScreen);
            return true;
        });
    }
}

void UIDocument::Unmount()
{
    if (mRootWidget && mMountParent)
    {
        // Find and remove the root widget from the mount parent
        for (uint32_t i = 0; i < mMountParent->GetNumChildren(); ++i)
        {
            if (mMountParent->GetChild(i) == mRootWidget)
            {
                mMountParent->RemoveChild(i);
                break;
            }
        }
    }
    mMountParent = nullptr;
}

//------------------------------------------------------------
// Queries
//------------------------------------------------------------

Widget* UIDocument::FindById(const std::string& id) const
{
    auto it = mLoadResult.mIdMap.find(id);
    if (it != mLoadResult.mIdMap.end())
        return it->second;
    return nullptr;
}

std::vector<Widget*> UIDocument::FindByClass(const std::string& className) const
{
    auto it = mLoadResult.mClassMap.find(className);
    if (it != mLoadResult.mClassMap.end())
        return it->second;
    return {};
}

Widget* UIDocument::GetRootWidget() const
{
    return mRootWidget;
}

//------------------------------------------------------------
// Data Binding
//------------------------------------------------------------

static const std::string sEmptyString;

void UIDocument::SetData(const std::string& key, const std::string& value)
{
    mDataStore[key] = value;
    UpdateDataBindings();
}

const std::string& UIDocument::GetData(const std::string& key) const
{
    auto it = mDataStore.find(key);
    if (it != mDataStore.end())
        return it->second;
    return sEmptyString;
}

void UIDocument::UpdateDataBindings()
{
    for (UIDataBinding& db : mLoadResult.mDataBindings)
    {
        if (!db.mWidget) continue;

        std::string result = db.mTemplateText;

        // Replace all {{key}} with data values
        size_t pos = 0;
        while ((pos = result.find("{{", pos)) != std::string::npos)
        {
            size_t end = result.find("}}", pos + 2);
            if (end == std::string::npos) break;

            std::string key = result.substr(pos + 2, end - pos - 2);

            // Trim whitespace from key
            size_t ks = key.find_first_not_of(" \t");
            size_t ke = key.find_last_not_of(" \t");
            if (ks != std::string::npos)
                key = key.substr(ks, ke - ks + 1);

            auto it = mDataStore.find(key);
            std::string replacement = (it != mDataStore.end()) ? it->second : "";

            result.replace(pos, end - pos + 2, replacement);
            pos += replacement.size();
        }

        // Apply result to the text widget
        if (db.mWidget->Is("Text"))
        {
            static_cast<Text*>(db.mWidget)->SetText(result);
        }
    }
}

//------------------------------------------------------------
// Event Callbacks
//------------------------------------------------------------

void UIDocument::SetCallback(
    const std::string& elementId,
    const std::string& eventName,
    const std::string& funcName)
{
    std::string key = elementId + ":" + eventName;
    mCallbacks[key].mFuncName = funcName;
}

void UIDocument::SetCallbackRef(
    const std::string& elementId,
    const std::string& eventName,
    int luaRef)
{
    std::string key = elementId + ":" + eventName;
    mCallbacks[key].mLuaRef = luaRef;
}

void UIDocument::Tick()
{
    // Check button state transitions for click events
    for (ButtonTracker& tracker : mButtonTrackers)
    {
        if (!tracker.mButton) continue;

        ButtonState curState = tracker.mButton->GetState();

        // Detect click: was Pressed, now Normal or Hovered
        if (tracker.mPrevState == ButtonState::Pressed &&
            (curState == ButtonState::Normal || curState == ButtonState::Hovered))
        {
            // Fire click callback
            std::string key = tracker.mElementId + ":click";
            auto it = mCallbacks.find(key);
            if (it != mCallbacks.end())
            {
                const EventCallback& cb = it->second;
                if (cb.mLuaRef >= 0 || !cb.mFuncName.empty())
                {
                    // Callback dispatch handled by Lua binding layer
                }
            }
        }

        // Detect hover enter
        if (tracker.mPrevState != ButtonState::Hovered &&
            curState == ButtonState::Hovered)
        {
            // Future: fire hover callback
        }

        tracker.mPrevState = curState;
    }
}

//------------------------------------------------------------
// Cleanup
//------------------------------------------------------------

void UIDocument::DestroyWidgetTree()
{
    if (mRootWidget && mMountParent)
    {
        Unmount();
    }

    mRootWidget = nullptr;
    mMountParent = nullptr;
    mLoadResult = UILoadResult();
    mButtonTrackers.clear();
}
