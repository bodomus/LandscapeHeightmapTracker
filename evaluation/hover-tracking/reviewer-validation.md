# Independent Architecture Review: Continuous Landscape Hover Tracking

## Overall Verdict

The first-stage investigation is directionally correct: passive hover tracking should be implemented through the editor mode's viewport mouse-move path, using the existing ray-building logic and panel-side landscape-to-heightmap mapping. The proposed architecture also correctly separates hover from reverse heightmap-to-landscape flow.

The plan needs several important corrections before implementation:

- `MouseMove` is the right primary hook, but it is not sufficient for all "continuous" cases if the camera moves while the mouse is stationary.
- A single cached mouse position is unsafe in a multi-viewport editor and can become stale when the viewport camera changes.
- Explicit Slate paint invalidation remains recommended, but the implementation should avoid exposing the private image-view class in the public panel header.
- Hover state must be strictly isolated from click and reverse state; hover must not mutate diagnostics or reverse marker data.
- Hover clearing must be driven by more than `MouseLeave`, because mode lifetime and viewport switching can leave stale state otherwise.

## Source Evidence

### Plugin Source

Current plugin behavior is click-driven:

- `FLandscapeHeightmapTrackerEdMode::InputKey` only reacts to left mouse `IE_Pressed` while tracking is enabled.
- `TraceLandscapeClick` uses `FEditorViewportClient::GetCursorWorldLocationFromMousePos`, `FViewportTraceRayBuilder::BuildTraceSegment`, and `World->LineTraceSingleByChannel(..., ECC_Visibility, ...)`, then broadcasts `OnViewportClickResult`.
- `Tick` only handles reverse marker cleanup.
- `Render` only draws the reverse marker.
- `SLandscapeHeightmapTrackerPanel::OnViewportClick` owns click-to-heightmap mapping and sets `bHasMarker` / `MarkerUV`.
- `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked` owns reverse flow and writes `LastMapping`, `LastLandscapeUV`, `bHasLandscapeUV`, and reverse marker globals.
- `SHeightmapTrackerImageView::OnPaint` reads marker attributes and draws the crosshair, but changing panel fields does not itself guarantee a repaint.

### Unreal Engine 5.7 Source

UE 5.7 supports the proposed hook:

- `FEdMode` exposes `MouseEnter`, `MouseLeave`, `MouseMove`, `CapturedMouseMove`, `ProcessCapturedMouseMoves`, `InputKey`, and `Tick`.
- `FLevelEditorViewportClient::MouseMove` forwards to `FEditorViewportClient::MouseMove`.
- `FEditorViewportClient::MouseMove` updates the current mouse position, calls `ModeTools->MouseMove(this, Viewport, x, y)`, and then caches last mouse coordinates.
- `FEditorModeTools::MouseMove` routes the event to interactive tools and all active legacy modes.
- `CapturedMouseMove` is explicitly capture-path behavior and is not the right primary hook for passive hover.
- `MouseLeave` is routed to active modes, but engine comments around `HoveredViewportClient` indicate leave/last-hover state has special behavior and should not be treated as the only reliable cleanup path.
- Slate `EInvalidateWidgetReason::Paint` exists for paint-only changes where layout is unchanged.

## Review Areas

### 1. MouseMove Hook Selection

Verdict: keep, with modifications.

`FEdMode::MouseMove` is the right primary hook for passive viewport hover. It receives normal viewport mouse movement without requiring a click or capture.

Implementation constraints:

- Override `MouseMove(FEditorViewportClient*, FViewport*, int32, int32)`.
- Call `FEdMode::MouseMove` to preserve base/tool behavior.
- Do not capture input.
- Do not rely on the returned `bool` to protect other modes; the editor viewport path ignores it for passive mouse movement.
- Gate all hover tracing behind `IsTrackingModeEnabled()`.

Rejected alternatives:

- `InputKey`: click-only; cannot deliver continuous hover.
- `CapturedMouseMove`: only applies while mouse capture is active.
- `Tick` as the only mechanism: would poll unnecessarily and would blur the event-driven behavior.

Important caveat:

`MouseMove` alone does not recompute the hit when the camera moves under a stationary cursor. If the feature promise includes that case, add a narrow `Tick` fallback that recomputes only for the last hovered viewport when tracking is enabled and the view state changed.

