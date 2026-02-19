#if EDITOR

#include "EditorUIHookManager.h"
#include "Log.h"

#include "imgui.h"
#include "imgui_dock.h"

#include <algorithm>

EditorUIHookManager* EditorUIHookManager::sInstance = nullptr;

// ===== Helper Functions Used by InitializeHooks =====

static void Hook_OpenWindow(const char* windowId)
{
    EditorUIHookManager* mgr = EditorUIHookManager::Get();
    if (mgr) mgr->OpenWindow(windowId ? windowId : "");
}

static void Hook_CloseWindow(const char* windowId)
{
    EditorUIHookManager* mgr = EditorUIHookManager::Get();
    if (mgr) mgr->CloseWindow(windowId ? windowId : "");
}

static bool Hook_IsWindowOpen(const char* windowId)
{
    EditorUIHookManager* mgr = EditorUIHookManager::Get();
    return mgr ? mgr->IsWindowOpen(windowId ? windowId : "") : false;
}

static void Hook_RemoveAllHooks(HookId hookId)
{
    EditorUIHookManager* mgr = EditorUIHookManager::Get();
    if (mgr) mgr->RemoveAllHooks(hookId);
}

// ===== EditorUIHookManager Implementation =====

void EditorUIHookManager::Create()
{
    if (sInstance == nullptr)
    {
        sInstance = new EditorUIHookManager();
    }
}

void EditorUIHookManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

EditorUIHookManager* EditorUIHookManager::Get()
{
    return sInstance;
}

EditorUIHookManager::EditorUIHookManager()
{
    InitializeHooks();
}

EditorUIHookManager::~EditorUIHookManager()
{
}

