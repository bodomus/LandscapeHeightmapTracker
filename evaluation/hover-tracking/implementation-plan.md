# Continuous Landscape Hover Tracking Implementation Plan

This is a planning artifact only. Do not implement from this file in the investigation task.

## Architecture Decision Summary

Recommended viewport integration mechanism: override `FLandscapeHeightmapTrackerEdMode::MouseMove`.

Recommended trace strategy: Option B, extract common viewport trace mechanics into a neutral helper in `FLandscapeHeightmapTrackerEdMode`, then keep click and hover entry points separate.

Recommended state model: separate hover marker state from existing click marker state. Hover marker should visually take precedence while valid; persistent click marker should remain stored and reappear when hover clears.

Recommended throttling: no timer and no initial rate limiting. Use deduplication only: skip identical viewport mouse positions and skip unchanged marker UV/pixel paint invalidation.

Main lifecycle principle: EdMode owns viewport input; module owns multicast delegates; panel owns assigned Landscape, marker state, mapping, and Slate invalidation.

## Event and Delegate Flow

```text
Mouse move in active level editor viewport
  -> FLandscapeHeightmapTrackerEdMode::MouseMove
  -> TraceLandscapeUnderCursor
  -> OnViewportHoverResult.Broadcast
  -> SLandscapeHeightmapTrackerPanel::OnViewportHover
  -> IsAssignedLandscapeHit
  -> FLandscapeCoordinateMapper::MapWorldPosition
  -> bHasHoverMarker / HoverMarkerUV
  -> active marker Slate attributes
  -> SHeightmapTrackerImageView::OnPaint
```

Click flow remains:

```text
InputKey(left press)
  -> TraceLandscapeClick
  -> TraceLandscapeUnderCursor
  -> OnViewportClickResult.Broadcast
  -> SLandscapeHeightmapTrackerPanel::OnViewportClick
```

Reverse flow remains:

```text
SHeightmapTrackerImageView::OnMouseButtonDown
  -> OnHeightmapClicked
  -> MapUVToLocalPosition
  -> TraceAssignedLandscapeSurface
  -> SetReverseMarker
```

## Ordered Implementation Steps

### 1. Add Hover Delegate API

Target file: `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`

Target class: `FLandscapeHeightmapTrackerModule`

Target symbols:

- Existing `FViewportClickResult`
- New `FViewportHoverResult`
- New `DECLARE_MULTICAST_DELEGATE_OneParam(FOnViewportHoverResult, const FViewportHoverResult&)`
- New `static FOnViewportHoverResult& OnViewportHoverResult()`

Responsibility:

- Keep hover event separate from click event.
- Include enough state for both hit and clear:
  - `bool bHasHit`
  - `FVector WorldPosition`
  - `TWeakObjectPtr<AActor> HitActor`
  - `TWeakObjectPtr<UPrimitiveComponent> HitComponent`
  - optional `FIntPoint MousePosition`

State changes: none yet.

Delegate changes: add hover delegate accessor.

Lifecycle implications: delegate must be cleared during module shutdown.

Invalid-state behavior: `bHasHit=false` means clear hover marker.

Verification:

- Build compiles.
- Existing click delegate users unchanged.

### 2. Store and Clear Hover Delegate

Target file: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`

Target class: `FLandscapeHeightmapTrackerModule`

Target symbols:

- New static `GOnViewportHoverResult`
- New `OnViewportHoverResult()`
- `ShutdownModule()`

Responsibility:

- Store module-level hover multicast delegate.
- Clear it in `ShutdownModule()` alongside `GOnViewportClickResult.Clear()`.

State changes: no persistent hover state in module.

Delegate changes: `GOnViewportHoverResult.Clear()`.

Lifecycle implications: prevents callbacks into stale panels during module unload.

Error behavior: none.

Verification:

- Confirm shutdown clears both click and hover delegates.

### 3. Extract Neutral Viewport Trace Helper

Target file: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`

Target class: `FLandscapeHeightmapTrackerEdMode`

Target symbols:

- New private trace result struct.
- New private `TraceLandscapeUnderCursor(FEditorViewportClient* ViewportClient, FViewport* Viewport) const`.
- Optional private `TOptional<FIntPoint> LastHoverMousePosition` if deduplication is stored in EdMode.

