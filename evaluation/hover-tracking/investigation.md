# Continuous Landscape Hover Tracking Investigation

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

Scope: investigation and implementation planning only. Production source was inspected but not modified.

Routing followed `AGENTS.md`: Graphify and CRG were used as candidate generators; all conclusions below are based on `rg`, plugin source, and local Unreal Engine 5.7 headers/source.

## 1. Current Forward Flow

The supplied click flow is accurate.

Verified flow:

```text
Left mouse press
  -> FLandscapeHeightmapTrackerEdMode::InputKey
  -> TraceLandscapeClick
  -> FEditorViewportClient::GetCursorWorldLocationFromMousePos
  -> FViewportTraceRayBuilder::BuildTraceSegment
  -> UWorld::LineTraceSingleByChannel
  -> FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast
  -> SLandscapeHeightmapTrackerPanel::OnViewportClick
  -> FLandscapeCoordinateMapper::MapWorldPosition
  -> bHasMarker / MarkerUV
  -> SHeightmapTrackerImageView Slate attributes
  -> SHeightmapTrackerImageView::OnPaint
```

Evidence:

```text
File: Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Symbol: FLandscapeHeightmapTrackerEdMode::InputKey
Observed behavior: Lines 14-25 handle only LeftMouseButton + IE_Pressed, gate on IsTrackingModeEnabled(), call TraceLandscapeClick(), then return false.
Architectural implication: Current forward acquisition is click-driven and lives in the active editor mode.
```

```text
File: Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Symbol: FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick
Observed behavior: Lines 58-129 validate ViewportClient/Viewport/GEditor/World, read GetCursorWorldLocationFromMousePos() at line 71, call BuildTraceSegment at lines 72-75, run LineTraceSingleByChannel at line 110, fill FViewportClickResult at lines 123-126, and broadcast at line 127.
Architectural implication: Trace mechanics are already centralized, but the method name and output delegate are click-specific.
```

```text
File: Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h
Symbol: FViewportClickResult / FOnViewportClickResult
Observed behavior: Lines 12-19 define the result payload and multicast delegate.
Architectural implication: Cross-subsystem communication is already delegate-based and weak-object safe for hit actor/component.
```

```text
File: Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Symbol: SLandscapeHeightmapTrackerPanel::Construct / ~SLandscapeHeightmapTrackerPanel
Observed behavior: Line 176 binds OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick); line 333 removes ClickDelegateHandle.
Architectural implication: Panel lifetime currently owns the callback binding; hover should follow the same AddSP/Remove pattern if panel receives hover events.
```

```text
File: Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Symbol: SLandscapeHeightmapTrackerPanel::OnViewportClick
Observed behavior: Lines 424-463 gate bTrackClicks, require AssignedLandscape, reject hits not belonging to the assigned Landscape, refresh bounds, call MapWorldPosition at line 449, and write bHasMarker/MarkerUV at lines 452-454.
Architectural implication: The panel owns forward marker state and assigned-Landscape filtering. The editor mode does not know the assigned Landscape.
```

```text
File: Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Symbol: SHeightmapTrackerImageView
Observed behavior: Lines 43-49 declare ImageBrush, MarkerUV, HasMarker, and click delegates as Slate arguments. Lines 55-59 store them. Lines 245-248 bind MarkerUV and HasMarker to panel lambdas. Lines 87-105 read those attributes and paint crosshair lines.
Architectural implication: The marker widget is passive; marker state changes in the panel must be made visible through attribute reads and repaint.
```

Missing intermediate behavior: there is no explicit `Invalidate()` after click marker writes. The current UI likely repaints because the update happens during editor/Slate activity, but continuous hover from viewport movement should not rely on incidental repaint.

## 2. Current Reverse Flow

Verified reverse flow:

```text
Heightmap left click
  -> SHeightmapTrackerImageView::OnMouseButtonDown
  -> FHeightmapImageClickMapper::MapLocalPositionToDisplayUV
  -> SLandscapeHeightmapTrackerPanel::OnHeightmapClicked
  -> FLandscapeCoordinateMapper::MapUVToLocalPosition
  -> AssignedLandscape->GetActorTransform().TransformPosition
  -> FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface
  -> FLandscapeHeightmapTrackerModule::SetReverseMarker
  -> FLandscapeHeightmapTrackerEdMode::Render draws world marker
```

