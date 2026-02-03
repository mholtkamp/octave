#include "FeatureFlags.h"

FeatureFlagsEditor& GetFeatureFlagsEditor()
{
	static FeatureFlagsEditor sFeatureFlagsEditor;
	return sFeatureFlagsEditor;
}
