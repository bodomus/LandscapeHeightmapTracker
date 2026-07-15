# Independent Code Review: Continuous Landscape Hover Tracking

## Executive summary

The implementation is broadly aligned with the reviewed architecture and is reasonable to take into interactive Unreal Editor validation. The primary `MouseMove` path is correct, click and reverse flows remain separate, and Slate paint invalidation is targeted and mostly complete.

I found no blockers and no high-severity issues. Two issues should be tracked: one real scope drift in `.gitignore`, and one camera-motion fallback gap where projection or viewport-size changes can stale the hover marker because the stored view state only includes location, rotation, and ortho zoom.

## Diff scope review

Required commands were run and inspected:

```text
git status
git diff --stat
git diff
```

Tracked changed files:

```text
.gitignore
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp
Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h
Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h
```

Production changes are limited to the expected six source files. The `.gitignore` change adds `.code-review-graph/`, which is unrelated to the hover implementation. There are also untracked docs/config files under `.codex/`, `AGENTS.md`, and `evaluation/`; these are not production source.

Scope verdict: `MINOR SCOPE DRIFT`.

The implementation report accurately describes the six production source changes, but it does not mention the tracked `.gitignore` change.

## Findings

ID: LHT-HOVER-001
Severity: MEDIUM
File: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h`, `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`
Symbol: `FHoverViewState`, `MakeHoverViewState`, `HasRememberedViewChanged`
Problem: The Tick fallback does not include projection- or viewport-size inputs that can change the cursor ray under a stationary cursor.
Evidence: `FHoverViewState` stores only `Location`, `Rotation`, and `OrthoZoom` at `LandscapeHeightmapTrackerEdMode.h:27-33`; `MakeHoverViewState` fills only `GetViewLocation()`, `GetViewRotation()`, and `GetOrthoZoom()` at `LandscapeHeightmapTrackerEdMode.cpp:253-259`; equality checks only those fields at `LandscapeHeightmapTrackerEdMode.cpp:276-280`. UE 5.7 `GetCursorWorldLocationFromMousePos()` rebuilds a `FSceneView` from `Viewport` and `CalcSceneView()` at `EditorViewportClient.cpp:6370-6387`. That projection uses `ModifiedViewFOV`, viewport dimensions, aspect ratio axis constraint, and `AspectRatio` in `EditorViewportClient.cpp:1237-1308`; engine navigation also updates `ViewFOV` independently at `EditorViewportClient.cpp:2386-2404`.
Impact: Camera FOV zoom, viewport resize, aspect-ratio/constrained-view changes, or viewport type/projection changes can alter the ray and world hit while `HasRememberedViewChanged()` returns false. The hover marker can remain stale until another mouse move or location/rotation/ortho zoom change.
Recommended fix: Extend the remembered fallback state with practical ray-affecting inputs available from public API, at minimum viewport size and viewport type. If FOV is not public on `FEditorViewportClient`, document that limitation explicitly or derive a comparable projection key from APIs available in UE 5.7.

ID: LHT-HOVER-002
Severity: LOW
File: `.gitignore`
Symbol: repository ignore rules
Problem: The diff includes an unrelated `.gitignore` change for `.code-review-graph/`.
Evidence: `git diff` shows `.gitignore` adding `# Added by code-review-graph` and `.code-review-graph/`. This file is outside the expected production diff surface and is unrelated to continuous hover tracking.
Impact: Minor scope drift and slightly dirtier review/commit surface. It does not affect runtime behavior.
Recommended fix: Exclude this change from the hover feature commit unless the user intentionally wants CRG output ignored as part of a separate housekeeping change.

## Verified correct areas

- `FLandscapeHeightmapTrackerEdMode::MouseMove` has the correct UE 5.7 signature and calls `FEdMode::MouseMove` before hover work.
- Mouse move tracing is gated by `FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled()`.
- Hover hit and clear are represented by a separate `FViewportHoverResult` with weak hit actor/component references.
- Click and hover reuse the same neutral `TraceViewportUnderCursor` mechanics.
- Hover mapping uses the same assigned-Landscape hit semantics as click through `IsAssignedLandscapeHit`.
- Hover does not write `LastMapping`, `LastLandscapeUV`, `bHasLandscapeUV`, `StatusText`, reverse marker globals, `bHasMarker`, or `MarkerUV`.
- `SHeightmapTrackerImageView::OnPaint` renders hover first, then click, then no marker.
- Module shutdown clears both click and hover delegates.
- Panel destruction removes both delegate handles.
- Existing reverse marker cleanup in `Tick` still runs before the hover fallback.

## Multi-viewport analysis

Scenario A: hover viewport A at `x=500,y=300`, then hover viewport B at the same coordinates.

