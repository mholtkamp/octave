#pragma once

#include <string>

class Node;

std::string FindRelativeNodePath(Node* src, Node* dst);
void AddPendingNodePath(WeakPtr<Object> owner, Datum* datum, std::string path);

// NOTE: Resolve pending paths after the last scene has instantiated.
// Use a count / stack to keep track of how many nested scenes are being instantiated.
// But does this work for async loading? Maybe a separate set of pending node paths
// that is merged with the main ones?
// ACTUALLY: All pending nodepaths need to be stored on the Scene I think
// And then after a Scene is intantiated then we need to resolve 
// FOR REAL ACTUALLY: Just store the node path in the Property mMisc string.
void ResolvePendingNodePaths();