Evidence:

```text
File: Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Symbol: SHeightmapTrackerImageView::OnMouseButtonDown
Observed behavior: Lines 111-139 handle only left button, map local widget position to display UV, then ExecuteIfBound(OnHeightmapClicked) at line 131.
Architectural implication: Reverse flow starts inside the heightmap widget, not the editor viewport.
```

```text
File: Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Symbol: SLandscapeHeightmapTrackerPanel::OnHeightmapClicked
Observed behavior: Lines 465-527 validate AssignedLandscape and heightmap, refresh bounds, call MapUVToLocalPosition at line 485, TransformPosition at line 493, TraceAssignedLandscapeSurface at line 495, write LastMapping/LastLandscapeUV/bHasLandscapeUV, and call SetReverseMarker at line 512.
Architectural implication: Reverse mapping updates diagnostic mapping state and the world reverse marker, but it does not set bHasMarker or MarkerUV for the image marker.
```

```text
File: Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp
Symbol: SetReverseMarker / GetReverseMarker / ClearReverseMarker
Observed behavior: Lines 76-83 store reverse marker world position and weak owner, activate the editor mode, and request viewport redraw. Lines 96-110 clear stale weak owner state. Lines 86-93 clear the reverse marker.
Architectural implication: Reverse marker state is module-global and weak-owner guarded; forward hover must not use or clear this state except through existing user actions.
```

```text
File: Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Symbol: Render / Tick
Observed behavior: Lines 38-55 draw the reverse marker line when GetReverseMarker succeeds. Lines 28-36 consume cleanup requests and clear invalid reverse marker state.
Architectural implication: Reverse visual result is in the editor viewport, not in the heightmap widget.
```

Shared state:

- `AssignedLandscape`, `LocalBounds`, `ImageSize`, `bFlipX`, `bFlipY`, and `LastMapping` are panel state used by both directions.
- `LastMapping` is shared diagnostic state.

Independent state:

- Forward image marker: `bHasMarker`, `MarkerUV` in `SLandscapeHeightmapTrackerPanel`.
- Reverse world marker: `GHasReverseMarker`, `GReverseMarkerWorldPosition`, `GReverseMarkerOwner` in `LandscapeHeightmapTrackerModule.cpp`.
- Reverse UV diagnostics: `LastLandscapeUV`, `bHasLandscapeUV`.

Continuous hover should not overwrite, hide, or change the lifetime of the reverse marker if it avoids `SetReverseMarker`, `ClearReverseMarker`, and reverse module state. It also should not call `OnHeightmapClicked`, `MapUVToLocalPosition`, or `TraceAssignedLandscapeSurface`.

## 3. Unreal Editor Viewport Mouse Movement Hooks

Local engine inspected: `C:\Program Files\Epic Games\UE_5.7`.

### Candidate Comparison

| Mechanism | Available/signature | Called when | Capture required | Passive movement | Appropriate? | Risks/dependencies |
|---|---|---|---|---|---|---|
| `FEdMode::MouseMove` | `virtual bool MouseMove(FEditorViewportClient*, FViewport*, int32 x, int32 y)` | `FEditorViewportClient::MouseMove` forwards ordinary mouse movement to `ModeTools->MouseMove` | No | Yes | Preferred | Return false to avoid consuming editor interaction. Existing `UnrealEd` dependency already covers it. |
| `FEdMode::CapturedMouseMove` | `virtual bool CapturedMouseMove(FEditorViewportClient*, FViewport*, int32, int32)` | Only while window input capture is in effect | Yes | No | Rejected as primary | Would miss ordinary hover; useful only during drags/captured input. |
| `FEdMode::ProcessCapturedMouseMoves` | `virtual bool ProcessCapturedMouseMoves(... TArrayView<FIntPoint>)` | Accumulated captured moves per frame | Yes | No | Rejected | Same capture limitation; batching is for drag/capture scenarios. |
| `FEdMode::Tick` | `virtual void Tick(FEditorViewportClient*, float)` | Every editor mode tick per viewport client | No | Polling only | Secondary fallback only | Would trace even when cursor is stationary unless deduped; less directly tied to movement. |
| `InputKey` | `virtual bool InputKey(... FKey, EInputEvent)` | Key/button events | No | No | Existing click flow only | Does not receive passive movement. |
| Viewport polling | `FEditorViewportClient::GetCursorWorldLocationFromMousePos()` | Callable from active viewport context | No | Only if called by another hook | Use inside chosen hook | Needs valid viewport client/world. |
| Editor delegates | Not needed for per-viewport movement | Varies | Varies | Varies | Rejected | More global lifecycle complexity than mode-local hook. |
| Slate-level mouse movement | `SWidget::OnMouseMove` style hooks | Slate widget under pointer | No | Only for that Slate widget | Rejected | The plugin's heightmap widget receives image mouse movement, not level viewport world hover; would couple to LevelEditor viewport widgets. |

