#pragma once

#if EDITOR

#include <functional>
#include <future>
#include <string>

struct ControllerCommand
{
    std::function<std::string()> mFunction;
    std::promise<std::string> mPromise;
};

#endif