### 2. Multi-Viewport Correctness

Verdict: the original single-position dedupe is unsafe.

The active editor mode can receive events from multiple viewport clients. The same `(x, y)` coordinate in two different viewports can represent different rays and different landscape hits.

Do not use a single `LastHoverMousePosition` as a correctness gate.

Acceptable options:

- Prefer no EdMode-side coordinate dedupe for the first implementation.
- If dedupe is added later, key it by viewport identity plus mouse position plus view/camera state.

Panel-side visual dedupe is safer because it operates on the final displayed marker state.

### 3. Hover Clearing Semantics

Verdict: must be strengthened.

`MouseLeave` is useful but insufficient as the only cleanup mechanism.

Hover should clear when:

- tracking is disabled;
- the panel is destroyed;
- the assigned landscape changes or is cleared;
- the heightmap image is unloaded or replaced;
- the cursor leaves the viewport, when `MouseLeave` is delivered;
- a hover trace misses;
- a hover trace hits a non-assigned actor;
- mapping from world position to heightmap UV fails.

The mode may remain active because of a reverse marker even after tracking is disabled. Therefore the hover path must check `IsTrackingModeEnabled()` and clear or ignore hover state appropriately.

The largest stale-state edge cases are viewport switching without a useful leave event, actor/map deletion without another mouse event, and camera movement under a stationary cursor.

### 4. Neutral Trace Extraction

Verdict: keep, but simplify.

Extract the current tracing logic from `TraceLandscapeClick` into a neutral helper inside the editor mode. Both click and hover should use the same ray-building and visibility trace path.

The helper should return only what both callers need:

- whether a hit occurred;
- world position;
- weak hit actor;
- weak hit component.

Avoid adding production-facing failure reasons unless they are needed for logs or tests. Hover must not log at high frequency.

### 5. Delegate Architecture

Verdict: keep a separate hover delegate, simplify payload.

The existing module-level click delegate is already the plugin's boundary between editor mode input and the Slate panel. Hover fits that architecture, but it should not be multiplexed through the click delegate.

Reasons to keep hover separate:

- click is discrete and persistent;
- hover is transient and may include clear/no-hit events;
- hover must be removable without affecting click behavior;
- the panel can subscribe/unsubscribe symmetrically with the existing click delegate.

The hover payload should be minimal. Do not include UI-specific mapping data in the mode-level event.

### 6. Marker State Model

Verdict: separate hover and click marker state remains recommended.

Current `bHasMarker` / `MarkerUV` represents the last successful viewport click marker. Reusing it for hover would change click semantics: every miss, non-assigned hit, or mouse leave would erase the user's last click marker.

Recommended panel state:

- keep existing click marker state unchanged;
- add transient hover marker state;
- render hover marker preferentially while present;
- fall back to click marker when no hover marker exists.

This is a UX-preserving choice rather than an architectural requirement. It should be documented so users are not surprised when a previous click marker reappears after hover clears.

### 7. Reverse-Flow Isolation

Verdict: mandatory.

Hover must not write reverse-flow state.

Ownership table:

| State | Click may write | Hover may write | Reverse image click may write |
| --- | --- | --- | --- |
| `bHasMarker` | Yes | No | No |
| `MarkerUV` | Yes | No | No |
| `bHasHoverMarker` | Optional clear | Yes | Optional clear |
| `HoverMarkerUV` | No | Yes | No |
| `LastMapping` | Yes | No | Yes |
| `LastLandscapeUV` | Clears/updates existing semantics | No | Yes |
| `bHasLandscapeUV` | Existing click behavior | No | Yes |
| `StatusText` | Yes | No by default | Yes |
| reverse marker globals | No | No | Yes |
| `AssignedLandscape` | User action only | No | No |

Hover should map world position to UV using local temporary variables and then discard the full mapping result.

### 8. Performance

Verdict: no wall-clock throttling initially.

The highest-cost operation is the viewport trace. The mapping and delegate broadcast are comparatively small.

Recommended first implementation:

- trace on `MouseMove` only while tracking is enabled;
- no timers;
- no fixed millisecond throttle;
- no global editor delegates;
- panel invalidates only when displayed marker state changes.

