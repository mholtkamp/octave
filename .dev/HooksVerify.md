 Implementation Order

 1. Batch 1 - Menu positioning (user's explicit request, highest impact)
 2. Batch 2 - Create/Spawn menus (critical for node/asset extensibility)
 3. Batch 3 - Viewport context + overlays (major Unity parity gap)
 4. Batch 4 - Preferences pages (essential for addon settings)
 5. Batch 5 - Keyboard shortcuts (essential for UX parity)
 6. Batch 8 - Additional context menus (many small wins)
 7. Batch 7 - Hierarchy & asset browser extensions
 8. Batch 6 - Property drawers
 9. Batch 9 - Drag-drop & asset pipeline
 10. Batch 10 - Build pipeline & gizmo tools

 Approach for Each Batch

 For each new hook:
 1. Add callback typedef to EditorUIHooks.h (if new signature needed)
 2. Add function pointer to END of EditorUIHooks struct
 3. Add RegisteredXxx struct to EditorUIHookManager.h
 4. Add storage vector + Draw/Fire method to EditorUIHookManager class
 5. Implement registration function + draw/fire in EditorUIHookManager.cpp
 6. Wire the static function pointer in InitializeHooks()
 7. Add Draw*/Fire* calls at the right spot in EditorImgui.cpp (or other editor files)
 8. Add RemoveAllHooks cleanup for the new hook type

 Verification

 After each batch:
 - Build with EDITOR defined (editor build)
 - Build without EDITOR (game build) - ensure no editor code leaks
 - Write a test addon using the new hooks to confirm they work
 - Verify existing addons still work (binary compat - no function pointer reordering)
 - Check that RemoveAllHooks properly cleans up new hook types

 Documentation

 After all batches, update/create example .md files in Documentation/Development/NativeAddon/Examples/ for:
 - Custom top-level menu with position control
 - Extending the Add Node menu
 - Extending the Create Asset menu
 - Viewport context menu and overlays
 - Custom preferences panel
 - Custom keyboard shortcuts
 - Hierarchy item GUI overlay
 - Custom play targets
 - Custom property drawers
╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