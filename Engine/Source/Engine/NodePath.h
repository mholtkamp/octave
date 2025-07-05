#pragma once

#include <string>
#include <vector>
#include "Datum.h"
#include "SmartPointer.h"

class Object;
class Node;
class Datum;

struct PendingNodePath
{
    WeakPtr<Node> mNode;
    std::string mPropName;
    Datum mPath;
};

std::string FindRelativeNodePath(Node* src, Node* dst);
void ResolveNodePaths(Node* node, bool recurseChildren);
Node* ResolveNodePath(Node* src, const std::string& path);
void ResolvePendingNodePaths(std::vector<PendingNodePath>& pending);