Engine evidence:

```text
Engine file/header: C:\Program Files\Epic Games\UE_5.7\Engine\Source\Editor\UnrealEd\Public\EdMode.h
Engine symbol: FEdMode::MouseMove / MouseLeave / CapturedMouseMove / Tick
Observed contract: Lines 51-76 declare MouseEnter, MouseLeave, MouseMove, CapturedMouseMove, ProcessCapturedMouseMoves, InputKey. Lines 61-69 document CapturedMouseMove as movement while window input capture is in effect. Line 71 declares the capture-specific signature.
Implication for this plugin: The existing legacy `FEdMode` class can directly override passive `MouseMove` and optional `MouseLeave`.
```

```text
Engine file/header: C:\Program Files\Epic Games\UE_5.7\Engine\Source\Editor\UnrealEd\Private\EditorViewportClient.cpp
Engine symbol: FEditorViewportClient::MouseMove / GetCursorWorldLocationFromMousePos / CapturedMouseMove
Observed contract: Lines 6344-6355 set CurrentMousePos and call ModeTools->MouseMove(this, Viewport, x, y). Lines 6370-6387 build FViewportCursorLocation from current viewport mouse X/Y. Lines 6390-6402 route captured movement separately.
Implication for this plugin: `MouseMove` is event-driven on ordinary movement and can reuse `GetCursorWorldLocationFromMousePos`.
```

```text
Engine file/header: C:\Program Files\Epic Games\UE_5.7\Engine\Source\Editor\UnrealEd\Private\EditorModeManager.cpp
Engine symbol: FEditorModeTools::MouseMove / MouseLeave / CapturedMouseMove
Observed contract: Lines 1573-1584 route MouseMove to all active legacy modes. Lines 1553-1570 route MouseLeave. Lines 1415-1423 route CapturedMouseMove separately.
Implication for this plugin: The hook will only run while the mode is active; lifecycle remains aligned with SetTrackingModeEnabled/UpdateEditorModeActivation.
```

```text
Engine file/header: C:\Program Files\Epic Games\UE_5.7\Engine\Source\Editor\UnrealEd\Private\LevelEditorViewport.cpp
Engine symbol: FLevelEditorViewportClient::MouseMove / CapturedMouseMove
Observed contract: Line 4730 delegates level viewport movement to FEditorViewportClient::MouseMove; lines 4740-4745 route captured movement to FEditorViewportClient::CapturedMouseMove.
Implication for this plugin: Level editor viewport movement reaches the FEdMode MouseMove path.
```

## 4. Preferred Integration Point

Preferred architecture: start continuous hover tracking in `FLandscapeHeightmapTrackerEdMode::MouseMove`.

Answers:

- Where should continuous hover tracking begin? In `FLandscapeHeightmapTrackerEdMode::MouseMove`.
- Which class should own the viewport hook? `FLandscapeHeightmapTrackerEdMode`, because viewport input already lives there and the mode is activated only when tracking or reverse marker state needs it.
- Should the hook live in `FLandscapeHeightmapTrackerEdMode` or elsewhere? In `FLandscapeHeightmapTrackerEdMode`.
- Should passive movement be event-driven or polled? Event-driven through `MouseMove`.
- Why safer than alternatives? It requires no mouse capture, runs only for active modes, avoids global editor delegates, and can return `false` so normal editor interaction continues.
- How does it fit lifecycle? `SetTrackingModeEnabled` already activates/deactivates the mode; panel destruction disables tracking and clears reverse marker.
- Does it preserve current viewport behavior? Yes if the override returns `false` and only broadcasts hover state.
- What happens when mode inactive? `FEditorModeTools::MouseMove` will not call the inactive mode.

