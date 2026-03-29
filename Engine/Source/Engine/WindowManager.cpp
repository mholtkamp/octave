#include "WindowManager.h"
#include "Nodes/Widgets/Window.h"
#include "Nodes/Node.h"
#include "Log.h"

WindowManager* WindowManager::sInstance = nullptr;

void WindowManager::Create()
{
    OCT_ASSERT(sInstance == nullptr);
    sInstance = new WindowManager();
}

void WindowManager::Destroy()
{
    if (sInstance != nullptr)
    {
        delete sInstance;
        sInstance = nullptr;
    }
}

WindowManager* WindowManager::Get()
{
    return sInstance;
}

WindowManager::WindowManager()
{
}

WindowManager::~WindowManager()
{
}

void WindowManager::Initialize()
{
    // Nothing to initialize currently
}

void WindowManager::Shutdown()
{
    mWindowMap.clear();
}

void WindowManager::RegisterWindow(const std::string& id, Window* window)
{
    if (id.empty() || window == nullptr)
    {
        return;
    }

    auto it = mWindowMap.find(id);
    if (it != mWindowMap.end())
    {
        LogWarning("WindowManager: Window with id '%s' already registered. Overwriting.", id.c_str());
    }

    mWindowMap[id] = window;
}

void WindowManager::UnregisterWindow(const std::string& id)
{
    auto it = mWindowMap.find(id);
    if (it != mWindowMap.end())
    {
        mWindowMap.erase(it);
    }
}

void WindowManager::UnregisterWindow(Window* window)
{
    if (window == nullptr)
    {
        return;
    }

    for (auto it = mWindowMap.begin(); it != mWindowMap.end(); ++it)
    {
        if (it->second == window)
        {
            mWindowMap.erase(it);
            return;
        }
    }
}

Window* WindowManager::FindWindow(const std::string& id)
{
    auto it = mWindowMap.find(id);
    if (it != mWindowMap.end())
    {
        return it->second;
    }
    return nullptr;
}

bool WindowManager::HasWindow(const std::string& id) const
{
    return mWindowMap.find(id) != mWindowMap.end();
}

void WindowManager::ShowWindow(const std::string& id)
{
    Window* window = FindWindow(id);
    if (window != nullptr)
    {
        window->Show();
    }
}

void WindowManager::HideWindow(const std::string& id)
{
    Window* window = FindWindow(id);
    if (window != nullptr)
    {
        window->Hide();
    }
}

void WindowManager::CloseWindow(const std::string& id)
{
    Window* window = FindWindow(id);
    if (window != nullptr)
    {
        window->Close();
    }
}

void WindowManager::BringToFront(const std::string& id)
{
    Window* window = FindWindow(id);
    if (window != nullptr && window->GetParent() != nullptr)
    {
        Node* parent = window->GetParent();
        // Re-attaching to the same parent moves node to end of children list
        window->Attach(parent);
    }
}

const std::unordered_map<std::string, Window*>& WindowManager::GetAllWindows() const
{
    return mWindowMap;
}
