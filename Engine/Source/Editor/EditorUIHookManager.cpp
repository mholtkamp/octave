#if EDITOR

#include "EditorUIHookManager.h"
#include "Log.h"
#include "InputDevices.h"
#include "Plugins/ImGuiPluginContext.h"

#include "imgui.h"
#include "imgui_dock.h"

#include <algorithm>
#include <cctype>

void GetImGuiPluginContext(ImGuiPluginContext* outCtx)
{
    if (outCtx)
    {
        outCtx->context = ImGui::GetCurrentContext();
        ImGui::GetAllocatorFunctions(
            &outCtx->allocFunc,
            &outCtx->freeFunc,
            &outCtx->allocUserData
        );
    }
}

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

    // ===== Batch 1: Menu Positioning & Top-Level Menu Control =====

    mHooks.AddTopLevelMenuItemEx = [](HookId hookId, const char* menuName,
                                       TopLevelMenuDrawCallback drawFunc, void* userData, int32_t position) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredTopLevelMenu menu;
        menu.mHookId = hookId;
        menu.mMenuName = menuName ? menuName : "";
        menu.mDrawFunc = drawFunc;
        menu.mUserData = userData;
        menu.mPosition = position;

        mgr->mTopLevelMenus.push_back(menu);
    };

    mHooks.AddMenuItemEx = [](HookId hookId, const char* menuPath, const char* itemPath,
                               MenuCallback callback, void* userData, const char* shortcut,
                               MenuValidationCallback validateFunc) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredMenuItemEx item;
        item.mHookId = hookId;
        item.mMenuPath = menuPath ? menuPath : "";
        item.mItemPath = itemPath ? itemPath : "";
        item.mCallback = callback;
        item.mUserData = userData;
        item.mShortcut = shortcut ? shortcut : "";
        item.mValidateFunc = validateFunc;

        mgr->mMenuItemsEx.push_back(item);
    };

    // ===== Batch 2: Create/Spawn Menu Extensions =====

    mHooks.AddNodeMenuItems = [](HookId hookId, const char* category,
                                  MenuSectionDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredNodeMenuItems item;
        item.mHookId = hookId;
        item.mCategory = category ? category : "";
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mNodeMenuItems.push_back(item);
    };

    mHooks.RemoveNodeMenuItems = [](HookId hookId, const char* category) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string cat = category ? category : "";
        mgr->mNodeMenuItems.erase(std::remove_if(mgr->mNodeMenuItems.begin(), mgr->mNodeMenuItems.end(),
            [hookId, &cat](const RegisteredNodeMenuItems& item) {
                return item.mHookId == hookId && item.mCategory == cat;
            }), mgr->mNodeMenuItems.end());
    };

    mHooks.AddCreateAssetItems = [](HookId hookId, MenuSectionDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredCreateAssetItems item;
        item.mHookId = hookId;
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mCreateAssetItems.push_back(item);
    };

    mHooks.RemoveCreateAssetItems = [](HookId hookId) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        mgr->mCreateAssetItems.erase(std::remove_if(mgr->mCreateAssetItems.begin(), mgr->mCreateAssetItems.end(),
            [hookId](const RegisteredCreateAssetItems& item) {
                return item.mHookId == hookId;
            }), mgr->mCreateAssetItems.end());
    };

    mHooks.AddSpawnBasic3dItems = [](HookId hookId, MenuSectionDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredSpawnItems item;
        item.mHookId = hookId;
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mSpawnBasic3dItems.push_back(item);
    };

    mHooks.AddSpawnBasicWidgetItems = [](HookId hookId, MenuSectionDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredSpawnItems item;
        item.mHookId = hookId;
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mSpawnBasicWidgetItems.push_back(item);
    };

    // ===== Scene Type Registration =====

    mHooks.RegisterSceneType = [](HookId hookId, const char* typeName,
                                   SceneCreationCallback createFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredSceneType sceneType;
        sceneType.mHookId = hookId;
        sceneType.mTypeName = typeName ? typeName : "";
        sceneType.mCreateFunc = createFunc;
        sceneType.mUserData = userData;

        mgr->mSceneTypes.push_back(sceneType);
    };

    mHooks.UnregisterSceneType = [](HookId hookId, const char* typeName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = typeName ? typeName : "";
        mgr->mSceneTypes.erase(std::remove_if(mgr->mSceneTypes.begin(), mgr->mSceneTypes.end(),
            [hookId, &name](const RegisteredSceneType& st) {
                return st.mHookId == hookId && st.mTypeName == name;
            }), mgr->mSceneTypes.end());
    };

    // ===== Batch 3: Viewport Context Menu & Overlay Drawing =====

    mHooks.AddViewportContextItem = [](HookId hookId, const char* itemPath,
                                        MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredViewportContextItem item;
        item.mHookId = hookId;
        item.mItemPath = itemPath ? itemPath : "";
        item.mCallback = callback;
        item.mUserData = userData;

        mgr->mViewportContextItems.push_back(item);
    };

    mHooks.RemoveViewportContextItem = [](HookId hookId, const char* itemPath) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string path = itemPath ? itemPath : "";
        mgr->mViewportContextItems.erase(std::remove_if(mgr->mViewportContextItems.begin(), mgr->mViewportContextItems.end(),
            [hookId, &path](const RegisteredViewportContextItem& item) {
                return item.mHookId == hookId && item.mItemPath == path;
            }), mgr->mViewportContextItems.end());
    };

    mHooks.RegisterViewportOverlay = [](HookId hookId, const char* overlayName,
                                         ViewportOverlayCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredViewportOverlay overlay;
        overlay.mHookId = hookId;
        overlay.mOverlayName = overlayName ? overlayName : "";
        overlay.mDrawFunc = drawFunc;
        overlay.mUserData = userData;

        mgr->mViewportOverlays.push_back(overlay);
    };

    mHooks.UnregisterViewportOverlay = [](HookId hookId, const char* overlayName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = overlayName ? overlayName : "";
        mgr->mViewportOverlays.erase(std::remove_if(mgr->mViewportOverlays.begin(), mgr->mViewportOverlays.end(),
            [hookId, &name](const RegisteredViewportOverlay& overlay) {
                return overlay.mHookId == hookId && overlay.mOverlayName == name;
            }), mgr->mViewportOverlays.end());
    };

    // ===== Batch 4: Custom Preferences/Settings Pages =====

    mHooks.RegisterPreferencesPanel = [](HookId hookId, const char* panelName, const char* panelCategory,
                                          PreferencesPanelDrawCallback drawFunc,
                                          PreferencesLoadCallback loadFunc, PreferencesSaveCallback saveFunc,
                                          void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredPreferencesPanel panel;
        panel.mHookId = hookId;
        panel.mPanelName = panelName ? panelName : "";
        panel.mPanelCategory = panelCategory ? panelCategory : "";
        panel.mDrawFunc = drawFunc;
        panel.mLoadFunc = loadFunc;
        panel.mSaveFunc = saveFunc;
        panel.mUserData = userData;

        mgr->mPreferencesPanels.push_back(panel);
    };

    mHooks.UnregisterPreferencesPanel = [](HookId hookId, const char* panelName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = panelName ? panelName : "";
        mgr->mPreferencesPanels.erase(std::remove_if(mgr->mPreferencesPanels.begin(), mgr->mPreferencesPanels.end(),
            [hookId, &name](const RegisteredPreferencesPanel& panel) {
                return panel.mHookId == hookId && panel.mPanelName == name;
            }), mgr->mPreferencesPanels.end());
    };

    // ===== Batch 5: Custom Keyboard Shortcuts =====

    mHooks.RegisterShortcut = [](HookId hookId, const char* shortcutId, const char* displayName,
                                  const char* defaultBinding, ShortcutCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredShortcut sc;
        sc.mHookId = hookId;
        sc.mShortcutId = shortcutId ? shortcutId : "";
        sc.mDisplayName = displayName ? displayName : "";
        sc.mDefaultBinding = defaultBinding ? defaultBinding : "";
        sc.mCallback = callback;
        sc.mUserData = userData;

        mgr->ParseKeyBinding(sc);
        mgr->mShortcuts.push_back(sc);
    };

    mHooks.UnregisterShortcut = [](HookId hookId, const char* shortcutId) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string id = shortcutId ? shortcutId : "";
        mgr->mShortcuts.erase(std::remove_if(mgr->mShortcuts.begin(), mgr->mShortcuts.end(),
            [hookId, &id](const RegisteredShortcut& sc) {
                return sc.mHookId == hookId && sc.mShortcutId == id;
            }), mgr->mShortcuts.end());
    };

    // ===== Batch 6: Property Drawer System =====

    mHooks.RegisterPropertyDrawer = [](HookId hookId, const char* propertyTypeName,
                                        PropertyDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredPropertyDrawer drawer;
        drawer.mHookId = hookId;
        drawer.mPropertyTypeName = propertyTypeName ? propertyTypeName : "";
        drawer.mDrawFunc = drawFunc;
        drawer.mUserData = userData;

        mgr->mPropertyDrawers.push_back(drawer);
    };

    mHooks.UnregisterPropertyDrawer = [](HookId hookId, const char* propertyTypeName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string typeName = propertyTypeName ? propertyTypeName : "";
        mgr->mPropertyDrawers.erase(std::remove_if(mgr->mPropertyDrawers.begin(), mgr->mPropertyDrawers.end(),
            [hookId, &typeName](const RegisteredPropertyDrawer& drawer) {
                return drawer.mHookId == hookId && drawer.mPropertyTypeName == typeName;
            }), mgr->mPropertyDrawers.end());
    };

    // ===== Batch 7: Hierarchy & Asset Browser Extensions =====

    mHooks.RegisterHierarchyItemGUI = [](HookId hookId, HierarchyItemGUICallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredHierarchyItemGUI item;
        item.mHookId = hookId;
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mHierarchyItemGUI.push_back(item);
    };

    mHooks.UnregisterHierarchyItemGUI = [](HookId hookId) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        mgr->mHierarchyItemGUI.erase(std::remove_if(mgr->mHierarchyItemGUI.begin(), mgr->mHierarchyItemGUI.end(),
            [hookId](const RegisteredHierarchyItemGUI& item) {
                return item.mHookId == hookId;
            }), mgr->mHierarchyItemGUI.end());
    };

    mHooks.RegisterAssetItemGUI = [](HookId hookId, AssetItemGUICallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredAssetItemGUI item;
        item.mHookId = hookId;
        item.mDrawFunc = drawFunc;
        item.mUserData = userData;

        mgr->mAssetItemGUI.push_back(item);
    };

    mHooks.UnregisterAssetItemGUI = [](HookId hookId) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        mgr->mAssetItemGUI.erase(std::remove_if(mgr->mAssetItemGUI.begin(), mgr->mAssetItemGUI.end(),
            [hookId](const RegisteredAssetItemGUI& item) {
                return item.mHookId == hookId;
            }), mgr->mAssetItemGUI.end());
    };

    mHooks.RegisterOnHierarchyChanged = [](HookId hookId, HierarchyChangedCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnHierarchyChanged.push_back({hookId, cb, userData});
    };

    // ===== Batch 8: Additional Context Menus =====

    mHooks.AddSceneTabContextItem = [](HookId hookId, const char* itemPath,
                                        MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mSceneTabContextItems.push_back({hookId, itemPath ? itemPath : "", callback, userData});
    };

    mHooks.AddDebugLogContextItem = [](HookId hookId, const char* itemPath,
                                        MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mDebugLogContextItems.push_back({hookId, itemPath ? itemPath : "", callback, userData});
    };

    mHooks.AddImportMenuItem = [](HookId hookId, const char* itemPath,
                                   MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mImportMenuItems.push_back({hookId, itemPath ? itemPath : "", callback, userData});
    };

    mHooks.AddAddonsMenuItem = [](HookId hookId, const char* itemPath,
                                   MenuCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mAddonsMenuItems.push_back({hookId, itemPath ? itemPath : "", callback, userData});
    };

    mHooks.AddPlayTarget = [](HookId hookId, const char* targetName, const char* iconText,
                               PlayTargetCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredPlayTarget target;
        target.mHookId = hookId;
        target.mTargetName = targetName ? targetName : "";
        target.mIconText = iconText ? iconText : "";
        target.mCallback = callback;
        target.mUserData = userData;

        mgr->mPlayTargets.push_back(target);
    };

    mHooks.RemovePlayTarget = [](HookId hookId, const char* targetName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = targetName ? targetName : "";
        mgr->mPlayTargets.erase(std::remove_if(mgr->mPlayTargets.begin(), mgr->mPlayTargets.end(),
            [hookId, &name](const RegisteredPlayTarget& target) {
                return target.mHookId == hookId && target.mTargetName == name;
            }), mgr->mPlayTargets.end());
    };

    // ===== Batch 9: Drag-Drop Enhancement & Asset Pipeline =====

    mHooks.RegisterDragDropHandler = [](HookId hookId, const char* targetArea,
                                         DragDropHandlerCallback handler, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredDragDropHandler h;
        h.mHookId = hookId;
        h.mTargetArea = targetArea ? targetArea : "";
        h.mHandler = handler;
        h.mUserData = userData;

        mgr->mDragDropHandlers.push_back(h);
    };

    mHooks.RegisterAssetImporter = [](HookId hookId, const char* extension,
                                       AssetImportCallback importFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredAssetImporter imp;
        imp.mHookId = hookId;
        imp.mExtension = extension ? extension : "";
        imp.mImportFunc = importFunc;
        imp.mUserData = userData;

        mgr->mAssetImporters.push_back(imp);
    };

    mHooks.UnregisterAssetImporter = [](HookId hookId, const char* extension) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string ext = extension ? extension : "";
        mgr->mAssetImporters.erase(std::remove_if(mgr->mAssetImporters.begin(), mgr->mAssetImporters.end(),
            [hookId, &ext](const RegisteredAssetImporter& imp) {
                return imp.mHookId == hookId && imp.mExtension == ext;
            }), mgr->mAssetImporters.end());
    };

    mHooks.RegisterOnPreAssetImport = [](HookId hookId, PreImportCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPreAssetImport.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnPostAssetImport = [](HookId hookId, StringEventCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPostAssetImport.push_back({hookId, cb, userData});
    };

    // ===== Batch 10: Build Pipeline & Editor State =====

    mHooks.RegisterOnPreBuild = [](HookId hookId, PreBuildCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPreBuild.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnPostBuild = [](HookId hookId, PackageFinishedCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnPostBuild.push_back({hookId, cb, userData});
    };

    mHooks.RegisterOnEditorModeChanged = [](HookId hookId, EditorModeCallback cb, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;
        mgr->mOnEditorModeChanged.push_back({hookId, cb, userData});
    };

    // ===== Game Preview Resolution Presets =====

    mHooks.AddGamePreviewResolution = [](HookId hookId, const char* name, uint32_t width, uint32_t height) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredGamePreviewResolution preset;
        preset.mHookId = hookId;
        preset.mName = name ? name : "";
        preset.mWidth = width;
        preset.mHeight = height;

        mgr->mGamePreviewResolutions.push_back(preset);
    };

    mHooks.RemoveGamePreviewResolution = [](HookId hookId, const char* name) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string n = name ? name : "";
        mgr->mGamePreviewResolutions.erase(std::remove_if(mgr->mGamePreviewResolutions.begin(), mgr->mGamePreviewResolutions.end(),
            [hookId, &n](const RegisteredGamePreviewResolution& preset) {
                return preset.mHookId == hookId && preset.mName == n;
            }), mgr->mGamePreviewResolutions.end());
    };

    mHooks.RegisterGizmoTool = [](HookId hookId, const char* toolName, const char* iconText,
                                   const char* tooltip, GizmoToolDrawCallback drawFunc, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredGizmoTool tool;
        tool.mHookId = hookId;
        tool.mToolName = toolName ? toolName : "";
        tool.mIconText = iconText ? iconText : "";
        tool.mTooltip = tooltip ? tooltip : "";
        tool.mDrawFunc = drawFunc;
        tool.mUserData = userData;
        tool.mIsActive = false;

        mgr->mGizmoTools.push_back(tool);
    };

    mHooks.UnregisterGizmoTool = [](HookId hookId, const char* toolName) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string name = toolName ? toolName : "";
        mgr->mGizmoTools.erase(std::remove_if(mgr->mGizmoTools.begin(), mgr->mGizmoTools.end(),
            [hookId, &name](const RegisteredGizmoTool& tool) {
                return tool.mHookId == hookId && tool.mToolName == name;
            }), mgr->mGizmoTools.end());
    };

    // ===== Controller Server Extension Hooks =====

    mHooks.RegisterControllerRoute = [](HookId hookId, const char* method, const char* path, ControllerRouteCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        RegisteredControllerRoute route;
        route.mHookId = hookId;
        route.mMethod = method ? method : "";
        route.mPath = path ? path : "";
        route.mCallback = callback;
        route.mUserData = userData;

        mgr->mControllerRoutes.push_back(route);
    };

    mHooks.UnregisterControllerRoute = [](HookId hookId, const char* path) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        std::string p = path ? path : "";
        mgr->mControllerRoutes.erase(std::remove_if(mgr->mControllerRoutes.begin(), mgr->mControllerRoutes.end(),
            [hookId, &p](const RegisteredControllerRoute& route) {
                return route.mHookId == hookId && route.mPath == p;
            }), mgr->mControllerRoutes.end());
    };

    mHooks.RegisterOnControllerServerStateChanged = [](HookId hookId, ControllerServerEventCallback callback, void* userData) {
        EditorUIHookManager* mgr = EditorUIHookManager::Get();
        if (mgr == nullptr) return;

        mgr->mOnControllerServerStateChanged.push_back({hookId, callback, userData});
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
    bool hasLegacy = (it != mMenuItems.end() && !it->second.empty());

    if (hasLegacy)
    {
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

    // Also draw MenuItemEx entries for this menu
    for (const RegisteredMenuItemEx& item : mMenuItemsEx)
    {
        if (item.mMenuPath != menuPath) continue;

        bool enabled = true;
        if (item.mValidateFunc)
        {
            enabled = item.mValidateFunc(item.mUserData);
        }

        const char* shortcut = item.mShortcut.empty() ? nullptr : item.mShortcut.c_str();
        if (ImGui::MenuItem(item.mItemPath.c_str(), shortcut, false, enabled))
        {
            if (item.mCallback)
            {
                item.mCallback(item.mUserData);
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

    // New hook types
    removeByHookId(mMenuItemsEx);
    removeByHookId(mNodeMenuItems);
    removeByHookId(mSceneTypes);
    removeByHookId(mCreateAssetItems);
    removeByHookId(mSpawnBasic3dItems);
    removeByHookId(mSpawnBasicWidgetItems);
    removeByHookId(mViewportContextItems);
    removeByHookId(mViewportOverlays);
    removeByHookId(mPreferencesPanels);
    removeByHookId(mShortcuts);
    removeByHookId(mPropertyDrawers);
    removeByHookId(mHierarchyItemGUI);
    removeByHookId(mAssetItemGUI);
    removeByHookId(mOnHierarchyChanged);
    removeByHookId(mSceneTabContextItems);
    removeByHookId(mDebugLogContextItems);
    removeByHookId(mImportMenuItems);
    removeByHookId(mAddonsMenuItems);
    removeByHookId(mPlayTargets);
    removeByHookId(mDragDropHandlers);
    removeByHookId(mAssetImporters);
    removeByHookId(mOnPreAssetImport);
    removeByHookId(mOnPostAssetImport);
    removeByHookId(mOnPreBuild);
    removeByHookId(mOnPostBuild);
    removeByHookId(mOnEditorModeChanged);
    removeByHookId(mGizmoTools);
    removeByHookId(mGamePreviewResolutions);
    removeByHookId(mControllerRoutes);
    removeByHookId(mOnControllerServerStateChanged);
}

// ===== Top-Level Menus and Toolbar Drawing =====

void EditorUIHookManager::DrawTopLevelMenus()
{
    // Draw only menus with position == -1 (legacy append behavior)
    for (const RegisteredTopLevelMenu& menu : mTopLevelMenus)
    {
        if (menu.mPosition != -1) continue;

        if (ImGui::BeginMenu(menu.mMenuName.c_str()))
        {
            if (menu.mDrawFunc)
            {
                menu.mDrawFunc(menu.mUserData);
            }
            ImGui::EndMenu();
        }
    }
}

void EditorUIHookManager::DrawTopLevelMenusAtPosition(int32_t builtinPosition)
{
    for (const RegisteredTopLevelMenu& menu : mTopLevelMenus)
    {
        if (menu.mPosition != builtinPosition) continue;

        if (ImGui::BeginMenu(menu.mMenuName.c_str()))
        {
            if (menu.mDrawFunc)
            {
                menu.mDrawFunc(menu.mUserData);
            }
            ImGui::EndMenu();
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

// ===== Batch 2: Create/Spawn Menu Extensions =====

void EditorUIHookManager::DrawNodeMenuItems(const char* category, void* parentNode)
{
    for (const RegisteredNodeMenuItems& item : mNodeMenuItems)
    {
        if (item.mCategory == category && item.mDrawFunc)
        {
            ImGui::Separator();
            item.mDrawFunc(parentNode, item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawCustomNodeCategories(void* parentNode)
{
    // Collect unique custom category names (not built-in ones)
    std::vector<std::string> customCategories;
    for (const RegisteredNodeMenuItems& item : mNodeMenuItems)
    {
        if (item.mCategory != "3D" && item.mCategory != "Widget" &&
            item.mCategory != "Scene" && item.mCategory != "Other")
        {
            bool found = false;
            for (const std::string& cat : customCategories)
            {
                if (cat == item.mCategory) { found = true; break; }
            }
            if (!found) customCategories.push_back(item.mCategory);
        }
    }

    for (const std::string& category : customCategories)
    {
        if (ImGui::BeginMenu(category.c_str()))
        {
            for (const RegisteredNodeMenuItems& item : mNodeMenuItems)
            {
                if (item.mCategory == category && item.mDrawFunc)
                {
                    item.mDrawFunc(parentNode, item.mUserData);
                }
            }
            ImGui::EndMenu();
        }
    }
}

void EditorUIHookManager::DrawCreateAssetItems()
{
    for (const RegisteredCreateAssetItems& item : mCreateAssetItems)
    {
        if (item.mDrawFunc)
        {
            ImGui::Separator();
            item.mDrawFunc(nullptr, item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawSpawnBasic3dItems(void* parentNode)
{
    for (const RegisteredSpawnItems& item : mSpawnBasic3dItems)
    {
        if (item.mDrawFunc)
        {
            ImGui::Separator();
            item.mDrawFunc(parentNode, item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawSpawnBasicWidgetItems(void* parentNode)
{
    for (const RegisteredSpawnItems& item : mSpawnBasicWidgetItems)
    {
        if (item.mDrawFunc)
        {
            ImGui::Separator();
            item.mDrawFunc(parentNode, item.mUserData);
        }
    }
}

// ===== Scene Type Registration =====

void EditorUIHookManager::FireSceneCreation(const std::string& typeName, const char* sceneName, void* rootNode)
{
    for (const RegisteredSceneType& st : mSceneTypes)
    {
        if (st.mTypeName == typeName && st.mCreateFunc)
        {
            st.mCreateFunc(sceneName, rootNode, st.mUserData);
            return;
        }
    }
}

// ===== Batch 3: Viewport Context Menu & Overlay Drawing =====

void EditorUIHookManager::DrawViewportContextItems()
{
    for (const RegisteredViewportContextItem& item : mViewportContextItems)
    {
        if (ImGui::MenuItem(item.mItemPath.c_str()))
        {
            if (item.mCallback)
            {
                item.mCallback(item.mUserData);
            }
        }
    }
}

void EditorUIHookManager::DrawViewportOverlays(float viewportX, float viewportY, float viewportW, float viewportH)
{
    for (const RegisteredViewportOverlay& overlay : mViewportOverlays)
    {
        if (overlay.mDrawFunc)
        {
            overlay.mDrawFunc(viewportX, viewportY, viewportW, viewportH, overlay.mUserData);
        }
    }
}

bool EditorUIHookManager::HasViewportContextItems() const
{
    return !mViewportContextItems.empty();
}

// ===== Batch 4: Preferences =====

void EditorUIHookManager::LoadAddonPreferences()
{
    for (const RegisteredPreferencesPanel& panel : mPreferencesPanels)
    {
        if (panel.mLoadFunc)
        {
            panel.mLoadFunc(panel.mUserData);
        }
    }
}

void EditorUIHookManager::SaveAddonPreferences()
{
    for (const RegisteredPreferencesPanel& panel : mPreferencesPanels)
    {
        if (panel.mSaveFunc)
        {
            panel.mSaveFunc(panel.mUserData);
        }
    }
}

// ===== Batch 5: Keyboard Shortcuts =====

void EditorUIHookManager::ParseKeyBinding(RegisteredShortcut& shortcut)
{
    // Parse "Ctrl+Shift+Alt+X" format
    std::string binding = shortcut.mDefaultBinding;
    shortcut.mCtrl = false;
    shortcut.mShift = false;
    shortcut.mAlt = false;
    shortcut.mKeyCode = -1;

    // Convert to uppercase for comparison
    std::string upper = binding;
    for (char& c : upper) c = (char)toupper((unsigned char)c);

    // Check modifiers
    if (upper.find("CTRL+") != std::string::npos) shortcut.mCtrl = true;
    if (upper.find("SHIFT+") != std::string::npos) shortcut.mShift = true;
    if (upper.find("ALT+") != std::string::npos) shortcut.mAlt = true;

    // Get the key (last part after the last '+')
    size_t lastPlus = upper.rfind('+');
    std::string key = (lastPlus != std::string::npos) ? upper.substr(lastPlus + 1) : upper;

    // Map common key names to OCTAVE_KEY_* constants from InputTypes.h
    if (key.length() == 1 && key[0] >= 'A' && key[0] <= 'Z')
    {
        shortcut.mKeyCode = OCTAVE_KEY_A + (key[0] - 'A');
    }
    else if (key.length() == 1 && key[0] >= '0' && key[0] <= '9')
    {
        shortcut.mKeyCode = OCTAVE_KEY_0 + (key[0] - '0');
    }
    else if (key == "F1")  shortcut.mKeyCode = OCTAVE_KEY_F1;
    else if (key == "F2")  shortcut.mKeyCode = OCTAVE_KEY_F2;
    else if (key == "F3")  shortcut.mKeyCode = OCTAVE_KEY_F3;
    else if (key == "F4")  shortcut.mKeyCode = OCTAVE_KEY_F4;
    else if (key == "F5")  shortcut.mKeyCode = OCTAVE_KEY_F5;
    else if (key == "F6")  shortcut.mKeyCode = OCTAVE_KEY_F6;
    else if (key == "F7")  shortcut.mKeyCode = OCTAVE_KEY_F7;
    else if (key == "F8")  shortcut.mKeyCode = OCTAVE_KEY_F8;
    else if (key == "F9")  shortcut.mKeyCode = OCTAVE_KEY_F9;
    else if (key == "F10") shortcut.mKeyCode = OCTAVE_KEY_F10;
    else if (key == "F11") shortcut.mKeyCode = OCTAVE_KEY_F11;
    else if (key == "F12") shortcut.mKeyCode = OCTAVE_KEY_F12;
    else if (key == "SPACE") shortcut.mKeyCode = OCTAVE_KEY_SPACE;
    else if (key == "ENTER" || key == "RETURN") shortcut.mKeyCode = OCTAVE_KEY_ENTER;
    else if (key == "ESCAPE" || key == "ESC") shortcut.mKeyCode = OCTAVE_KEY_ESCAPE;
    else if (key == "TAB") shortcut.mKeyCode = OCTAVE_KEY_TAB;
    else if (key == "DELETE" || key == "DEL") shortcut.mKeyCode = OCTAVE_KEY_DELETE;
    else if (key == "BACKSPACE") shortcut.mKeyCode = OCTAVE_KEY_BACKSPACE;
}

void EditorUIHookManager::ProcessShortcuts()
{
    // Don't process shortcuts when typing in text fields
    if (ImGui::GetIO().WantTextInput) return;

    for (const RegisteredShortcut& sc : mShortcuts)
    {
        if (sc.mKeyCode < 0 || sc.mCallback == nullptr) continue;

        // Check modifier key state using engine's input system
        bool ctrlMatch = sc.mCtrl ? IsControlDown() : !IsControlDown();
        bool shiftMatch = sc.mShift ? IsShiftDown() : !IsShiftDown();
        bool altMatch = sc.mAlt ? IsAltDown() : !IsAltDown();

        if (ctrlMatch && shiftMatch && altMatch && IsKeyJustDown(sc.mKeyCode))
        {
            sc.mCallback(sc.mUserData);
        }
    }
}

// ===== Batch 6: Property Drawers =====

bool EditorUIHookManager::DrawPropertyDrawer(const char* propertyTypeName, const char* propertyName,
                                              void* propertyOwner, int32_t propertyType)
{
    for (const RegisteredPropertyDrawer& drawer : mPropertyDrawers)
    {
        if (drawer.mPropertyTypeName == propertyTypeName && drawer.mDrawFunc)
        {
            if (drawer.mDrawFunc(propertyName, propertyOwner, propertyType, drawer.mUserData))
            {
                return true;
            }
        }
    }
    return false;
}

// ===== Batch 7: Hierarchy & Asset Browser Extensions =====

void EditorUIHookManager::DrawHierarchyItemGUI(void* node, float rowX, float rowY, float rowW, float rowH)
{
    for (const RegisteredHierarchyItemGUI& item : mHierarchyItemGUI)
    {
        if (item.mDrawFunc)
        {
            item.mDrawFunc(node, rowX, rowY, rowW, rowH, item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawAssetItemGUI(const char* assetName, const char* assetType,
                                            float rowX, float rowY, float rowW, float rowH)
{
    for (const RegisteredAssetItemGUI& item : mAssetItemGUI)
    {
        if (item.mDrawFunc)
        {
            item.mDrawFunc(assetName, assetType, rowX, rowY, rowW, rowH, item.mUserData);
        }
    }
}

void EditorUIHookManager::FireOnHierarchyChanged(int32_t changeType, void* node)
{
    for (const auto& entry : mOnHierarchyChanged)
    {
        if (entry.mCallback) entry.mCallback(changeType, node, entry.mUserData);
    }
}

// ===== Batch 8: Additional Context Menus =====

void EditorUIHookManager::DrawSceneTabContextItems()
{
    for (const RegisteredSimpleContextItem& item : mSceneTabContextItems)
    {
        if (ImGui::MenuItem(item.mItemPath.c_str()))
        {
            if (item.mCallback) item.mCallback(item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawDebugLogContextItems()
{
    for (const RegisteredSimpleContextItem& item : mDebugLogContextItems)
    {
        if (ImGui::MenuItem(item.mItemPath.c_str()))
        {
            if (item.mCallback) item.mCallback(item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawImportMenuItems()
{
    for (const RegisteredSimpleContextItem& item : mImportMenuItems)
    {
        if (ImGui::MenuItem(item.mItemPath.c_str()))
        {
            if (item.mCallback) item.mCallback(item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawAddonsMenuItems()
{
    for (const RegisteredSimpleContextItem& item : mAddonsMenuItems)
    {
        if (ImGui::MenuItem(item.mItemPath.c_str()))
        {
            if (item.mCallback) item.mCallback(item.mUserData);
        }
    }
}

void EditorUIHookManager::DrawPlayTargets()
{
    for (const RegisteredPlayTarget& target : mPlayTargets)
    {
        if (ImGui::Selectable(target.mTargetName.c_str()))
        {
            if (target.mCallback) target.mCallback(target.mUserData);
        }
    }
}

bool EditorUIHookManager::HasPlayTargets() const
{
    return !mPlayTargets.empty();
}

// ===== Batch 9: Drag-Drop & Asset Pipeline =====

bool EditorUIHookManager::HandleDragDrop(const char* targetArea, const char* payloadType,
                                          const void* payloadData, int32_t payloadSize)
{
    for (const RegisteredDragDropHandler& handler : mDragDropHandlers)
    {
        if (handler.mTargetArea == targetArea && handler.mHandler)
        {
            if (handler.mHandler(payloadType, payloadData, payloadSize, handler.mUserData))
            {
                return true;
            }
        }
    }
    return false;
}

bool EditorUIHookManager::HandleAssetImport(const char* filePath, const char* extension)
{
    for (const RegisteredAssetImporter& imp : mAssetImporters)
    {
        if (imp.mExtension == extension && imp.mImportFunc)
        {
            if (imp.mImportFunc(filePath, extension, imp.mUserData))
            {
                return true;
            }
        }
    }
    return false;
}

bool EditorUIHookManager::FireOnPreAssetImport(const char* filePath)
{
    for (const auto& entry : mOnPreAssetImport)
    {
        if (entry.mCallback && !entry.mCallback(filePath, entry.mUserData))
        {
            return false; // Cancelled
        }
    }
    return true;
}

void EditorUIHookManager::FireOnPostAssetImport(const char* assetPath)
{
    for (const auto& entry : mOnPostAssetImport)
    {
        if (entry.mCallback) entry.mCallback(assetPath, entry.mUserData);
    }
}

// ===== Batch 10: Build Pipeline & Editor State =====

bool EditorUIHookManager::FireOnPreBuild(int32_t platform)
{
    for (const auto& entry : mOnPreBuild)
    {
        if (entry.mCallback && !entry.mCallback(platform, entry.mUserData))
        {
            return false; // Cancelled
        }
    }
    return true;
}

void EditorUIHookManager::FireOnPostBuild(int32_t platform, bool success)
{
    for (const auto& entry : mOnPostBuild)
    {
        if (entry.mCallback) entry.mCallback(platform, success, entry.mUserData);
    }
}

void EditorUIHookManager::FireOnEditorModeChanged(int32_t newMode)
{
    for (const auto& entry : mOnEditorModeChanged)
    {
        if (entry.mCallback) entry.mCallback(newMode, entry.mUserData);
    }
}

void EditorUIHookManager::DrawGizmoTools(void* selectedNode)
{
    for (RegisteredGizmoTool& tool : mGizmoTools)
    {
        ImGui::SameLine();
        if (tool.mIsActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
        if (ImGui::Button(tool.mIconText.c_str()))
        {
            tool.mIsActive = !tool.mIsActive;
        }
        if (tool.mIsActive) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered() && !tool.mTooltip.empty())
        {
            ImGui::SetTooltip("%s", tool.mTooltip.c_str());
        }

        if (tool.mIsActive && tool.mDrawFunc && selectedNode)
        {
            tool.mDrawFunc(selectedNode, tool.mUserData);
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

void EditorUIHookManager::FireOnControllerServerStateChanged(int32_t state)
{
    for (const auto& entry : mOnControllerServerStateChanged)
    {
        if (entry.mCallback) entry.mCallback(state, entry.mUserData);
    }
}

#endif // EDITOR
