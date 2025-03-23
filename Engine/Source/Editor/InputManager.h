#pragma once

class InputManager
{
public:

    ~InputManager();

    static void Create();
    static void Destroy();
    static InputManager* Get();

    void Update();

protected:

    static InputManager* sInstance;
    InputManager();

    void UpdateHotkeys();
};
