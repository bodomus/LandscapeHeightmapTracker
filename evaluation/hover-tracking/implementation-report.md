# Continuous Landscape Hover Tracking Implementation Report

## Summary

Implemented continuous forward hover tracking for the editor viewport. The feature now traces under the viewport cursor on `FEdMode::MouseMove`, maps valid assigned-Landscape hits to heightmap UV in the panel, and renders a transient hover marker above the existing persistent click marker.

The existing click flow and reverse heightmap-to-landscape flow were preserved.

## Architecture Actually Implemented

Runtime hover flow:

```text
FLandscapeHeightmapTrackerEdMode::MouseMove
  -> TraceViewportUnderCursor
  -> FLandscapeHeightmapTrackerModule::FViewportHoverResult
  -> FLandscapeHeightmapTrackerModule::OnViewportHoverResult
  -> SLandscapeHeightmapTrackerPanel::OnViewportHover
  -> IsAssignedLandscapeHit
  -> FLandscapeCoordinateMapper::MapWorldPosition
  -> bHasHoverMarker / HoverMarkerUV
  -> SHeightmapTrackerImageView hover attributes
  -> EInvalidateWidgetReason::Paint
  -> SHeightmapTrackerImageView::OnPaint
```

Click flow after implementation:

```text
InputKey LeftMouseButton IE_Pressed
  -> TraceLandscapeClick
  -> TraceViewportUnderCursor
  -> OnViewportClickResult
  -> OnViewportClick
  -> bHasMarker / MarkerUV
```

Reverse flow remains:

```text
SHeightmapTrackerImageView::OnMouseButtonDown
  -> OnHeightmapClicked
  -> MapUVToLocalPosition
  -> TraceAssignedLandscapeSurface
  -> SetReverseMarker
  -> FLandscapeHeightmapTrackerEdMode::Render
```

## Files Changed

File: `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
Symbols changed: `FViewportHoverResult`, `FOnViewportHoverResult`, `OnViewportHoverResult`.
Reason: add separate module-owned hover event.

File: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`
Symbols changed: `GOnViewportHoverResult`, `OnViewportHoverResult`, `ShutdownModule`.
Reason: store and clear the hover multicast delegate alongside the click delegate.

File: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`
Symbols changed: `MouseMove`, `MouseLeave`, `FViewportTraceResult`, `FHoverViewState`, hover viewport/view-state members.
Reason: add passive hover hook, shared trace helper declaration, and narrow Tick fallback state.

File: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`
Symbols changed: `Tick`, `MouseMove`, `MouseLeave`, `TraceLandscapeClick`, `TraceViewportUnderCursor`, hover broadcast/view-state helpers.
Reason: implement hover tracing, reuse trace mechanics for click and hover, and retrace on camera view-state changes only for the last relevant viewport.

File: `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`
Symbols changed: `OnViewportHover`, `ClearHoverMarker`, `SetHoverMarkerUV`, `InvalidateHeightmapMarkerPaint`, hover state fields, `HoverDelegateHandle`, `HeightmapImageWidget`.
Reason: add transient panel-owned hover marker state and targeted paint invalidation.