Responsibility:

- Separate trace mechanics from click semantics.

State changes:

- Optional last mouse position cache for hover deduplication.

Delegate changes: none.

Lifecycle implications:

- Cache should reset on `MouseLeave` and possibly when trace fails due invalid viewport/world.

Invalid-state behavior:

- Result should carry `bHasHit=false` and failure reason when ViewportClient/Viewport/GEditor/World/trace segment/hit is invalid.

Verification:

- Existing click flow still calls through the helper.

### 4. Implement Neutral Trace and Rewire Click Path

Target file: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`

Target class: `FLandscapeHeightmapTrackerEdMode`

Target symbols:

- Existing `TraceLandscapeClick`
- New `TraceLandscapeUnderCursor`

Responsibility:

- Move shared logic from `TraceLandscapeClick` into `TraceLandscapeUnderCursor`:
  - viewport/editor/world validation;
  - `GetCursorWorldLocationFromMousePos`;
  - `FViewportTraceRayBuilder::BuildTraceSegment`;
  - `LineTraceSingleByChannel`;
  - result payload fill.
- Keep click-specific logging/stat text if useful, but do not duplicate mechanics.
- `TraceLandscapeClick` broadcasts `OnViewportClickResult` only when neutral trace hits.

State changes: none.

Delegate changes: click broadcast unchanged in semantics.

Lifecycle implications: none beyond current mode lifecycle.

Invalid-state behavior:

- Click miss continues to return false.

Verification:

- Manual click flow still updates marker.
- Existing trace ray builder tests remain valid.

### 5. Add MouseMove and MouseLeave Hooks

Target file: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`

Target class: `FLandscapeHeightmapTrackerEdMode`

Target symbols:

- `virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;`
- `virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;`

Responsibility:

- Receive passive cursor movement only while the mode is active.

State changes:

- Track last hover mouse position for deduplication.

Delegate changes: none in header beyond using module hover delegate in implementation.

Lifecycle implications:

- `MouseLeave` clears hover and resets dedupe.

Invalid-state behavior:

- Invalid viewport/client should broadcast hover clear once if needed, then return false.

Verification:

- Moving mouse over viewport triggers hover path.
- Returning false preserves editor viewport behavior.

### 6. Broadcast Hover Result From EdMode

