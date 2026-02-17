# Spline3D

A 3D spline path node that can be sampled, played along, and linked to other splines.

Inheritance:
* [Node](../Node.md)
* [Node3D](Node3D.md)

---

### AddPoint
Add a control point to the spline.

Sig: `Spline3D:AddPoint(point)`
 - Arg: `Vector point` Point in local space
---
### ClearPoints
Clear all control points from the spline.

Sig: `Spline3D:ClearPoints()`
---
### GetPointCount
Get number of control points.

Sig: `count = Spline3D:GetPointCount()`
 - Ret: `integer count` Number of points
---
### GetPoint
Get a control point by index.

Sig: `point = Spline3D:GetPoint(index)`
 - Arg: `integer index` 1-based point index
 - Ret: `Vector point` Point in local space
---
### SetPoint
Set a control point by index.

Sig: `Spline3D:SetPoint(index, point)`
 - Arg: `integer index` 1-based point index
 - Arg: `Vector point` Point in local space
---
### GetPositionAt
Sample spline position.

Sig: `position = Spline3D:GetPositionAt(t)`
 - Arg: `number t` Parametric value along spline
 - Ret: `Vector position` Position in local space
---
### GetTangentAt
Sample spline tangent direction.

Sig: `tangent = Spline3D:GetTangentAt(t)`
 - Arg: `number t` Parametric value along spline
 - Ret: `Vector tangent` Tangent in local space
---
### Play
Start spline playback/follow behavior.

Sig: `Spline3D:Play()`
---
### Stop
Stop spline playback/follow behavior.

Sig: `Spline3D:Stop()`
---
### SetPaused
Pause or unpause active spline movement.

Sig: `Spline3D:SetPaused(paused)`
 - Arg: `boolean paused` True to pause, false to resume
---
### IsPaused
Check pause state.

Sig: `paused = Spline3D:IsPaused()`
 - Ret: `boolean paused` Current pause state
---
### SetFollowLinkEnabled
Enable/disable automatic follow for a specific link slot.

Sig: `Spline3D:SetFollowLinkEnabled(index, enabled)`
 - Arg: `integer index` Link slot index in range `[1, 64]`
 - Arg: `boolean enabled` True to auto-follow this link
---
### IsFollowLinkEnabled
Check whether a link slot is set to auto-follow.

Sig: `enabled = Spline3D:IsFollowLinkEnabled(index)`
 - Arg: `integer index` Link slot index in range `[1, 64]`
 - Ret: `boolean enabled` True if link auto-follow is enabled
---
### IsNearLinkFrom
Check if current spline playback is near the "from" endpoint for a link.

Sig: `near = Spline3D:IsNearLinkFrom(index, epsilon=0.05)`
 - Arg: `integer index` Link slot index in range `[1, 64]`
 - Arg: `number epsilon` Optional proximity threshold
 - Ret: `boolean near` True if near the link-from endpoint
---
### IsNearLinkTo
Check if current spline playback is near the "to" endpoint for a link.

Sig: `near = Spline3D:IsNearLinkTo(index, epsilon=0.05)`
 - Arg: `integer index` Link slot index in range `[1, 64]`
 - Arg: `number epsilon` Optional proximity threshold
 - Ret: `boolean near` True if near the link-to endpoint
---
### IsLinkDirectionForward
Check whether travel direction aligns with a link's forward direction.

Sig: `forward = Spline3D:IsLinkDirectionForward(index, threshold=0.0)`
 - Arg: `integer index` Link slot index in range `[1, 64]`
 - Arg: `number threshold` Optional direction threshold
 - Ret: `boolean forward` True if direction is considered forward
---
### TriggerLink
Force an immediate handoff on a specific link slot.

Sig: `ok = Spline3D:TriggerLink(index)`
 - Arg: `integer index` Link slot index in range `[1, 64]`
 - Ret: `boolean ok` True if link handoff was accepted
---
### CancelActiveLink
Cancel any active link transition in progress.

Sig: `Spline3D:CancelActiveLink()`
---
