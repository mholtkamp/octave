#include "FeatureFlags.h"

#if EDITOR


FeatureFlagsEditor& GetFeatureFlagsEditor()
{
	static FeatureFlagsEditor sFeatureFlagsEditor;
	return sFeatureFlagsEditor;
}

#endif