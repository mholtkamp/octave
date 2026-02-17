 1. Phase 0: Open old scene -> nodes get UUIDs auto-assigned + logged -> save -> reopen -> UUIDs stable -> World::FindNodeByUuid() returns correct node
 2. Phase 1: Create Timeline asset in code -> add tracks/clips -> save -> load -> tracks/clips restored with correct types
 3. Phase 2: Programmatically evaluate each track type -> verify transforms animate, audio plays, animation plays on skeletal mesh, script properties change, activate  
 toggles
 4. Phase 3: Spawn TimelinePlayer in editor -> assign Timeline asset -> Play in Editor -> timeline runs -> Stop -> state restored. Also test Lua scripting:
 self.timeline:Play(), :GetTime(), :Stop() all work from Lua scripts.
 5. Phase 4: Full editor workflow:
   - Right-click in asset browser -> Create > Timeline -> new .oct asset created
   - Double-click timeline asset -> Timeline panel opens (dockable/floatable window)
   - Add tracks, bind to scene nodes
   - Add/move/trim/split clips, scrub playhead -> scene updates live in viewport (no Play mode needed)
   - Stop preview -> node states restored to pre-preview
   - Close panel (X button), reopen via View > Timeline
   - Dock at bottom, undock to float, resize freely
   - Undo/redo all operations
 6. Phase 5: Create custom track in native addon -> register -> appears in editor -> evaluates at runtime

  Verification

 1. Right-click track area > Add Clip > verify clip type matches track type
 2. Click track target button > node picker popup > select node > track shows name, UUID stored
 3. Select TransformClip in inspector > see per-keyframe Time/Pos/Rot/Scale/Interp
 4. Right-click TransformClip > "Add Keyframe at Playhead" > keyframe appears
 5. Edit keyframe values > scrub playhead > node animates in viewport
 6. AnimationClip inspector shows Animation Name, Blend In/Out, Weight (unchanged)
 7. ActivateClip inspector shows Set Active, Set Visible (unchanged)
╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