void EditorUIHookManager::InitializeHooks()
{
    // Set up the hooks struct with function pointers
    // These need to call back into this manager

    // For AddMenuItem, we need a static function that can access the manager
    mHooks.AddMenuItem = [](HookId hookId, const char* menuPath, const char* itemPath,
                            MenuCallback callback, void* userData, const char* shortcut) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredMenuItem item;
        item.mHookId = hookId;
        item.mMenuPath = menuPath ? menuPath : "";
        item.mItemPath = itemPath ? itemPath : "";
        item.mCallback = callback;
        item.mUserData = userData;
        item.mShortcut = shortcut ? shortcut : "";
        item.mIsSeparator = false;

        mgr->mMenuItems[item.mMenuPath].push_back(item);
    };

    mHooks.AddMenuSeparator = [](HookId hookId, const char* menuPath) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredMenuItem item;
        item.mHookId = hookId;
        item.mMenuPath = menuPath ? menuPath : "";
        item.mIsSeparator = true;

        mgr->mMenuItems[item.mMenuPath].push_back(item);
    };

    mHooks.RemoveMenuItem = [](HookId hookId, const char* menuPath, const char* itemPath) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string menu = menuPath ? menuPath : "";
        std::string path = itemPath ? itemPath : "";

        auto it = mgr->mMenuItems.find(menu);
        if (it != mgr->mMenuItems.end())
        {
            auto& items = it->second;
            items.erase(std::remove_if(items.begin(), items.end(),
                [hookId, &path](const RegisteredMenuItem& item) {
                    return item.mHookId == hookId && item.mItemPath == path;
                }), items.end());
        }
    };

    mHooks.RegisterWindow = [](HookId hookId, const char* windowName, const char* windowId,
                               WindowDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredWindow win;
        win.mHookId = hookId;
        win.mWindowName = windowName ? windowName : "";
        win.mWindowId = windowId ? windowId : "";
        win.mDrawFunc = drawFunc;
        win.mUserData = userData;
        win.mIsOpen = false;

        mgr->mWindows.push_back(win);
    };

    mHooks.UnregisterWindow = [](HookId hookId, const char* windowId) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string id = windowId ? windowId : "";
        mgr->mWindows.erase(std::remove_if(mgr->mWindows.begin(), mgr->mWindows.end(),
            [hookId, &id](const RegisteredWindow& win) {
                return win.mHookId == hookId && win.mWindowId == id;
            }), mgr->mWindows.end());
    };

    mHooks.OpenWindow = Hook_OpenWindow;
    mHooks.CloseWindow = Hook_CloseWindow;
    mHooks.IsWindowOpen = Hook_IsWindowOpen;

    mHooks.RegisterInspector = [](HookId hookId, const char* nodeTypeName,
                                  InspectorDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredInspector insp;
        insp.mHookId = hookId;
        insp.mNodeTypeName = nodeTypeName ? nodeTypeName : "";
        insp.mDrawFunc = drawFunc;
        insp.mUserData = userData;

        mgr->mInspectors.push_back(insp);
    };

    mHooks.UnregisterInspector = [](HookId hookId, const char* nodeTypeName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string typeName = nodeTypeName ? nodeTypeName : "";
        mgr->mInspectors.erase(std::remove_if(mgr->mInspectors.begin(), mgr->mInspectors.end(),
            [hookId, &typeName](const RegisteredInspector& insp) {
                return insp.mHookId == hookId && insp.mNodeTypeName == typeName;
            }), mgr->mInspectors.end());
    };

    mHooks.AddNodeContextItem = [](HookId hookId, const char* itemPath,
                                   MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredContextItem ctx;
        ctx.mHookId = hookId;
        ctx.mItemPath = itemPath ? itemPath : "";
        ctx.mCallback = callback;
        ctx.mUserData = userData;
        ctx.mIsNodeContext = true;

        mgr->mContextItems.push_back(ctx);
    };

    mHooks.AddAssetContextItem = [](HookId hookId, const char* itemPath,
                                    const char* assetTypeFilter,
                                    MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredContextItem ctx;
        ctx.mHookId = hookId;
        ctx.mItemPath = itemPath ? itemPath : "";
        ctx.mAssetTypeFilter = assetTypeFilter ? assetTypeFilter : "*";
        ctx.mCallback = callback;
        ctx.mUserData = userData;
        ctx.mIsNodeContext = false;

        mgr->mContextItems.push_back(ctx);
    };

    mHooks.RemoveAllHooks = Hook_RemoveAllHooks;

    // ===== Top-Level Menus =====

    mHooks.AddTopLevelMenuItem = [](HookId hookId, const char* menuName,
                                    TopLevelMenuDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredTopLevelMenu menu;
        menu.mHookId = hookId;
        menu.mMenuName = menuName ? menuName : "";
        menu.mDrawFunc = drawFunc;
        menu.mUserData = userData;

        mgr->mTopLevelMenus.push_back(menu);
    };

    mHooks.RemoveTopLevelMenuItem = [](HookId hookId, const char* menuName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = menuName ? menuName : "";
        mgr->mTopLevelMenus.erase(std::remove_if(mgr->mTopLevelMenus.begin(), mgr->mTopLevelMenus.end(),
            [hookId, &name](const RegisteredTopLevelMenu& menu) {
                return menu.mHookId == hookId && menu.mMenuName == name;
            }), mgr->mTopLevelMenus.end());
    };

    // ===== Toolbar =====

    mHooks.AddToolbarItem = [](HookId hookId, const char* itemName,
                               ToolbarDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredToolbarItem item;
        item.mHookId = hookId;
        item.mItemName = itemName ? itemName : "";
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mToolbarItems.push_back(item);
    };

    mHooks.RemoveToolbarItem = [](HookId hookId, const char* itemName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = itemName ? itemName : "";
        mgr->mToolbarItems.erase(std::remove_if(mgr->mToolbarItems.begin(), mgr->mToolbarItems.end(),
            [hookId, &name](const RegisteredToolbarItem& item) {
                return item.mHookId == hookId && item.mItemName == name;
            }), mgr->mToolbarItems.end());
    };

    // ===== Project Lifecycle Events =====

    mHooks.RegisterOnProjectOpen = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnProjectOpen.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnProjectClose = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnProjectClose.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnProjectSave = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnProjectSave.push_back({hookId, cb, userData});
    };

    // ===== Scene Lifecycle Events =====

    mHooks.RegisterOnSceneOpen = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnSceneOpen.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnSceneClose = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnSceneClose.push_back({hookId, cb, userData});
    };

    // ===== Packaging/Build Events =====

    mHooks.RegisterOnPackageStarted = [](HookId hookId, PlatformEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPackageStarted.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnPackageFinished = [](HookId hookId, PackageFinishedCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPackageFinished.push_back({hookId, cb, userData});
    };

    // ===== Editor State Events =====

    mHooks.RegisterOnSelectionChanged = [](HookId hookId, EventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnSelectionChanged.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnPlayModeChanged = [](HookId hookId, PlayModeCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPlayModeChanged.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnEditorShutdown = [](HookId hookId, EventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnEditorShutdown.push_back({hookId, cb, userData});
    };

    // ===== Asset Pipeline Events =====

    mHooks.RegisterOnAssetImported = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetImported.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnAssetDeleted = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetDeleted.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnAssetSaved = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetSaved.push_back({hookId, cb, userData});
    };

    // ===== Asset Open Events =====

    mHooks.RegisterOnAssetOpen = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetOpen.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnAssetOpened = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetOpened.push_back({hookId, cb, userData});
    };

    // ===== Undo/Redo =====

    mHooks.RegisterOnUndoRedo = [](HookId hookId, EventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnUndoRedo.push_back({hookId, cb, userData});
    };

    // ===== Drag-and-Drop Events =====

    mHooks.RegisterOnAssetDropHierarchy = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetDropHierarchy.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnAssetDropViewport = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnAssetDropViewport.push_back({hookId, cb, userData});
    };
}

