#pragma once

#include <string>

class Node;

std::string FindRelativeNodePath(Node* src, Node* dst);
void AddPendingNodePath(WeakPtr<Object> owner, Datum* datum, std::string path);

// NODE: Resolve pending paths after the last scene has instantiated.
// Use a count / stack to keep track of how many nested scenes are being instantiated.
void ResolvePendingNodePaths();