Target file: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`

Target class: `FLandscapeHeightmapTrackerEdMode`

Target functions:

- `MouseMove`
- `MouseLeave`

Responsibility:

- In `MouseMove`, first call `FEdMode::MouseMove(...)` or decide whether to call it before/after local handling; return false unless base/current tool handled and existing behavior needs propagation.
- Check `FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled()`.
- Deduplicate identical `(x, y)` per viewport movement.
- Call `TraceLandscapeUnderCursor`.
- Broadcast `FViewportHoverResult` with hit data or `bHasHit=false`.
- In `MouseLeave`, broadcast `bHasHit=false` and reset dedupe.

State changes:

- Last hover mouse position.

Delegate changes:

- New hover broadcast.

Lifecycle implications:

- Runs only while mode active; inactive mode receives no movement.

Invalid-state behavior:

- On no hit, wrong world, invalid trace segment, or no tracking, clear hover.

Verification:

- Hover over Landscape sends hit.
- Hover over non-Landscape sends clear/no-hit.
- Leaving viewport sends clear.

### 7. Add Panel Hover State and Delegate Binding

Target file: `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`

Target class: `SLandscapeHeightmapTrackerPanel`

Target symbols:

- `void OnViewportHover(const FLandscapeHeightmapTrackerModule::FViewportHoverResult& Hover);`
- `void ClearHoverMarker();`
- `FVector2D HoverMarkerUV`
- `bool bHasHoverMarker`
- `FDelegateHandle HoverDelegateHandle`
- optional `TSharedPtr<SHeightmapTrackerImageView> HeightmapImageView`

Responsibility:

- Store hover marker independent from click marker.
- Track delegate handle for safe unbinding.
- Store image widget pointer if paint invalidation is targeted.

State changes:

- Add hover marker state.

Delegate changes:

- Add hover binding handle.

Lifecycle implications:

- Destructor must remove hover delegate.

Invalid-state behavior:

- `ClearHoverMarker()` clears hover only and invalidates paint if the rendered marker changes.

Verification:

- Header exposes no new public API beyond private implementation details.

### 8. Bind Hover Delegate and Update Marker Attributes

Target file: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Target class: `SLandscapeHeightmapTrackerPanel`

Target functions/symbols:

- `Construct`
- destructor
- marker lambdas in `SNew(SHeightmapTrackerImageView)`

Responsibility:

- Bind `OnViewportHoverResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportHover)`.
- Remove hover delegate in destructor.
- Change marker lambdas to expose active display marker:
  - `HasMarker`: `bHasHoverMarker || bHasMarker`
  - `MarkerUV`: `bHasHoverMarker ? HoverMarkerUV : MarkerUV`
- If storing image widget: assign with `SAssignNew(HeightmapImageView, SHeightmapTrackerImageView)`.

State changes:

- None beyond binding.

Delegate changes:

- AddSP/Remove for hover.

Lifecycle implications:

- Panel destruction stops callback.

Invalid-state behavior:

- If image widget pointer is invalid during clear/update, simply skip targeted invalidation.

Verification:

- Existing click marker still appears.
- Hover marker appears when `bHasHoverMarker` is true.

### 9. Implement Panel Hover Mapping

Target file: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Target class: `SLandscapeHeightmapTrackerPanel`

Target functions:

- New `OnViewportHover`
- New `ClearHoverMarker`

Responsibility:

- If tracking disabled, hover result has no hit, AssignedLandscape invalid, or hit is not assigned Landscape: clear hover marker.
- On valid assigned Landscape hit:
  - `RefreshLandscapeBounds()`;
  - build `FLandscapeTrackerMappingOptions` from `bFlipX/bFlipY`;
  - call `FLandscapeCoordinateMapper::MapWorldPosition`;
  - if valid, update hover marker state.
- Do not call `SetReverseMarker`, `ClearReverseMarker`, or `OnHeightmapClicked`.
- Avoid status text churn on every hover. If status is updated for hover, dedupe messages carefully.

State changes:

- `bHasHoverMarker`
- `HoverMarkerUV`
- optional `LastHoverMapping`

Delegate changes: none.

Lifecycle implications:

- Uses `TWeakObjectPtr<ALandscapeProxy>` validity checks.

Invalid-state behavior:

- Clear hover marker, preserve click marker and reverse marker.

Verification:

- Hover over assigned Landscape moves image marker.
- Hover over other actors clears hover and falls back to click marker.

### 10. Add Paint-Only Invalidation

Target file: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Target class: `SLandscapeHeightmapTrackerPanel`

Target symbols:

- `ClearHoverMarker`
- `OnViewportHover`
- possibly `OnViewportClick`, `ClearMarker`, `AssignLandscape`, `LoadPngTexture`

Responsibility:

- Call `Invalidate(EInvalidateWidgetReason::Paint)` on the image widget or panel when displayed marker state changes.
- Do not use layout invalidation.
- Invalidate only on transitions:
  - hover inactive -> active;
  - hover active -> inactive;
  - hover UV/pixel changed beyond epsilon;
  - click marker changed when hover inactive.

State changes:

- Optional `LastRenderedMarkerUV` or helper that returns active marker state for comparison.

Delegate changes: none.

Lifecycle implications:

- Targeted invalidation requires stored widget pointer to remain panel-owned.

Invalid-state behavior:

- If no widget pointer, fall back to panel `Invalidate(EInvalidateWidgetReason::Paint)`.

Verification:

- Hover marker visibly updates without resizing/reflowing UI.

### 11. Preserve Existing User Actions

Target file: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Target class: `SLandscapeHeightmapTrackerPanel`

Target functions:

- `OnViewportClick`
- `OnHeightmapClicked`
- `ClearMarker`
- `AssignLandscape`
- `LoadPngTexture`
- `SetTrackingEnabled`

Responsibility:

- `OnViewportClick`: keep updating persistent click marker. Do not clear hover unless UX decides that click should pin marker and hover should continue immediately after.
- `OnHeightmapClicked`: keep reverse flow independent.
- `ClearMarker`: clear click, hover, diagnostics, and reverse marker.
- `AssignLandscape`: clear click, hover, diagnostics, and reverse marker.
- `LoadPngTexture`: clear click and hover marker because image coordinate target changed.
- `SetTrackingEnabled(false)`: clear hover marker.

State changes:

- Existing and new marker fields.

Delegate changes: none.

Lifecycle implications:

- Tracking disabled should remove hover visualization even if mode deactivation follows.

Invalid-state behavior:

- Assigned Landscape invalid should not dereference; clear hover/click as appropriate.

Verification:

- Existing click and reverse flows still pass manual checks.

## State Model

Persistent click marker:

- `bHasMarker`
- `MarkerUV`
- `LastMapping` for diagnostics

Transient hover marker:

- `bHasHoverMarker`
- `HoverMarkerUV`
- optional `LastHoverMapping`

Displayed image marker:

```text
HasDisplayedMarker = bHasHoverMarker || bHasMarker
DisplayedMarkerUV = bHasHoverMarker ? HoverMarkerUV : MarkerUV
```

Reverse marker:

- Existing module state remains independent:
  - `GHasReverseMarker`
  - `GReverseMarkerWorldPosition`
  - `GReverseMarkerOwner`

## Lifecycle Safeguards

- Route hover through module multicast delegates, not raw EdMode-to-panel pointers.
- Use `AddSP` and remove `HoverDelegateHandle` in panel destructor.
- Clear hover delegate in module shutdown.
- Use weak object checks for assigned Landscape and hit actor/component.
- Return false from `MouseMove` to avoid consuming viewport input.
- Clear hover on `MouseLeave`, tracking disable, panel destruction, assigned Landscape change, and heightmap reload.
- Do not modify reverse marker lifetime from hover path.

## Performance Safeguards

- Event-driven `MouseMove`, no polling timer.
- Deduplicate identical mouse coordinates in EdMode.
- Deduplicate unchanged displayed UV/pixel in panel before invalidation.
- Use paint-only invalidation.
- Avoid status text updates per hover frame.
- Avoid `RedrawAllViewports` for image marker updates; that is for reverse viewport marker rendering.

## Verification Strategy

Manual/editor integration scenarios:

1. Move mouse across assigned Landscape; hover marker appears and follows cursor.
2. Marker moves continuously without clicking.
3. Hover marker coordinates match existing click result when clicking the same point.
4. Move over non-assigned actor or empty world; hover marker clears or falls back to click marker.
5. Move outside viewport; hover marker clears.
6. Existing click flow still creates persistent click marker.
7. Existing heightmap-to-Landscape reverse flow still sets viewport reverse marker.
8. Reverse marker remains visually correct while hover marker updates in image widget.
9. Tracking checkbox activation/deactivation activates/deactivates EdMode and clears hover.
10. Panel close/reopen leaves no stale callbacks and restores settings.
11. Landscape selection/change clears stale markers and maps against the new assigned Landscape.
12. Landscape deletion or invalid weak pointer clears hover and does not crash.
13. Rapid mouse movement remains responsive.
14. Stationary cursor does not retrace/invalidate repeatedly after dedupe.
15. Editor viewport camera movement, selection, gizmo interaction, and normal input remain unaffected.

Automated tests that are realistic:

- Existing `ViewportTraceRayBuilder` tests should remain and can be extended only if neutral trace extraction adds pure helper behavior.
- Existing `LandscapeCoordinateMapper` tests should remain unchanged; add tests only if active marker selection is factored into a pure helper.
- Unit tests for hover state selection can be added if marker state logic is extracted into a small pure function or lightweight struct.

Manual-only/editor integration:

- `FEdMode::MouseMove` routing.
- Actual viewport `LineTraceSingleByChannel`.
- Slate repaint behavior in the dock tab.
- Panel close/reopen with live editor mode.
- Landscape deletion/map changes.

## Unresolved Questions

- Whether hover should update diagnostic text fields (`World`, `Landscape`, `UV`, `Pixel`) live or leave diagnostics tied to clicks/reverse clicks. Recommendation: avoid live diagnostic churn initially unless explicitly requested.
- Whether the UI label "Track Landscape Clicks" should become "Track Landscape Cursor" or gain a separate hover toggle. Recommendation: keep one tracking toggle for the first implementation unless product UX asks for separate click/hover controls.
