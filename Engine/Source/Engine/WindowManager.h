#pragma once

#include "EngineTypes.h"
#include <unordered_map>
#include <string>

class Window;

class WindowManager
{
public:

    static void Create();
    static void Destroy();
    static WindowManager* Get();

    void Initialize();
    void Shutdown();

    // Registration
    void RegisterWindow(const std::string& id, Window* window);
    void UnregisterWindow(const std::string& id);
    void UnregisterWindow(Window* window);

    // Lookup
    Window* FindWindow(const std::string& id);
    bool HasWindow(const std::string& id) const;

    // Control by ID
    void ShowWindow(const std::string& id);
    void HideWindow(const std::string& id);
    void CloseWindow(const std::string& id);
    void BringToFront(const std::string& id);

    // Query
    const std::unordered_map<std::string, Window*>& GetAllWindows() const;

private:

    static WindowManager* sInstance;

    WindowManager();
    ~WindowManager();

    std::unordered_map<std::string, Window*> mWindowMap;
};
