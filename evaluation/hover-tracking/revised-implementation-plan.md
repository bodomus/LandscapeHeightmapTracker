# Revised Implementation Plan: Continuous Landscape Hover Tracking

## Goal

Add continuous viewport-hover preview for the currently assigned landscape and loaded heightmap without changing the existing click marker or reverse heightmap-click behavior.

This plan supersedes the first-stage implementation plan where it conflicts with reviewer findings.

## Non-Goals

- Do not implement global editor lifecycle hooks in the first pass.
- Do not add timers or fixed millisecond throttling.
- Do not change coordinate-mapping helpers.
- Do not change reverse marker behavior.
- Do not update status text or diagnostics on every hover movement.
- Do not commit as part of implementation unless explicitly requested.

## Behavioral Contract

When tracking is enabled and the cursor moves over an editor viewport:

1. The editor mode traces under the cursor.
2. The panel receives a transient hover result.
3. If the hit belongs to the assigned landscape and maps to the loaded heightmap, the image view renders a hover marker.
4. If hover is cleared, the image view falls back to the last click marker, if one exists.
5. Reverse heightmap-click markers remain independent.

If camera movement under a stationary cursor must be supported, the implementation adds the narrow Tick fallback described below.

## Phase 1: Module-Level Hover Event

Files:

- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`

Add a minimal hover result type:

```cpp
struct FViewportHoverResult
{
    bool bHasHit = false;
    FVector WorldPosition = FVector::ZeroVector;
    TWeakObjectPtr<AActor> HitActor;
    TWeakObjectPtr<UPrimitiveComponent> HitComponent;
};
```

Add:

```cpp
DECLARE_MULTICAST_DELEGATE_OneParam(FOnViewportHoverResult, const FViewportHoverResult&);
FOnViewportHoverResult& OnViewportHoverResult();
```

Module shutdown must clear the hover delegate, matching the existing click delegate pattern.

Do not include mapped UV, landscape-local data, or failure reasons in the module event.

## Phase 2: Neutral Trace Helper in Editor Mode

Files:

- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`

Add a private neutral trace helper. It should contain the existing viewport validation, cursor ray construction, visibility trace, and result extraction currently embedded in `TraceLandscapeClick`.

Suggested shape:

```cpp
struct FViewportTraceResult
{
    bool bHit = false;
    FVector WorldPosition = FVector::ZeroVector;
    TWeakObjectPtr<AActor> HitActor;
    TWeakObjectPtr<UPrimitiveComponent> HitComponent;
};

bool TraceViewportUnderCursor(
    FEditorViewportClient* ViewportClient,
    FViewport* Viewport,
    FViewportTraceResult& OutResult) const;
```

Then rewrite `TraceLandscapeClick` to call the helper and broadcast only when a hit exists, preserving existing click behavior.

Keep trace channel, query params, and ray construction unchanged.

## Phase 3: Editor Mode MouseMove

Files:

- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`

Override:

```cpp
virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 x, int32 y) override;
virtual bool MouseLeave(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;
```

Mouse move behavior:

1. Call `FEdMode::MouseMove`.
2. If tracking is disabled, broadcast a clear only if needed and return the base result.
3. Trace under the cursor.
4. Broadcast `FViewportHoverResult` with `bHasHit = true` on hit.
5. Broadcast a clear result on miss.
6. Return the base result.

Mouse leave behavior:

1. Call `FEdMode::MouseLeave`.
2. Broadcast a clear result if tracking is enabled.
3. Return the base result.

Do not add a single `LastHoverMousePosition` cache.

If dedupe is required after profiling, key it by:

- `FEditorViewportClient*` or `FViewport*`;
- mouse x/y;
- view location/rotation and orthographic zoom.

## Phase 4: Optional Stationary-Cursor Camera Support

Files:

- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`

This phase is required only if the intended behavior includes camera movement under a stationary cursor.

Extend `Tick` carefully:

1. Preserve existing reverse-marker cleanup.
2. Track the last viewport client and viewport seen by `MouseMove`.
3. Store a compact last-view state for that viewport.
4. If tracking is enabled and the view state changes while the cursor is still inside the viewport, retrace and broadcast hover.

Do not poll every viewport. Do not run this when tracking is disabled.

If the implementation cannot confidently detect cursor-inside-viewport state, document the limitation and keep the MVP mouse-event driven.

## Phase 5: Panel Hover State

Files:

- `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`
- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Add panel state:

```cpp
FVector2D HoverMarkerUV = FVector2D::ZeroVector;
bool bHasHoverMarker = false;
FDelegateHandle HoverDelegateHandle;
```

Optionally add:

```cpp
TSharedPtr<SWidget> HeightmapImageWidget;
```

Use `TSharedPtr<SWidget>` rather than exposing the private `SHeightmapTrackerImageView` class in the header.

Subscribe in `Construct`:

```cpp
HoverDelegateHandle = OnViewportHoverResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportHover);
```

Unsubscribe in the destructor.

Add:

```cpp
void OnViewportHover(const FViewportHoverResult& Result);
void ClearHoverMarker();
bool SetHoverMarkerUV(const FVector2D& NewUV);
void InvalidateHeightmapMarkerPaint();
```

Panel hover handling:

1. If tracking is disabled, clear hover.
2. If no assigned landscape exists, clear hover.
3. If the hover result has no hit, clear hover.
4. If the hit does not belong to the assigned landscape, clear hover.
5. Refresh landscape bounds as the click path does.
6. Map world position to heightmap UV using a local mapping variable.
7. If mapping succeeds, update `HoverMarkerUV` and `bHasHoverMarker`.
8. If mapping fails, clear hover.
9. Invalidate paint only when the visible marker state actually changes.

Hover must not write:

- `LastMapping`;
- `LastLandscapeUV`;
- `bHasLandscapeUV`;
- reverse marker globals;
- status text, unless a later UX pass explicitly asks for it.

## Phase 6: Image View Rendering

Files:

- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Extend `SHeightmapTrackerImageView` args with hover marker attributes:

```cpp
SLATE_ATTRIBUTE(FVector2D, HoverMarkerUV)
SLATE_ATTRIBUTE(bool, HasHoverMarker)
```

Rendering rule:

1. If `HasHoverMarker` is true, draw the hover marker.
2. Else if `HasMarker` is true, draw the existing click marker.
3. Else draw no marker.

The hover marker can initially reuse the existing marker style to minimize scope. If visual distinction is desired later, use a small style change only in the image view.

Bind attributes in `SLandscapeHeightmapTrackerPanel::Construct`.

Use `SAssignNew(HeightmapImageWidget, SHeightmapTrackerImageView)` if targeted invalidation is preferred. Otherwise call panel-level `Invalidate(EInvalidateWidgetReason::Paint)`.

## Phase 7: Clear Paths

Files:

- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Call `ClearHoverMarker()` from:

- destructor;
- `SetTrackingEnabled(false)`;
- `AssignLandscape`;
- `ClearMarker`;
- image load/unload paths that clear the normal marker;
- hover miss/non-assigned/mapping-failure path;
- optional reverse click start if product behavior should hide hover while reverse action is being processed.

Do not call `ClearReverseMarker()` from hover clear.

## Phase 8: Verification

Manual checks:

- Hover over assigned landscape updates marker without clicking.
- Hover over non-assigned actor clears hover and leaves previous click marker intact.
- Hover miss clears hover and leaves previous click marker intact.
- Left click still sets the persistent click marker.
- Heightmap click still places the reverse marker in the viewport.
- Disabling tracking clears hover and does not clear reverse marker unless existing behavior already does.
- Closing the panel removes both click and hover delegate subscriptions.
- Multiple editor viewports do not reuse stale marker positions from another viewport.
- If optional Tick fallback is implemented, moving the camera with stationary cursor recomputes hover.

Source-level checks:

- Production changes are limited to the six files listed above.
- No changes to build dependencies unless compilation proves one is missing.
- No hover writes to reverse state.
- No single global mouse-position dedupe.
- No high-frequency hover logs.

## Expected Final Implementation Diff

Expected production files:

- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`
- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`
- `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`
- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

Expected documentation files:

- `evaluation/hover-tracking/reviewer-validation.md`
- `evaluation/hover-tracking/revised-implementation-plan.md`

No commit should be created unless explicitly requested.
