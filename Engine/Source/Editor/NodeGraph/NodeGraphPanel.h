#pragma once

#if EDITOR

class NodeGraphAsset;
class NodeGraph;
class Asset;

void DrawNodeGraphContent();
void OpenNodeGraphForEditing(NodeGraphAsset* asset);
void OpenNodeGraphForEditing(NodeGraph* graph, Asset* owner);
void CloseNodeGraphPanel();

#endif
