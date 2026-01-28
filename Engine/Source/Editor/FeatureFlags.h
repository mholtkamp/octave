#pragma once

#if EDITOR

#include "EngineTypes.h"

struct FeatureFlagsEditor
{

    bool mShowTheming = false;
    bool mShowPreferences = false;
    bool mShow2DBorder = false;
};

extern FeatureFlagsEditor gFeatureFlagsEditor;

// Make a Static instance of FeatureFlagsEditor for the editor
FeatureFlagsEditor& GetFeatureFlagsEditor();




#endif