const std::vector<RegisteredMenuItem>& EditorUIHookManager::GetMenuItems(const std::string& menuPath) const
{
    auto it = mMenuItems.find(menuPath);
    if (it != mMenuItems.end())
    {
        return it->second;
    }
    return mEmptyMenuItems;
}

void EditorUIHookManager::DrawMenuItems(const std::string& menuPath)
{
    auto it = mMenuItems.find(menuPath);
    if (it == mMenuItems.end() || it->second.empty())
    {
        return;
    }

    for (const RegisteredMenuItem& item : it->second)
    {
        if (item.mIsSeparator)
        {
            ImGui::Separator();
        }
        else
        {
            const char* shortcut = item.mShortcut.empty() ? nullptr : item.mShortcut.c_str();
            if (ImGui::MenuItem(item.mItemPath.c_str(), shortcut))
            {
                if (item.mCallback)
                {
                    item.mCallback(item.mUserData);
                }
            }
        }
    }
}

void EditorUIHookManager::DrawWindows()
{
    for (RegisteredWindow& win : mWindows)
    {
        if (win.mIsOpen)
        {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
            if (ImGui::BeginDock(win.mWindowName.c_str(), &win.mIsOpen))
            {
                if (win.mDrawFunc)
                {
                    win.mDrawFunc(win.mUserData);
                }
            }
            ImGui::EndDock();
            ImGui::PopStyleColor();
        }
    }
}

void EditorUIHookManager::OpenWindow(const std::string& windowId)
{
    for (RegisteredWindow& win : mWindows)
    {
        if (win.mWindowId == windowId)
        {
            win.mIsOpen = true;
            return;
        }
    }
}

void EditorUIHookManager::CloseWindow(const std::string& windowId)
{
    for (RegisteredWindow& win : mWindows)
    {
        if (win.mWindowId == windowId)
        {
            win.mIsOpen = false;
            return;
        }
    }
}

bool EditorUIHookManager::IsWindowOpen(const std::string& windowId) const
{
    for (const RegisteredWindow& win : mWindows)
    {
        if (win.mWindowId == windowId)
        {
            return win.mIsOpen;
        }
    }
    return false;
}

const RegisteredInspector* EditorUIHookManager::GetInspector(const std::string& nodeTypeName) const
{
    for (const RegisteredInspector& insp : mInspectors)
    {
        if (insp.mNodeTypeName == nodeTypeName)
        {
            return &insp;
        }
    }
    return nullptr;
}

bool EditorUIHookManager::DrawInspector(const std::string& nodeTypeName, void* node)
{
    const RegisteredInspector* insp = GetInspector(nodeTypeName);
    if (insp && insp->mDrawFunc)
    {
        insp->mDrawFunc(node, insp->mUserData);
        return true;
    }
    return false;
}