File: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`
Symbols changed: `SHeightmapTrackerImageView`, `Construct`, destructor, `ClearMarker`, `OnViewportClick`, `OnViewportHover`, `SetTrackingEnabled`, `AssignLandscape`, `ReleaseTexture`, `LoadPngTexture`, hover helper methods.
Reason: bind hover delegate, map hover hits, render hover above click marker, clear hover on lifecycle paths, and invalidate paint only when marker state changes.

## Camera-Motion Fallback

Remembered viewport state:

- raw `FEditorViewportClient*`;
- raw `FViewport*`;
- last mouse position from `MouseMove`;
- `bHasRememberedHoverViewport`.

Remembered view state:

- `GetViewLocation()`;
- `GetViewRotation()`;
- `GetOrthoZoom()`.

View-change comparison:

- location uses `FVector::Equals(..., KINDA_SMALL_NUMBER)`;
- rotation uses `FRotator::Equals(..., KINDA_SMALL_NUMBER)`;
- ortho zoom uses `FMath::IsNearlyEqual(..., KINDA_SMALL_NUMBER)`.

Tick retraces only when:

- tracking is enabled;
- existing reverse-marker cleanup has already been preserved and run;
- the current `Tick` viewport client is the remembered client;
- `ViewportClient->Viewport` still matches the remembered viewport;
- the current mouse position is inside that viewport;
- the view state changed since the last remembered state.

Stale pointer/lifetime safeguards:

- the fallback does not poll every viewport;
- it only runs for the viewport client passed into `Tick`;
- it requires `ViewportClient->Viewport == LastHoverViewport` before dereferencing the remembered viewport;
- remembered state is cleared on tracking disabled and matching `MouseLeave`.

Unavoidable lifetime assumption:

`FEditorViewportClient` and `FViewport` are non-UObject editor objects, so this implementation stores raw pointers. The lifetime window is intentionally narrow and validated against the current `Tick` client before use.

## State Ownership

| State | Owner | Click writes | Hover writes | Reverse writes |
| --- | --- | --- | --- | --- |
| `bHasMarker` | Panel click state | Yes | No | No |
| `MarkerUV` | Panel click state | Yes | No | No |
| `bHasHoverMarker` | Panel hover state | No | Yes | No |
| `HoverMarkerUV` | Panel hover state | No | Yes | No |
| `LastMapping` | Panel diagnostics | Yes | No | Yes |
| `LastLandscapeUV` | Panel reverse diagnostics | Clear/update existing behavior | No | Yes |
| `bHasLandscapeUV` | Panel diagnostics mode flag | Existing click behavior | No | Yes |
| `StatusText` | Panel status | Yes | No | Yes |
| reverse marker globals | Module reverse state | No | No | Yes |
| remembered hover viewport/view state | EdMode hover fallback | No | Yes | No |

## Performance Behavior

Traces occur:

- on `MouseMove` while tracking is enabled;
- on `Tick` only for the last relevant viewport when view state changed under a stationary cursor.

Traces do not occur:

- when tracking is disabled;
- on unchanged Tick view state;
- for every viewport;
- from a timer;
- from a fixed millisecond throttle/debounce path.

Invalidation conditions:

- hover marker appears;
- hover UV changes beyond a small UV epsilon;
- hover marker clears;
- click marker changes while no hover marker is active;
- marker state is cleared because landscape/heightmap state changes.

Dedupe strategy:

- no EdMode single mouse-position dedupe;
- panel dedupes hover marker UV with `FVector2D::Equals(..., KINDA_SMALL_NUMBER)`;
- paint invalidation uses `EInvalidateWidgetReason::Paint`.

No timer or fixed throttle was added.

## Build Results

Command:

```text
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' UE57EditorEditor Win64 Development -Project='J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -WaitMutex -NoHotReload
```

Result: succeeded.

Notes:

- Unreal Build Tool reported `Result: Succeeded`.
- Warnings were existing engine/deprecation warnings, including Visual Studio preferred-version warning and engine API deprecation warnings.
- No compile fixes outside the expected six production files were required.

## Test Results

Command:

```text
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoSplash -NullRHI -NoSound -ExecCmds='Automation RunTests LandscapeHeightmapTracker; Quit' -TestExit='Automation Test Queue Empty'
```

Result: succeeded, exit code 0.

Automation log reported 9 matching tests and all completed with `Result={Success}`:

- `LandscapeHeightmapTracker.Mapper.Basic`
- `LandscapeHeightmapTracker.Mapper.OptionsAndTransforms`
- `LandscapeHeightmapTracker.ReverseMapping.FittedImageClick`
- `LandscapeHeightmapTracker.ReverseMapping.UVToLocal`
- `LandscapeHeightmapTracker.ReverseMapping.VerticalTraceSegment`
- `LandscapeHeightmapTracker.ViewportTrace.InvalidDirection`
- `LandscapeHeightmapTracker.ViewportTrace.Orthographic`
- `LandscapeHeightmapTracker.ViewportTrace.Perspective`
- `LandscapeHeightmapTracker.ViewportTrace.ReversedOrthographic`

## Manual Verification

Manual editor interaction was not performed in this environment. Status below is therefore intentionally conservative.

| Scenario | Status |
| --- | --- |
| tracking enabled | NOT VERIFIED |
| mouse moves over assigned Landscape | NOT VERIFIED |
| hover marker appears without click | NOT VERIFIED |
| hover marker follows mouse movement | NOT VERIFIED |
| click at same point produces consistent mapped position | NOT VERIFIED |
| hover over non-assigned actor | NOT VERIFIED |
| hover over empty space | NOT VERIFIED |
| mouse leaves viewport | NOT VERIFIED |
| persistent click marker reappears after hover clear | NOT VERIFIED |
| click marker remains functional | NOT VERIFIED |
| reverse heightmap click remains functional | NOT VERIFIED |
| reverse world marker remains visible while hover updates | NOT VERIFIED |
| camera moves with stationary cursor | NOT VERIFIED |
| hover recomputes only when view state changes | NOT VERIFIED |
| tracking disable clears hover | NOT VERIFIED |
| panel close/reopen produces no stale callback | NOT VERIFIED |
| multiple editor viewports | NOT VERIFIED |
| assigned Landscape change | NOT VERIFIED |
| invalid/deleted assigned Landscape | NOT VERIFIED |
| rapid mouse movement | NOT VERIFIED |
| normal camera controls still work | NOT VERIFIED |
| gizmo/editor selection input is not consumed | NOT VERIFIED |

## Regression Analysis

- `InputKey` still handles left mouse press and calls `TraceLandscapeClick` only when tracking is enabled.
- `TraceLandscapeClick` still broadcasts `OnViewportClickResult` for valid hits.
- `OnViewportClick` still updates persistent `bHasMarker` / `MarkerUV` and diagnostics.
- `OnHeightmapClicked` keeps the reverse mapping path and still calls `SetReverseMarker`.
- Hover does not call `OnHeightmapClicked`, `MapUVToLocalPosition`, `TraceAssignedLandscapeSurface`, `SetReverseMarker`, `ClearReverseMarker`, or `RedrawAllViewports`.
- `Render` reverse marker behavior remains intact.
- Existing `Tick` reverse cleanup remains intact and runs before hover fallback.
- Module shutdown now clears both click and hover delegates.
- Panel destructor removes both click and hover delegate handles.

## Known Limitations

- Manual viewport behavior was not verified interactively.
- The Tick fallback stores raw editor viewport pointers because the relevant UE editor viewport types are not UObject/weak-pointer managed. The implementation validates them against the current `Tick` client before use.
- Hover tracing uses the same visibility trace mechanics as click tracing; any pre-existing trace-channel limitations are unchanged.
