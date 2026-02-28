#include "FeatureFlags.h"
#include <cstdlib>
#include <cstring>

static FeatureFlagsEditor InitializeFeatureFlags()
{
	FeatureFlagsEditor flags;

	// Check for OCTAVE_EXTRAFEATURES environment variable
	const char* extraFeatures = std::getenv("OCTAVE_EXTRAFEATURES");
	if (extraFeatures != nullptr && std::strcmp(extraFeatures, "1") == 0)
	{
		// Enable all extra features
		flags.mShowTheming = true;
		flags.mShowPreferences = true;
		flags.mShow2DBorder = true;
	}

	return flags;
}

FeatureFlagsEditor& GetFeatureFlagsEditor()
{
	static FeatureFlagsEditor sFeatureFlagsEditor = InitializeFeatureFlags();
	return sFeatureFlagsEditor;
}