void EditorUIHookManager::DrawNodeContextItems()
{
    for (const RegisteredContextItem& ctx : mContextItems)
    {
        if (ctx.mIsNodeContext)
        {
            if (ImGui::MenuItem(ctx.mItemPath.c_str()))
            {
                if (ctx.mCallback)
                {
                    ctx.mCallback(ctx.mUserData);
                }
            }
        }
    }
}

void EditorUIHookManager::DrawAssetContextItems(const std::string& assetType)
{
    for (const RegisteredContextItem& ctx : mContextItems)
    {
        if (!ctx.mIsNodeContext)
        {
            // Check if this item applies to this asset type
            if (ctx.mAssetTypeFilter == "*" || ctx.mAssetTypeFilter == assetType)
            {
                if (ImGui::MenuItem(ctx.mItemPath.c_str()))
                {
                    if (ctx.mCallback)
                    {
                        ctx.mCallback(ctx.mUserData);
                    }
                }
            }
        }
    }
}

void EditorUIHookManager::RemoveAllHooks(HookId hookId)
{
    // Remove menu items
    for (auto& pair : mMenuItems)
    {
        pair.second.erase(std::remove_if(pair.second.begin(), pair.second.end(),
            [hookId](const RegisteredMenuItem& item) {
                return item.mHookId == hookId;
            }), pair.second.end());
    }

    // Remove windows
    mWindows.erase(std::remove_if(mWindows.begin(), mWindows.end(),
        [hookId](const RegisteredWindow& win) {
            return win.mHookId == hookId;
        }), mWindows.end());

    // Remove inspectors
    mInspectors.erase(std::remove_if(mInspectors.begin(), mInspectors.end(),
        [hookId](const RegisteredInspector& insp) {
            return insp.mHookId == hookId;
        }), mInspectors.end());

    // Remove context items
    mContextItems.erase(std::remove_if(mContextItems.begin(), mContextItems.end(),
        [hookId](const RegisteredContextItem& ctx) {
            return ctx.mHookId == hookId;
        }), mContextItems.end());

    // Remove top-level menus
    mTopLevelMenus.erase(std::remove_if(mTopLevelMenus.begin(), mTopLevelMenus.end(),
        [hookId](const RegisteredTopLevelMenu& menu) {
            return menu.mHookId == hookId;
        }), mTopLevelMenus.end());

    // Remove toolbar items
    mToolbarItems.erase(std::remove_if(mToolbarItems.begin(), mToolbarItems.end(),
        [hookId](const RegisteredToolbarItem& item) {
            return item.mHookId == hookId;
        }), mToolbarItems.end());

    // Remove event callbacks - helper lambda
    auto removeByHookId = [hookId](auto& vec) {
        vec.erase(std::remove_if(vec.begin(), vec.end(),
            [hookId](const auto& entry) {
                return entry.mHookId == hookId;
            }), vec.end());
    };

    removeByHookId(mOnProjectOpen);
    removeByHookId(mOnProjectClose);
    removeByHookId(mOnProjectSave);
    removeByHookId(mOnSceneOpen);
    removeByHookId(mOnSceneClose);
    removeByHookId(mOnPackageStarted);
    removeByHookId(mOnPackageFinished);
    removeByHookId(mOnSelectionChanged);
    removeByHookId(mOnPlayModeChanged);
    removeByHookId(mOnEditorShutdown);
    removeByHookId(mOnAssetImported);
    removeByHookId(mOnAssetDeleted);
    removeByHookId(mOnAssetSaved);
    removeByHookId(mOnAssetOpen);
    removeByHookId(mOnAssetOpened);
    removeByHookId(mOnUndoRedo);
    removeByHookId(mOnAssetDropHierarchy);
    removeByHookId(mOnAssetDropViewport);
}

// ===== Top-Level Menus and Toolbar Drawing =====

