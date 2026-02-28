#pragma once

#include "Asset.h"
#include "UI/UILoader.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

class Widget;
class Button;
enum class ButtonState : uint8_t;

class UIDocument : public Asset
{
public:

    DECLARE_ASSET(UIDocument, Asset);

    UIDocument();
    virtual ~UIDocument();

    // Asset interface
    virtual void Create() override;
    virtual void LoadStream(Stream& stream, Platform platform) override;
    virtual void SaveStream(Stream& stream, Platform platform) override;
    virtual bool Import(const std::string& path, ImportOptions* options) override;
    virtual void GatherProperties(std::vector<Property>& outProps) override;
    virtual glm::vec4 GetTypeColor() override;
    virtual const char* GetTypeName() override;
    virtual const char* GetTypeImportExt() override;

    void SetXmlSource(const std::string& xml) { mXmlSource = xml; }

    // Instantiate widget tree (does not attach to any parent)
    Widget* Instantiate();

    // Mount the instantiated tree under a parent widget
    void Mount(Widget* parent);

    // Unmount and destroy the widget tree
    void Unmount();

    // Query
    Widget* FindById(const std::string& id) const;
    std::vector<Widget*> FindByClass(const std::string& className) const;
    Widget* GetRootWidget() const;

    // Data binding: replaces {{key}} in text content
    void SetData(const std::string& key, const std::string& value);
    const std::string& GetData(const std::string& key) const;

    // Event callbacks
    void SetCallback(const std::string& elementId, const std::string& eventName, const std::string& funcName);
    void SetCallbackRef(const std::string& elementId, const std::string& eventName, int luaRef);

    // Call each frame to process events (button clicks, etc.)
    void Tick();

    const std::string& GetXmlSource() const { return mXmlSource; }
    const std::string& GetBasePath() const { return mBasePath; }
    void SetBasePath(const std::string& basePath) { mBasePath = basePath; }
    const std::string& GetSourceFilePath() const { return mSourceFilePath; }
    void SetSourceFilePath(const std::string& path) { mSourceFilePath = path; }

private:

    void UpdateDataBindings();
    void DestroyWidgetTree();

    // Source data
    std::string mXmlSource;
    std::string mBasePath;
    std::string mSourceFilePath; // Path to the .xml file on disk

    // Runtime state (after Instantiate)
    Widget* mRootWidget = nullptr;
    Widget* mMountParent = nullptr;
    UILoadResult mLoadResult;

    // Data binding
    std::unordered_map<std::string, std::string> mDataStore;

    // Event system
    struct EventCallback
    {
        std::string mFuncName;
        int mLuaRef = -1; // LUA_NOREF
    };
    // Key: "elementId:eventName"
    std::unordered_map<std::string, EventCallback> mCallbacks;

    struct ButtonTracker
    {
        Button* mButton = nullptr;
        ButtonState mPrevState;
        std::string mElementId;
    };
    std::vector<ButtonTracker> mButtonTrackers;
};
