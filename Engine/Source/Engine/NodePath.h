#pragma once

#include <string>
#include "SmartPointer.h"

class Object;
class Node;
class Datum;

std::string FindRelativeNodePath(Node* src, Node* dst);
void ResolveNodePaths(Node* node, bool recurseChildren);
Node* ResolveNodePath(Node* src, const std::string& path);