Rejected alternative: `Tick` polling. It could poll `Viewport->GetMouseX/Y` and trace while the mode is active, but it is not inherently tied to motion. It would require deduplication to avoid stationary repeated traces and could run more often than necessary across viewport ticks.

Rejected alternative: `CapturedMouseMove`. Engine comments and routing show it is for movement while input capture is in effect. It would miss ordinary hover.

Rejected alternative: Slate-level mouse move on the heightmap widget or LevelEditor viewport widget. The heightmap widget is not the 3D level viewport, and hooking LevelEditor Slate widgets would add lifecycle and ownership complexity outside the plugin's current EdMode architecture.

## 5. Trace Pipeline Reuse

`TraceLandscapeClick` is semantically click-specific but mechanically reusable.

Evidence:

```text
File: Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Symbol: TraceLandscapeClick
Observed behavior: The method does three separable things: resolve cursor/world/trace/hit, log click-specific messages, and broadcast click-specific delegate.
Architectural implication: Hover should reuse the cursor/trace/hit mechanics but not the click-specific method unchanged.
```

```text
File: Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.cpp
Symbol: FViewportTraceRayBuilder::BuildTraceSegment
Observed behavior: Lines 3-32 validate trace distance and direction; perspective traces origin->origin+direction*distance; orthographic traces centered around origin.
Architectural implication: This helper is already neutral and should be reused for hover.
```

Recommendation: Option B, extract common tracing mechanics into a neutral reusable method while keeping click and hover entry points separate.

Suggested shape:

```text
private:
  struct FViewportLandscapeTraceResult { bool bHit; FVector WorldPosition; TWeakObjectPtr<AActor> HitActor; TWeakObjectPtr<UPrimitiveComponent> HitComponent; FString FailureReason; };
  FViewportLandscapeTraceResult TraceLandscapeUnderCursor(FEditorViewportClient*, FViewport*) const;
```

Then:

- `TraceLandscapeClick` calls `TraceLandscapeUnderCursor` and broadcasts click result only on hit.
- `MouseMove` calls `TraceLandscapeUnderCursor` and broadcasts hover result or clear/no-hit state.

Do not duplicate `GetCursorWorldLocationFromMousePos`, `BuildTraceSegment`, and `LineTraceSingleByChannel` logic.

## 6. Click State vs Hover State

Recommendation: hover and click require separate forward state in the panel.

Reason: current `bHasMarker`/`MarkerUV` represents the last accepted forward click. If hover reuses it directly, then leaving the Landscape or viewport would erase the click marker, and a hover miss could make click behavior feel unstable. Separate state preserves existing click semantics while enabling live hover visualization.

Suggested state model:

- Existing click marker: keep `bHasMarker` and `MarkerUV` as persistent click marker state.
- New hover marker: add `bHasHoverMarker` and `HoverMarkerUV`.
- Rendered image marker: widget attributes should expose "active display marker": hover marker when valid, otherwise click marker.
- Diagnostics: avoid updating `LastMapping` and status text on every hover unless explicitly desired; consider separate `LastHoverMapping` if hover diagnostics are required.

Case behavior:

| Case | Expected behavior |
|---|---|
| A. Mouse over assigned Landscape | Update hover marker UV; rendered image marker follows hover; click marker remains stored. |
| B. Mouse inside viewport but not over assigned Landscape | Clear hover marker; fall back to persistent click marker if one exists. Do not clear reverse marker. |
| C. Mouse leaves viewport | `MouseLeave` clears hover marker; click marker remains. |
| D. Editor Mode becomes inactive | Hover updates stop. Implementation should clear hover marker on panel tracking disabled/destruction or via a hover-clear delegate on mode deactivation if added. |
| E. Panel/widget destroyed while EdMode still exists | Panel unbinds hover delegate; `AddSP`/Remove pattern prevents stale Slate callback. EdMode may continue active only for reverse marker; hover broadcasts have no subscriber. |
| F. Assigned Landscape invalid/removed | Panel `TWeakObjectPtr` fails; hover callback clears hover marker. Existing reverse marker owner weak checks remain independent. |
| G. User clicks Landscape while hover active | Click flow updates persistent click marker and diagnostics. Hover marker may visually remain dominant while cursor is still over Landscape; when hover clears, new click marker is visible. |
| H. User clicks heightmap while hover continues | Reverse marker updates module state; hover marker continues to move image marker. Reverse world marker remains independent and visible in viewport render. |

