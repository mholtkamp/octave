#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

#include <stdint.h>
#include "Maths.h"

void EditorShowCursor(bool show);
void EditorCenterCursor();
void EditorSetCursorPos(int32_t x, int32_t y);
void EditorGetWindowCenter(int32_t& x, int32_t& y);
glm::vec3 EditorGetFocusPosition();
