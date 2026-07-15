# Graphify Analysis - Controlled Rerun

Scope: Q3, Q5, Q6, Q10 only. Graphify was used first for each question, then findings were checked against source. I treated Graphify shortest paths as structural proximity unless source confirmed calls, delegate binding, state writes, or paint reads.

## Q3 - Cursor Acquisition

Graphify found the right candidate area before fallback validation: `FLandscapeHeightmapTrackerEdMode`, `InputKey`, `TraceLandscapeClick`, `FViewportTraceRayBuilder::BuildTraceSegment`, `SLandscapeHeightmapTrackerPanel::OnViewportClick`, and `FLandscapeCoordinateMapper::MapWorldPosition`.

Verified runtime path:

1. Entry point: `FLandscapeHeightmapTrackerEdMode::InputKey` handles `EKeys::LeftMouseButton` + `IE_Pressed`; if `FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled()` is true, it calls `TraceLandscapeClick` (`Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp:14`, `:16`, `:18`, `:20`).
2. Cursor ray acquisition: `TraceLandscapeClick` calls `ViewportClient->GetCursorWorldLocationFromMousePos()` and passes `Cursor.GetOrigin()` / `Cursor.GetDirection()` into `FViewportTraceRayBuilder::BuildTraceSegment` (`LandscapeHeightmapTrackerEdMode.cpp:71-75`).
3. Trace segment derivation: `BuildTraceSegment` validates trace distance/direction, normalizes direction, and derives `Start`/`End`; orthographic viewports use a centered segment around the origin (`Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.cpp:3`, `:20-27`).
4. Landscape hit acquisition: `TraceLandscapeClick` performs `World->LineTraceSingleByChannel(... ECC_Visibility ...)` (`LandscapeHeightmapTrackerEdMode.cpp:105-110`).
5. Result propagation: it copies `Hit.ImpactPoint`, actor, and component into `FViewportClickResult`, then broadcasts `OnViewportClickResult()` (`LandscapeHeightmapTrackerEdMode.cpp:123-127`).
6. Consumer: `SLandscapeHeightmapTrackerPanel::Construct` subscribes with `AddSP` (`Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:176`); `OnViewportClick` validates the assigned Landscape, maps `Click.WorldPosition` through `MapWorldPosition`, and writes marker state (`SLandscapeHeightmapTrackerPanel.cpp:424-454`).

Relationship classification:

- Call relationships: `InputKey -> TraceLandscapeClick`; `TraceLandscapeClick -> BuildTraceSegment`; `TraceLandscapeClick -> LineTraceSingleByChannel`; `TraceLandscapeClick -> Broadcast`; `OnViewportClick -> MapWorldPosition`.
- Dependency relationships: `TraceLandscapeClick` depends on `FEditorViewportClient`, `FViewport`, `UWorld`; panel depends on module delegate and mapper.
- Ownership relationships: module owns the static delegate storage; panel owns marker state and image widget.
- Structural proximity: Graphify links through common types like `FVector2D`, `FIntPoint`, and evaluation docs are not runtime flow.

Failures/misleading points:

- Graphify did not directly report the concrete `GetCursorWorldLocationFromMousePos` and `LineTraceSingleByChannel` lines in its first useful excerpt; source validation was required.
- Graphify surfaced `OnHeightmapClicked`, which is reverse image-to-Landscape behavior, not cursor acquisition from the viewport.
- The graph corpus is contaminated by older evaluation/CRG artifacts; those were ignored.

Usefulness score: 3/5.

## Q5 - Marker Runtime Path

Graphify found the relevant widget, panel, module, mapper, paint, and reverse-mapping helpers, but it blurred click-driven Landscape updates with image-click reverse mapping. Source validation was required to separate them.

Verified click-driven image marker update after Landscape interaction:

