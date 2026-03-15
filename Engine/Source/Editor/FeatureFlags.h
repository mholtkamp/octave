#pragma once

#if EDITOR

#include "EngineTypes.h"

struct FeatureFlagsEditor
{

    bool mShowTheming = true;
    bool mShowPreferences = true;
    bool mShow2DBorder = true;
};

extern FeatureFlagsEditor gFeatureFlagsEditor;

// Make a Static instance of FeatureFlagsEditor for the editor
FeatureFlagsEditor& GetFeatureFlagsEditor();




#endif