`MouseMove` does not dedupe by a global mouse coordinate. It calls `RememberHoverViewport(ViewportClient, Viewport, x, y)` for every movement and then traces the passed viewport/client (`LandscapeHeightmapTrackerEdMode.cpp:94-98`). B therefore traces independently and replaces remembered state with B.

Scenario B: hover A, then move camera A with stationary cursor.

`Tick` receives a `ViewportClient`. It only proceeds if that current client equals `LastHoverViewportClient`, the remembered `FViewport*` is present, `ViewportClient->Viewport == LastHoverViewport`, the current viewport mouse position is inside the viewport, and view state changed (`LandscapeHeightmapTrackerEdMode.cpp:43-57`, `225-238`). For A this causes one retrace per detected view-state change.

Scenario C: hover A, then hover B, then Tick A, then Tick B.

After B's `MouseMove`, `LastHoverViewportClient` and `LastHoverViewport` point at B (`LandscapeHeightmapTrackerEdMode.cpp:202-209`). A's later `Tick` fails `ViewportClient != LastHoverViewportClient` and does not retrace. B's `Tick` can retrace if B's view state changed. This is correct for the "last relevant hovered viewport" requirement.

Scenario D: hover A, close/recreate A.

The fallback stores raw `FEditorViewportClient*` and `FViewport*` (`LandscapeHeightmapTrackerEdMode.h:47-48`). It does not dereference `LastHoverViewportClient`; it compares the current `Tick` client pointer to the stored pointer first (`LandscapeHeightmapTrackerEdMode.cpp:225-232`). It dereferences `LastHoverViewport` only after `ViewportClient->Viewport == LastHoverViewport` (`LandscapeHeightmapTrackerEdMode.cpp:232-238`). This is acceptable for a narrow editor-mode fallback, but it is not as strong as weak ownership because editor viewport objects are non-UObject raw-pointer APIs. Pointer reuse after close/recreate is theoretically possible but unlikely; manual validation should include closing/reopening viewport layouts and panel tabs.

MouseLeave sequencing:

- If A leaves before B moves, `MouseLeave(A)` clears remembered state and broadcasts hover clear; B's next `MouseMove` traces B.
- If B moves before A's delayed `MouseLeave(A)`, remembered state points at B and `MouseLeave(A)` does not clear B because `Viewport != LastHoverViewport` (`LandscapeHeightmapTrackerEdMode.cpp:107-114`).

## Camera-motion fallback analysis

Stored viewport/client pointers:

- `FEditorViewportClient* LastHoverViewportClient`
- `FViewport* LastHoverViewport`

Stored view state:

- `FVector Location`
- `FRotator Rotation`
- `float OrthoZoom`

Equality:

- `FVector::Equals(..., KINDA_SMALL_NUMBER)`
- `FRotator::Equals(..., KINDA_SMALL_NUMBER)`
- `FMath::IsNearlyEqual(..., KINDA_SMALL_NUMBER)`

Retrace happens only when:

- tracking is enabled;
- remembered viewport state exists;
- current `Tick` client is the remembered client;
- current client's `Viewport` equals the remembered viewport;
- current mouse position from `LastHoverViewport->GetMouseX/Y()` is inside the remembered viewport size;
- remembered location/rotation/ortho zoom changed.

Retrace does not happen when:

- tracking is disabled;
- Tick is for another viewport client;
- the client no longer points at the remembered viewport;
- mouse position is outside the viewport;
- only FOV/projection, aspect ratio, viewport size, or constrained-view rectangle changed.

Verdict: `ACCEPTABLE WITH LIMITATION`.

Raw pointer safety:

The implementation validates aggressively enough for normal editor lifecycle. It does not own the viewport objects, and it cannot use `TWeakObjectPtr` because these are not UObjects. It should still be manually tested against viewport layout changes and tab close/reopen.

View-state coverage:

Location/rotation/ortho zoom are sufficient for normal perspective camera translation/rotation and orthographic pan/zoom. They are not complete for FOV zoom, aspect ratio, constrained view rect, viewport size, or viewport type changes. This is the medium finding above.

## Slate invalidation analysis

`HeightmapImageWidget` is stored as `TSharedPtr<SWidget>` in the public panel header (`SLandscapeHeightmapTrackerPanel.h:76`), so the private concrete image class remains private. `InvalidateHeightmapMarkerPaint()` invalidates that widget with `EInvalidateWidgetReason::Paint`, falling back to panel invalidation if the widget is not valid (`SLandscapeHeightmapTrackerPanel.cpp:767-775`).

Verified transitions:

| Transition | Source behavior | Verdict |
| --- | --- | --- |
| no marker -> hover marker | `SetHoverMarkerUV` sets `bHasHoverMarker` and invalidates | correct |
| hover marker UV A -> UV B | `SetHoverMarkerUV` invalidates unless UV is equal within epsilon | correct |
| hover marker -> persistent click marker | `ClearHoverMarker` clears hover and invalidates; `OnPaint` falls back to click | correct |
| hover marker -> no marker | `ClearHoverMarker` clears hover and invalidates | correct |
| click marker A -> click marker B while hover inactive | `OnViewportClick` invalidates when `!bHasHoverMarker` | correct |
| click marker changes while hover active | no invalidation, but visible marker remains hover; later hover clear invalidates | correct |
| tracking disable | `SetTrackingEnabled(false)` clears hover and invalidates if hover existed | correct |
| Landscape reassignment | `AssignLandscape` clears hover and always invalidates | correct |
| heightmap reload | `ReleaseTexture` and `LoadPngTexture` clear marker state and invalidate | correct |

The implementation avoids layout invalidation for marker-only updates.

UV dedupe classification: `HARMLESS BUT USELESS`. UV is normalized 0..1, and `KINDA_SMALL_NUMBER` is far smaller than one pixel for normal heightmap sizes, so it only suppresses near-identical floating point repeats. Pixel-coordinate equality would be more meaningful, but no change is required for correctness.

## Click regression analysis

Verdict: `PRESERVED`.

`InputKey` still handles only left mouse `IE_Pressed`, gates on tracking enabled, calls `TraceLandscapeClick`, and returns false (`LandscapeHeightmapTrackerEdMode.cpp:14-25`). `TraceLandscapeClick` still broadcasts `OnViewportClickResult` only after a valid trace hit (`LandscapeHeightmapTrackerEdMode.cpp:119-140`). `OnViewportClick` still performs assigned-Landscape validation, refreshes bounds, maps with `MapWorldPosition`, updates `LastMapping`, `bHasMarker`, `MarkerUV`, diagnostics, and status (`SLandscapeHeightmapTrackerPanel.cpp:439-489`).

Click marker state is not lost while hover is active: `OnViewportClick` updates `MarkerUV` even when hover is visible and skips paint invalidation only because visible output remains hover until hover clears.

## Reverse regression analysis

Verdict: `PRESERVED`.

`OnHeightmapClicked` still validates assigned landscape and loaded texture, calls `MapUVToLocalPosition`, transforms to world XY, calls `TraceAssignedLandscapeSurface`, writes reverse diagnostics, calls `SetReverseMarker`, and updates status (`SLandscapeHeightmapTrackerPanel.cpp:527-589`). `Render` still draws the reverse marker from `GetReverseMarker` (`LandscapeHeightmapTrackerEdMode.cpp:60-78`). `Tick` still consumes reverse cleanup requests before any hover fallback (`LandscapeHeightmapTrackerEdMode.cpp:28-35`).

Search confirmed hover path does not call `OnHeightmapClicked`, `MapUVToLocalPosition`, `TraceAssignedLandscapeSurface`, `SetReverseMarker`, `ClearReverseMarker`, or `RedrawAllViewports`. `RedrawAllViewports` remains confined to module reverse marker redraw behavior.

## Performance analysis

1. Every `MouseMove` while tracking is enabled generates one trace. This is acceptable for the MVP and matches the reviewed event-driven plan.
2. Every detected view-state change for the last relevant viewport generates one retrace.
3. Tick does not poll every viewport for traces; non-remembered viewport ticks fail the identity check.
4. Tick can retrace once for each observed location/rotation/ortho zoom change. It updates `LastHoverViewState` after retrace, so it does not retrace repeatedly on unchanged state.
5. Clear results can be broadcast repeatedly on repeated mouse-move misses, but the panel suppresses repeated paint invalidation because `ClearHoverMarker` returns early when hover is already absent.
6. There is no timer, wall-clock throttle, debounce, high-frequency hover logging, or global viewport redraw for hover.
7. The only visible inefficiency is repeated delegate broadcasts on misses; harmless for MVP.

## State ownership table