Avoid single-coordinate dedupe in the editor mode. If performance later requires it, use a viewport-aware key and include view-state changes.

### 9. Slate Invalidation

Verdict: explicit paint invalidation remains recommended.

`SHeightmapTrackerImageView` reads bound attributes in `OnPaint`, but changing the captured panel fields does not by itself guarantee the widget will repaint immediately.

Use `EInvalidateWidgetReason::Paint` when the visible marker state changes.

Important implementation correction:

`SHeightmapTrackerImageView` is a private class in the `.cpp` file. Do not add `TSharedPtr<SHeightmapTrackerImageView>` to the public panel header unless the class is moved out of the anonymous implementation scope. Prefer one of these:

- call `Invalidate(EInvalidateWidgetReason::Paint)` on `SLandscapeHeightmapTrackerPanel`;
- or store the image widget as `TSharedPtr<SWidget>` and invalidate that.

### 10. Lifecycle and Ownership

Verdict: lifecycle must be explicit but should stay small.

The panel owns subscriptions. The module owns global delegates. The editor mode owns viewport tracing.

Required cleanup:

- remove hover delegate handle in panel destructor;
- clear hover state in panel destructor;
- clear hover state when tracking is disabled;
- clear hover state when assigning or clearing landscape;
- clear hover state when loading/unloading heightmap image;
- clear all hover delegates in module shutdown.

Do not add broad map-change or actor-deletion listeners in the first implementation unless testing proves they are needed.

### 11. Implementation Scope

Must change:

- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`
- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`
- `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`
- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Should not change for the first implementation:

- `LandscapeHeightmapTracker.Build.cs`
- `ViewportTraceRayBuilder`
- `LandscapeCoordinateMapper`
- `LandscapeSurfaceTraceHelper`
- `HeightmapImageClickMapper`

The current dependencies are sufficient for the planned changes.

### 12. What to Remove from the First Plan

Remove or defer:

- single `LastHoverMousePosition` dedupe in the editor mode;
- hover `FailureReason` in production delegate payload;
- concrete `SHeightmapTrackerImageView` pointer in the public header;
- hover updates to status text or diagnostics;
- timers;
- global map/actor lifecycle hooks;
- changes to reverse mapping helpers;
- any attempt to use `CapturedMouseMove` as the primary hook.

## Mandatory Adversarial Questions

### What is the strongest reason this architecture is wrong?

The strongest objection is that `MouseMove` alone is not truly continuous if the camera moves while the cursor remains stationary. A world-space hover marker can become stale without another mouse event. A single mouse-position cache makes that problem worse and also breaks multi-viewport correctness.

### What is the strongest reason it is correct?

UE 5.7 source confirms that passive viewport movement is routed through `FEditorViewportClient::MouseMove` into `FEditorModeTools::MouseMove` and active `FEdMode::MouseMove` implementations. The plugin already uses an editor mode plus module multicast delegate plus Slate panel mapping, so hover can be added without changing the plugin's ownership model.

### Is `MouseMove` sufficient when the camera moves but the mouse is stationary?

No, not if the expected behavior is "marker follows the landscape under the cursor during camera motion." Add a minimal view-change recompute path in `Tick`, or explicitly define that the MVP updates only on mouse movement.

### Is a single cached mouse position correct with multiple editor viewports?

No. It must be removed or keyed by viewport and view state.

### Is separate hover state genuinely necessary?

It is not strictly necessary for rendering a crosshair, but it is the cleanest way to preserve existing click behavior.

### Is a separate hover delegate genuinely necessary?

Not strictly, but it is recommended. Hover has transient clear semantics that do not belong in the click delegate.

### Is explicit Slate invalidation definitely required?

It is required for reliable immediate UI updates. Other editor activity may incidentally repaint the widget, but relying on that would make hover feel inconsistent.

### What edge case is most likely missing?

Stationary cursor plus camera movement is the most important behavioral edge case. Stale hover after viewport switch, tracking disable, or assigned landscape deletion are the next most likely.

### What must the implementation agent not do?

Do not mutate reverse state from hover. Do not use a single mouse-position cache. Do not update diagnostics on every hover. Do not rely on `CapturedMouseMove`. Do not add broad lifecycle hooks before the narrow event-driven implementation has been tested.