## 7. Update Frequency and Performance

Expected cost per mouse move:

- One `GetCursorWorldLocationFromMousePos()` and scene view calculation.
- One ray segment build.
- One `LineTraceSingleByChannel` against `ECC_Visibility`.
- One multicast delegate broadcast.
- One `MapWorldPosition` in the panel if hit belongs to assigned Landscape.
- One paint-only invalidation when hover marker changes.

Mapping cost is low: `MapWorldPosition` does one inverse transform and simple bounds/UV/pixel math.

Line trace cost is the main cost. Event-driven `MouseMove` should be tried before rate limiting because it only fires on movement and matches the target behavior. Do not add a timer.

Recommended safeguards:

- Deduplicate viewport coordinates in `FLandscapeHeightmapTrackerEdMode` (`LastHoverMousePos`) so repeated same-position events do not retrace.
- Deduplicate rendered UV in the panel with a small epsilon or exact pixel comparison before invalidating.
- Clear hover only on state transition from valid to invalid/missing, not repeatedly on every miss.
- Return `false` from `MouseMove` so editor viewport interaction remains normal.

Definitions:

- Deduplication: skip work for identical mouse position or unchanged rendered UV.
- Rate limiting: cap updates to a maximum frequency; not initially recommended.
- Throttling: deliberately delay or drop intermediate hover updates; not initially recommended.
- Debounce: wait until movement stops; inappropriate because live marker should move continuously.

If later profiling shows line traces are expensive, rate limiting belongs in the EdMode hover entry point, not in Slate rendering. A conservative strategy would be "no more than one trace per viewport per frame" rather than a wall-clock timer.

## 8. Slate Rendering Behavior

Marker path:

- `SHeightmapTrackerImageView` stores `TAttribute<FVector2D> MarkerUV` and `TAttribute<bool> HasMarker`.
- Panel binds lambdas at `SLandscapeHeightmapTrackerPanel.cpp:247-248`.
- `OnPaint` reads `HasMarker.Get()` and `MarkerUV.Get()` at lines 87 and 89, then draws crosshair lines.

Engine evidence:

```text
Engine file/header: C:\Program Files\Epic Games\UE_5.7\Engine\Source\Runtime\SlateCore\Public\Widgets\SWidget.h
Engine symbol: SWidget::Invalidate / SWidget::OnPaint
Observed contract: Lines 1103-1107 say Invalidate forces redraw/cache on next paint pass; lines 1636-1650 define OnPaint as the draw-element population hook.
Implication for this plugin: Marker-only changes should request paint invalidation, not layout invalidation.
```

```text
Engine file/header: C:\Program Files\Epic Games\UE_5.7\Engine\Source\Runtime\SlateCore\Public\Widgets\InvalidateWidgetReason.h
Engine symbol: EInvalidateWidgetReason::Paint / Layout
Observed contract: Lines 17-25 say Layout is expensive and Paint is for painting changes that do not affect sizing.
Implication for this plugin: Hover marker movement should use `EInvalidateWidgetReason::Paint` if explicit invalidation is needed.
```

Current click flow has no explicit invalidation after marker state writes. For hover, implementation should store a `TSharedPtr<SHeightmapTrackerImageView>` or invalidate the panel/image area and call `Invalidate(EInvalidateWidgetReason::Paint)` only when the displayed marker changes.

Do not use layout invalidation. Marker movement does not affect desired size or child order.

## 9. Editor Mode Lifecycle

Current lifecycle:

- Module startup registers the editor mode at `LandscapeHeightmapTrackerModule.cpp:134-139`.
- Module tab spawner creates `SLandscapeHeightmapTrackerPanel` at lines 169-175.
- Panel construction binds viewport-click delegate and calls `SetTrackingModeEnabled(bTrackClicks)` at lines 176-177.
- `SetTrackingModeEnabled` writes `GIsTrackingModeEnabled` and calls `UpdateEditorModeActivation` at lines 65-69.
- `UpdateEditorModeActivation` activates the mode when tracking or reverse marker state is true, and deactivates when both are false at lines 37-55.
- Panel destructor removes delegate, disables tracking, clears reverse marker, and releases texture at lines 331-336.
- Module shutdown disables tracking, clears reverse marker, clears delegate, unregisters tab/mode/menus at lines 151-164.

