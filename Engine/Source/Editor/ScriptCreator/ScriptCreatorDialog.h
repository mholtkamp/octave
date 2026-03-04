#pragma once

#if EDITOR

/**
 * @file ScriptCreatorDialog.h
 * @brief Dialogs for creating new Lua scripts and C++ source file pairs.
 */

/** Opens the Create Script dialog (resets state). */
void OpenCreateScriptDialog();

/** Opens the Create C++ File dialog (resets state). */
void OpenCreateCppFileDialog();

/** Renders all script creator modal dialogs. Call once per frame from EditorImguiDraw(). */
void DrawScriptCreatorDialogs();

#endif // EDITOR
