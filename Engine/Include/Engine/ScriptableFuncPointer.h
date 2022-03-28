#pragma once

#include "ObjectRef.h"

class ScriptComponent;

template<typename T>
struct ScriptableFP
{
    T mFuncPointer = nullptr;
    mutable std::string mScriptTableName;
    mutable std::string mScriptFuncName;
};