Risks and guardrails:

- Stale delegates: use `AddSP` and remove `FDelegateHandle` in panel destructor, matching click binding.
- Destroyed Slate widgets: do not bind raw panel pointers from EdMode; route hover via module multicast and panel `AddSP`.
- Shared pointer lifetime: if storing the image widget pointer for invalidation, keep it as `TSharedPtr` owned by panel or `TWeakPtr` if only used opportunistically.
- Mode transitions: hover hook only runs while mode active. Disabling tracking should clear hover marker state in the panel.
- PIE/map changes/Landscape deletion: assigned Landscape is `TWeakObjectPtr`; hover callback must clear hover marker if invalid. Reverse marker already checks weak owner in `GetReverseMarker`.
- Panel close while reverse marker exists: current destructor clears reverse marker, so mode deactivates unless tracking remains enabled elsewhere.

Do not introduce global raw pointers from the mode to the panel.

## 10. Impact Analysis

Production files likely involved:

| File | Classification | Reason | Expected responsibility / symbols |
|---|---|---|---|
| `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h` | MUST CHANGE | Add `MouseMove` and likely `MouseLeave` overrides; add neutral trace helper declaration and optional mouse-position cache. | `FLandscapeHeightmapTrackerEdMode::MouseMove`, `MouseLeave`, `TraceLandscapeUnderCursor`. |
| `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp` | MUST CHANGE | Implement hover hook, trace extraction, click path reuse, hover broadcast/clear. | `InputKey`, `TraceLandscapeClick`, new trace helper, new hover handling. |
| `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h` | MUST CHANGE | Define hover result delegate separately from click result. | `FViewportHoverResult` or shared trace result, `FOnViewportHoverResult`, `OnViewportHoverResult()`. |
| `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp` | MUST CHANGE | Store and expose hover delegate, clear it on shutdown. | `GOnViewportHoverResult`, `OnViewportHoverResult`, `ShutdownModule`. |
| `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h` | MUST CHANGE | Add hover callback binding handle and hover state; optionally store image widget pointer. | `OnViewportHover`, `ClearHoverMarker`, `bHasHoverMarker`, `HoverMarkerUV`, `HoverDelegateHandle`. |
| `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp` | MUST CHANGE | Bind/unbind hover delegate, map hover world positions, update hover marker state, invalidate paint, choose rendered marker state. | `Construct`, destructor, `OnViewportHover`, marker lambdas, `OnViewportClick`, `ClearMarker`, `AssignLandscape`. |
| `Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.*` | SHOULD NOT CHANGE | Already neutral and tested. | Reuse unchanged unless tests reveal a trace geometry bug. |
| `Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h` and `.cpp` | SHOULD NOT CHANGE | Mapping logic already supports world-to-image and is tested. | Use `MapWorldPosition` unchanged. |
| `Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.*` | SHOULD NOT CHANGE | Reverse surface resolution only; hover forward trace uses viewport ray hit, not vertical reverse trace. | No hover responsibility. |
| `Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.*` | SHOULD NOT CHANGE | Heightmap widget click-to-UV only. | Reverse path only. |
| `Source/LandscapeHeightmapTracker/LandscapeHeightmapTracker.Build.cs` | MAY CHANGE | Existing dependencies already include `UnrealEd`, `Slate`, `SlateCore`, `Landscape`, `InputCore`; likely no change. | Only update if implementation introduces a new dependency, which is not currently recommended. |
| Existing mapper/trace tests | MAY CHANGE | Automated tests can cover extracted trace helper and marker-state pure behavior if factored testably. | Add focused tests only where feasible. |

## Evidence Summary

Graph tools:

- Graphify query surfaced EdMode, panel, module, mapper, trace builder, image widget, and reverse marker candidates.
- CRG exact lookup confirmed known symbols but had no `MouseMove`/`CapturedMouseMove` nodes in plugin, matching source.

Source authority:

- Current plugin does not override `MouseMove` or `CapturedMouseMove`.
- Current click path and reverse path are separate.
- Local UE 5.7 provides `FEdMode::MouseMove` and routes it for ordinary movement.
- Local UE 5.7 provides `CapturedMouseMove`, but only for captured input.
- Slate supports paint invalidation; marker changes are paint-only.
