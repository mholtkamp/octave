#pragma once

/**
 * @file AddonsMenu.h
 * @brief Shared ImGui menu drawing and dialog rendering for the Addons menu system.
 *
 * Provides the unified "Addons" menu accessible from the viewport bar popup,
 * the Addons window menu bar, and the Project Select window.
 */

#if EDITOR

/**
 * @brief Draw the full Addons menu tree for the viewport bar popup.
 *
 * Contains: Create >, Initialize >, Publish >, Edit package.json
 */
void DrawAddonsPopupContent();

/**
 * @brief Draw the same Addons menu structure for AddonsWindow's menu bar.
 */
void DrawAddonsMenuBar();

/**
 * @brief Draw a subset of Create items for ProjectSelectWindow.
 */
void DrawAddonsCreateItems_ProjectSelect();

/**
 * @brief Render all addon-related modal dialogs.
 *
 * Must be called once per frame from EditorImguiDraw() outside of popups.
 */
void DrawAddonsDialogs();

/** @brief Opens the Create Native Addon dialog (resets state). */
void OpenCreateNativeAddonDialog();

#endif // EDITOR
