#pragma once

#include "Maths.h"

void InitializeGrid();
void EnableGrid(bool enable);
bool IsGridEnabled();
void ToggleGrid();
void SetGridColor(const glm::vec4& color);
glm::vec4 GetGridColor();