1. Panel subscribes to viewport click results with `OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick)` (`SLandscapeHeightmapTrackerPanel.cpp:176`).
2. `FLandscapeHeightmapTrackerEdMode::InputKey` handles left mouse press only, not hover, and calls `TraceLandscapeClick` when tracking is enabled (`LandscapeHeightmapTrackerEdMode.cpp:14-20`).
3. `TraceLandscapeClick` derives the cursor ray, line traces, builds `FViewportClickResult`, and broadcasts it (`LandscapeHeightmapTrackerEdMode.cpp:71-75`, `:110`, `:123-127`).
4. `SLandscapeHeightmapTrackerPanel::OnViewportClick` checks `bTrackClicks`, assigned Landscape, and `IsAssignedLandscapeHit`, then calls `FLandscapeCoordinateMapper::MapWorldPosition` (`SLandscapeHeightmapTrackerPanel.cpp:424-449`).
5. On valid mapping, the image marker state is written: `bHasMarker = true`, `bHasLandscapeUV = false`, `MarkerUV = LastMapping.NormalizedUV` (`SLandscapeHeightmapTrackerPanel.cpp:450-454`).
6. The child widget receives state through Slate attributes: `.MarkerUV_Lambda([this]() { return MarkerUV; })` and `.HasMarker_Lambda([this]() { return bHasMarker; })` (`SLandscapeHeightmapTrackerPanel.cpp:245-248`).
7. `SHeightmapTrackerImageView::OnPaint` reads `HasMarker.Get()` and `MarkerUV.Get()`, computes `MarkerCenter`, and draws crosshair lines (`SLandscapeHeightmapTrackerPanel.cpp:67-90`, `:95-105`).

Hover/mouse-move distinction:

- There is no verified hover or mouse-move marker update path. `rg -n "OnMouseMove|MouseMove|IE_Repeat|IE_Axis|MouseMove|Moved|Hover|OnCursor" Source` returned no matches.
- The runtime path is click-driven through editor mode `InputKey`, not cursor movement.

Reverse image-click path, not the same as Landscape-click image marker update:

- `SHeightmapTrackerImageView::OnMouseButtonDown` maps image-local click position with `FHeightmapImageClickMapper::MapLocalPositionToDisplayUV` and executes `OnHeightmapClicked` (`SLandscapeHeightmapTrackerPanel.cpp:111-131`).
- `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked` maps display UV back to local Landscape coordinates with `MapUVToLocalPosition`, vertical traces the surface, writes diagnostic `LastMapping` fields, and calls `SetReverseMarker` (`SLandscapeHeightmapTrackerPanel.cpp:465-512`).
- `SetReverseMarker` stores module-level reverse marker state and requests viewport redraw (`Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp:76-83`); `FLandscapeHeightmapTrackerEdMode::Render` reads it with `GetReverseMarker` and draws a 3D line (`LandscapeHeightmapTrackerEdMode.cpp:38-55`).
- This reverse path does not set `bHasMarker = true` or assign `MarkerUV`; it is not the source of image marker updates after a Landscape click.

Failures/misleading points:

- Graphify listed `.OnMouseButtonDown`, `OnHeightmapClicked`, `TraceAssignedLandscapeSurface`, and `SetReverseMarker` near the marker path. These are real symbols but belong to the image-to-Landscape reverse path, not the Landscape-click image-marker path.
- Graphify proximity to `.OnPaint()` was useful, but source validation was needed to verify the actual Slate attributes and state reads.
- Any wording from graph output suggesting cursor movement/hover is unsupported by source.

Usefulness score: 2/5.

## Q6 - Actual Callers

Graphify found candidate caller nodes before fallback validation, including `SLandscapeHeightmapTrackerPanel::OnViewportClick` and `FLandscapeCoordinateMapperOptionsTest::RunTest`. However, `graphify explain "FLandscapeCoordinateMapper::MapWorldPosition"` did not show incoming caller edges; it only showed outgoing references and an inferred call from `MapWorldPosition` to `MapLocalPosition`.

Verified direct source search:

```text
Source\LandscapeHeightmapTracker\Public\LandscapeCoordinateMapper.h:44: declaration
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:8: definition
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449: production call
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:59: test call
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:65: test call
```

Actual callers:

- Production caller: `SLandscapeHeightmapTrackerPanel::OnViewportClick`, call at `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:449`.
- Test caller: `FLandscapeCoordinateMapperOptionsTest::RunTest`, calls at `Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp:59` and `:65`.

Not actual callers:

- `LandscapeCoordinateMapper.h:44` is a declaration.
- `LandscapeCoordinateMapper.cpp:8` is the definition.
- `FLandscapeCoordinateMapper::MapLocalPosition` is called by `MapWorldPosition`; it is a callee, not a caller.
- `FLandscapeCoordinateMapperBasicTest::RunTest` and `FLandscapeCoordinateMapperReverseMappingTest::RunTest` appeared in Graphify Q6 output but do not call `MapWorldPosition`; they call other mapper functions.
- `OnHeightmapClicked` is a reverse-mapping consumer and does not call `MapWorldPosition`.

Graphify measurement:

- Found real production caller before fallback validation: yes, as a candidate node (`SLandscapeHeightmapTrackerPanel::OnViewportClick`).
- Found real test caller before fallback validation: partial, as `FLandscapeCoordinateMapperOptionsTest::RunTest` and the test file appeared.
- Found exact caller list before fallback validation: no.
- Found actual call edges into `MapWorldPosition`: no, not in `graphify explain` output.

Usefulness score: 3/5.

## Q10 - Cross-Subsystem Connection

Graphify was useful for candidate discovery and architecture orientation: it found `SHeightmapTrackerImageView`, `SLandscapeHeightmapTrackerPanel`, `OnPaint`, `OnMouseButtonDown`, `OnViewportClick`, `LandscapeHeightmapTrackerModule`, `TraceLandscapeClick`, and mapper helpers. Its shortest paths were not reliable runtime flow.

Verified actual connection from heightmap marker widget to Landscape coordinate acquisition:

- Ownership/structure: `SLandscapeHeightmapTrackerPanel::Construct` creates `SHeightmapTrackerImageView` (`SLandscapeHeightmapTrackerPanel.cpp:245`).
- State connection into widget: the widget receives marker state via `.MarkerUV_Lambda` and `.HasMarker_Lambda` (`SLandscapeHeightmapTrackerPanel.cpp:247-248`).
- Paint usage: `SHeightmapTrackerImageView::OnPaint` reads `MarkerUV` and `HasMarker` attributes to draw the marker (`SLandscapeHeightmapTrackerPanel.cpp:67-90`, `:95-105`).
- Landscape coordinate acquisition does not start from `SHeightmapTrackerImageView`; it starts in editor mode with `InputKey -> TraceLandscapeClick -> GetCursorWorldLocationFromMousePos -> BuildTraceSegment -> LineTraceSingleByChannel -> Broadcast` (`LandscapeHeightmapTrackerEdMode.cpp:14-20`, `:71-75`, `:110`, `:123-127`).
- Cross-subsystem bridge: `FLandscapeHeightmapTrackerModule::OnViewportClickResult` is the delegate bridge; panel subscribes at `SLandscapeHeightmapTrackerPanel.cpp:176`.
- Coordinate mapping bridge: `SLandscapeHeightmapTrackerPanel::OnViewportClick` receives the broadcast, maps `Click.WorldPosition` via `FLandscapeCoordinateMapper::MapWorldPosition`, then writes the marker state that the widget paints (`SLandscapeHeightmapTrackerPanel.cpp:424-454`).

Graphify path evaluation:

- `SHeightmapTrackerImageView -> FVector2D -> OnHeightmapClicked -> FLandscapeTrackerMappingResult -> MapWorldPosition` is structural proximity through shared types/results. It is not runtime execution flow.
- `SHeightmapTrackerImageView -> OnHeightmapClicked <- Construct -> FLandscapeHeightmapTrackerModule -> TraceLandscapeClick` is also not a runtime call path. It identifies the panel/module neighborhood but uses ambiguous symbol matching and references.

Irrelevant or misleading files/symbols:

- `HeightmapImageClickMapper.*`, `OnMouseButtonDown`, `OnHeightmapClicked`, `TraceAssignedLandscapeSurface`, and `SetReverseMarker` are relevant to reverse image-to-Landscape behavior, but not the image marker update after a Landscape viewport click.
- Evaluation documents in the graph were not used.

Additional source reads needed:

- `SLandscapeHeightmapTrackerPanel.cpp` for widget construction, attributes, `OnViewportClick`, `OnHeightmapClicked`, and paint.
- `SLandscapeHeightmapTrackerPanel.h` for owned state fields.
- `LandscapeHeightmapTrackerEdMode.cpp` for viewport input, cursor ray, trace, broadcast, and reverse marker render.
- `LandscapeHeightmapTrackerModule.cpp/.h` for delegate and reverse-marker global state.
- `LandscapeCoordinateMapper.cpp/.h` for coordinate conversion.
- `HeightmapImageClickMapper.cpp` and `LandscapeSurfaceTraceHelper.cpp` to rule in/out the reverse path.

Usefulness score: 2/5.

## Overall

Graphify was useful as a candidate finder but weak as a verifier. The refreshed graph still contains benchmark artifacts, including forbidden CRG/evaluation nodes, so query output required filtering. The most reliable workflow was: Graphify candidate discovery first, then direct source validation for calls, delegates, state writes, Slate attributes, and paint usage.