void EditorUIHookManager::DrawTopLevelMenus()
{
    for (const RegisteredTopLevelMenu& menu : mTopLevelMenus)
    {
        ImGui::SameLine();
        if (ImGui::Button(menu.mMenuName.c_str()))
        {
            ImGui::OpenPopup(menu.mMenuName.c_str());
        }
    }

    for (const RegisteredTopLevelMenu& menu : mTopLevelMenus)
    {
        if (ImGui::BeginPopup(menu.mMenuName.c_str()))
        {
            if (menu.mDrawFunc)
            {
                menu.mDrawFunc(menu.mUserData);
            }
            ImGui::EndPopup();
        }
    }
}

void EditorUIHookManager::DrawToolbarItems()
{
    for (const RegisteredToolbarItem& item : mToolbarItems)
    {
        ImGui::SameLine();
        if (item.mDrawFunc)
        {
            item.mDrawFunc(item.mUserData);
        }
    }
}

// ===== Event Dispatchers =====

void EditorUIHookManager::FireOnProjectOpen(const char* projectPath)
{
    for (const auto& entry : mOnProjectOpen)
    {
        if (entry.mCallback) entry.mCallback(projectPath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnProjectClose(const char* projectPath)
{
    for (const auto& entry : mOnProjectClose)
    {
        if (entry.mCallback) entry.mCallback(projectPath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnProjectSave(const char* filePath)
{
    for (const auto& entry : mOnProjectSave)
    {
        if (entry.mCallback) entry.mCallback(filePath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnSceneOpen(const char* scenePath)
{
    for (const auto& entry : mOnSceneOpen)
    {
        if (entry.mCallback) entry.mCallback(scenePath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnSceneClose(const char* scenePath)
{
    for (const auto& entry : mOnSceneClose)
    {
        if (entry.mCallback) entry.mCallback(scenePath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnPackageStarted(int32_t platform)
{
    for (const auto& entry : mOnPackageStarted)
    {
        if (entry.mCallback) entry.mCallback(platform, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnPackageFinished(int32_t platform, bool success)
{
    for (const auto& entry : mOnPackageFinished)
    {
        if (entry.mCallback) entry.mCallback(platform, success, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnSelectionChanged()
{
    for (const auto& entry : mOnSelectionChanged)
    {
        if (entry.mCallback) entry.mCallback(entry.mUserData);
    }
}

void EditorUIHookManager::FireOnPlayModeChanged(int32_t state)
{
    for (const auto& entry : mOnPlayModeChanged)
    {
        if (entry.mCallback) entry.mCallback(state, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnEditorShutdown()
{
    for (const auto& entry : mOnEditorShutdown)
    {
        if (entry.mCallback) entry.mCallback(entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetImported(const char* assetPath)
{
    for (const auto& entry : mOnAssetImported)
    {
        if (entry.mCallback) entry.mCallback(assetPath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetDeleted(const char* assetPath)
{
    for (const auto& entry : mOnAssetDeleted)
    {
        if (entry.mCallback) entry.mCallback(assetPath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetSaved(const char* assetPath)
{
    for (const auto& entry : mOnAssetSaved)
    {
        if (entry.mCallback) entry.mCallback(assetPath, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetOpen(const char* assetName)
{
    for (const auto& entry : mOnAssetOpen)
    {
        if (entry.mCallback) entry.mCallback(assetName, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetOpened(const char* assetName)
{
    for (const auto& entry : mOnAssetOpened)
    {
        if (entry.mCallback) entry.mCallback(assetName, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnUndoRedo()
{
    for (const auto& entry : mOnUndoRedo)
    {
        if (entry.mCallback) entry.mCallback(entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetDropHierarchy(const char* assetName)
{
    for (const auto& entry : mOnAssetDropHierarchy)
    {
        if (entry.mCallback) entry.mCallback(assetName, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnAssetDropViewport(const char* assetName)
{
    for (const auto& entry : mOnAssetDropViewport)
    {
        if (entry.mCallback) entry.mCallback(assetName, entry.mUserData);
    }
}

HookId GenerateHookId(const char* identifier)
{
    if (identifier == nullptr)
    {
        return 0;
    }

    // Simple hash function
    uint64_t hash = 0;
    while (*identifier)
    {
        hash = hash * 31 + static_cast<uint64_t>(*identifier);
        ++identifier;
    }
    return hash;
}

#endif // EDITOR