| State | Owner | Click writes? | Hover writes? | Reverse writes? | Clear paths |
| --- | --- | --- | --- | --- | --- |
| `bHasMarker` | Panel click state | Yes, `OnViewportClick` | No | No | `ClearMarker`, `AssignLandscape`, `ReleaseTexture`, `LoadPngTexture`, invalid click mapping |
| `MarkerUV` | Panel click state | Yes, `OnViewportClick` | No | No | Value becomes irrelevant when `bHasMarker=false` |
| `bHasHoverMarker` | Panel hover state | No | Yes, `ClearHoverMarker`/`SetHoverMarkerUV` | No | tracking disable, panel destruction, assignment, texture release/load, hover miss, wrong actor, mapping failure |
| `HoverMarkerUV` | Panel hover state | No | Yes | No | reset by `ClearHoverMarker` |
| `LastMapping` | Panel diagnostics | Yes | No | Yes | `ClearMarker`, reverse/click mapping update |
| `LastLandscapeUV` | Panel reverse diagnostics | Cleared by clear-all | No | Yes | `ClearMarker` |
| `bHasLandscapeUV` | Panel diagnostics flag | Yes, click clears reverse mode | No | Yes | `ClearMarker`, `AssignLandscape` |
| `StatusText` | Panel status | Yes | No | Yes | many user actions; hover does not write |
| reverse marker globals | Module reverse state | No | No | Yes | `ClearReverseMarker`, invalid owner cleanup, panel destruction, assignment, clear-all |
| remembered viewport state | EdMode hover fallback | No | Yes | No | tracking disabled, matching `MouseLeave`, next `MouseMove` |
| remembered view state | EdMode hover fallback | No | Yes | No | tracking disabled, matching `MouseLeave`, next `MouseMove`, Tick retrace update |

## Build/test verification

Build command:

```text
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' UE57EditorEditor Win64 Development -Project='J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -WaitMutex -NoHotReload
```

Result: succeeded. UBT reported `Target is up to date` and `Result: Succeeded`.

Automation command:

```text
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoSplash -NullRHI -NoSound -ExecCmds='Automation RunTests LandscapeHeightmapTracker; Quit' -TestExit='Automation Test Queue Empty'
```

Result: process exited 0. Log reported 9 tests found and all completed with `Result={Success}`:

- `LandscapeHeightmapTracker.Mapper.Basic`
- `LandscapeHeightmapTracker.Mapper.OptionsAndTransforms`
- `LandscapeHeightmapTracker.ReverseMapping.FittedImageClick`
- `LandscapeHeightmapTracker.ReverseMapping.UVToLocal`
- `LandscapeHeightmapTracker.ReverseMapping.VerticalTraceSegment`
- `LandscapeHeightmapTracker.ViewportTrace.InvalidDirection`
- `LandscapeHeightmapTracker.ViewportTrace.Orthographic`
- `LandscapeHeightmapTracker.ViewportTrace.Perspective`
- `LandscapeHeightmapTracker.ViewportTrace.ReversedOrthographic`

These automated tests do not exercise live editor viewport hover behavior.

## Manual validation risks

Must be tested interactively in Unreal Editor:

- hover appears over assigned Landscape without clicking;
- hover follows mouse movement;
- miss over empty space clears hover and shows persistent click marker;
- hover over non-assigned actor clears hover;
- click while hover is active updates persistent marker;
- heightmap reverse click still sets world marker;
- reverse marker remains while hover updates;
- stationary cursor plus camera translate/rotate recomputes hover;
- FOV zoom and viewport resize behavior, because the current fallback does not key those states;
- multiple level editor viewports and viewport switching;
- close/reopen panel and viewport layout changes;
- tracking disable while reverse marker exists;
- selection, gizmo movement, and normal camera controls are not affected.

## Required adversarial checks

1. Can raw stored viewport pointers become unsafe? Acceptable with limitation. The code validates against the current Tick client before dereferencing the stored viewport, but raw non-owning editor pointers remain weaker than weak ownership.
2. Can camera movement change the cursor ray without changing location, rotation, or ortho zoom? Yes, FOV/projection changes can.
3. Can viewport resize or FOV change stale the marker? Yes. This is finding LHT-HOVER-001.
4. Can the Tick fallback retrace the wrong viewport? Not in normal multi-viewport flow; it requires the current Tick client and viewport to match remembered identity.
5. Can hover remain visible after switching viewports? Briefly until the next B `MouseMove` or A leave/clear, depending event order; the code handles normal A/B sequencing safely.
6. Can hover survive tracking disable because reverse marker keeps the mode active? Panel `SetTrackingEnabled(false)` clears hover, and EdMode clears remembered state when tracking is disabled.
7. Can click marker state be lost while hover is active? No. Click updates persistent `MarkerUV`; hover only masks it visually.
8. Can reverse marker state be affected indirectly? Hover does not call reverse helpers. Existing clear-all, assignment, panel destruction behavior still clears reverse markers as before.
9. Is Slate invalidation complete for every visible-state transition? Yes for reviewed marker transitions.
10. Is UV epsilon dedupe meaningful at real heightmap resolutions? Mostly no; harmless but not very useful.
11. Are repeated miss broadcasts excessive or harmless? Harmless for MVP; panel suppresses repeated visual invalidation.
12. Is the implementation ready for interactive manual validation? Yes, with LHT-HOVER-001 tracked as an edge-case limitation to verify/fix.
