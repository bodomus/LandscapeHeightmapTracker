# Phase B Independent Reviewer Validation

Scope: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

Inputs read:

- `evaluation/graphify/raw-results.md`
- `evaluation/graphify/analysis.md`
- `evaluation/graphify/metrics.md`
- `evaluation/crg/crg-results.md`
- `evaluation/crg/raw-results.md`
- `evaluation/crg/analysis.md`
- `evaluation/crg/metrics.md`

Method:

- I did not rerun Graphify or CRG.
- I independently inspected actual project source with line-numbered reads and `rg`.
- I did not modify production code.
- This artifact is the only file written.

Source baseline:

- Current source commit observed by `git rev-parse HEAD`: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`.
- Graphify report says its graph was built at `f17cf66490dc1af63aea15b651ba1fa069b9d69c`, so Graphify line numbers and some relationships must be treated as stale until source-verified.
- CRG report says its graph was built at `93b214fbe2ea`, matching the current checkout.

Classification vocabulary:

- verified: exact file, symbol, and claimed relationship match source.
- partially verified: relevant file/symbol found, but relationship, completeness, line number, or interpretation needs correction.
- misleading: output could reasonably lead to a wrong runtime or dependency conclusion.
- false: source contradicts the claim.
- unverifiable: report did not provide enough concrete evidence and source did not expose the claimed relationship.

Important runtime distinction:

- Graph proximity, shortest paths, communities, hub nodes, and impact-neighborhood edges are not verified runtime execution flow.
- Verified runtime flow requires direct source evidence: direct calls, delegate declaration/broadcast/subscription, Slate attribute binding, widget construction, or Unreal lifecycle registration.

## Source Facts Verified

The plugin is an Editor module in `LandscapeHeightmapTracker.uplugin:17-22`. Runtime editor-facing registration is in `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`:

- `StartupModule()` initializes style/commands, maps `OpenPluginWindow`, registers `FLandscapeHeightmapTrackerEdMode`, registers menus, and registers a Nomad tab spawner at lines 125-149.
- `OnSpawnPluginTab()` creates `SLandscapeHeightmapTrackerPanel` at lines 169-176.
- `RegisterMenus()` adds the Tools menu entry at lines 184-190.
- `IMPLEMENT_MODULE` is at line 195.

The main image widget is the private `SHeightmapTrackerImageView` in `SLandscapeHeightmapTrackerPanel.cpp:40-148`. The public owning panel is `SLandscapeHeightmapTrackerPanel` in `Public/SLandscapeHeightmapTrackerPanel.h:11-71`. The image widget is created in `SLandscapeHeightmapTrackerPanel::Construct()` at `SLandscapeHeightmapTrackerPanel.cpp:245-251`.

The implemented Landscape acquisition path is click-based, not hover or continuous cursor-move:

`FLandscapeHeightmapTrackerEdMode::InputKey()` at `LandscapeHeightmapTrackerEdMode.cpp:14-25`
-> `TraceLandscapeClick()` at `LandscapeHeightmapTrackerEdMode.cpp:58-129`
-> `ViewportClient->GetCursorWorldLocationFromMousePos()` at line 71
-> `FViewportTraceRayBuilder::BuildTraceSegment()` at lines 72-75
-> `World->LineTraceSingleByChannel(...)` at line 110
-> `FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result)` at line 127
-> panel subscription from `SLandscapeHeightmapTrackerPanel.cpp:176`
-> `SLandscapeHeightmapTrackerPanel::OnViewportClick()` at lines 424-463
-> `FLandscapeCoordinateMapper::MapWorldPosition(...)` at line 449
-> `MarkerUV = LastMapping.NormalizedUV` and `bHasMarker = true` at lines 452-454
-> `SHeightmapTrackerImageView::OnPaint()` draws the marker at lines 87-105.

The reverse image-click path is separate:

`SHeightmapTrackerImageView::OnMouseButtonDown()` at `SLandscapeHeightmapTrackerPanel.cpp:111-138`
-> `FHeightmapImageClickMapper::MapLocalPositionToDisplayUV()` at `HeightmapImageClickMapper.cpp:37-62`
-> `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked()` at `SLandscapeHeightmapTrackerPanel.cpp:465-527`
-> `FLandscapeCoordinateMapper::MapUVToLocalPosition()` at line 485
-> `LandscapeTransform.TransformPosition(...)` at lines 492-493
-> `FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface()` at line 495
-> `FLandscapeHeightmapTrackerModule::SetReverseMarker()` at line 512
-> `FLandscapeHeightmapTrackerEdMode::Render()` draws the editor viewport reverse marker at `LandscapeHeightmapTrackerEdMode.cpp:38-56`.

## Per-Question Validation

### Q1. Where does the plugin initialize and register editor-facing functionality?

Verified answer:

- Editor module metadata: `LandscapeHeightmapTracker.uplugin:17-22`.
- Main module class: `FLandscapeHeightmapTrackerModule`, `Public/LandscapeHeightmapTrackerModule.h:9-41`.
- `StartupModule()`: `Private/LandscapeHeightmapTrackerModule.cpp:125-149`.
- Static identifiers: `PluginTabName` and `EditorModeId`, `LandscapeHeightmapTrackerModule.cpp:17-18`, declared at `LandscapeHeightmapTrackerModule.h:21-22`.
- Menu registration: `RegisterMenus()`, `LandscapeHeightmapTrackerModule.cpp:184-190`.
- Tab content creation: `OnSpawnPluginTab()`, `LandscapeHeightmapTrackerModule.cpp:169-176`.
- Shutdown unregisters menus, tab spawner, mode, commands, and style at `LandscapeHeightmapTrackerModule.cpp:151-166`.

Graphify validation:

- Correct files: verified for module cpp/h; partially verified for command/style files as supporting registration.
- Correct symbols: verified for `StartupModule`, `RegisterMenus`, `PluginTabName`, `EditorModeId`, `OnSpawnPluginTab`.
- Missed important files: partially verified; `.uplugin` was not emphasized as module metadata.
- False positives: partially verified; README/build/doc nodes are architectural context, not registration implementation.
- Misleading relationships: misleading; `Initialize --calls --> StartupModule` is directionally wrong for source, where `StartupModule()` calls `FLandscapeHeightmapTrackerStyle::Initialize()`.
- Additional source reads/searches needed: module cpp/h, `.uplugin`, `rg` for `RegisterMode`, `RegisterNomadTabSpawner`, `RegisterMenus`.
- Output/context volume: medium.
- Usefulness: 4/5.

CRG validation:

- Correct files: verified.
- Correct symbols: verified for exact `StartupModule`; partially verified for broader lifecycle because exact search did not itself enumerate all registration code.
- Missed important files: partially verified; `.uplugin` and Build.cs not surfaced, but runtime registration is in the module.
- False positives: none material for exact lookup.
- Misleading relationships: partially verified; later impact output overincluded module startup for mapper changes.
- Additional source reads/searches needed: module cpp/h and registration `rg`.
- Output/context volume: low for exact lookup.
- Usefulness: 4/5.

### Q2. Which class implements the main heightmap visualization widget, and what code creates or owns it?

Verified answer:

- Private visualization widget: `SHeightmapTrackerImageView`, `SLandscapeHeightmapTrackerPanel.cpp:40-148`.
- It draws the heightmap and marker in `OnPaint()`, lines 67-108.
- It maps image clicks in `OnMouseButtonDown()`, lines 111-138.
- Owner panel: `SLandscapeHeightmapTrackerPanel`, header lines 11-71.
- The panel creates the image widget via `SNew(SHeightmapTrackerImageView)` at `SLandscapeHeightmapTrackerPanel.cpp:245-251`.
- The module creates the panel in `OnSpawnPluginTab()` at `LandscapeHeightmapTrackerModule.cpp:169-176`.

Graphify validation:

- Correct files: verified.
- Correct symbols: verified for `SLandscapeHeightmapTrackerPanel`, `SHeightmapTrackerImageView`, `Construct`, `OnPaint`, `OnMouseButtonDown`.
- Missed important files: partially verified; module ownership chain needed source confirmation.
- False positives: partially verified; surface trace helper is unrelated to widget ownership.
- Misleading relationships: partially verified; proximity around `SCompoundWidget` broadened output but did not prove ownership.
- Additional source reads/searches needed: panel cpp/h, module cpp, `rg` for `SNew(...)`.
- Output/context volume: medium-high.
- Usefulness: 4/5.

CRG validation:

- Correct files: verified.
- Correct symbols: verified; exact `SLandscapeHeightmapTrackerPanel` query found `SHeightmapTrackerImageView`, `Construct`, `OnViewportClick`, and `OnHeightmapClicked`.
- Missed important files: partially verified; module tab ownership was not connected as a runtime ownership path by CRG flows.
- False positives: partially verified; header `class` pseudo-functions are parser artifacts.
- Misleading relationships: partially verified; CRG located pieces but did not model the module-tab-to-panel-to-child-widget chain.
- Additional source reads/searches needed: panel cpp/h, module cpp.
- Output/context volume: medium.
- Usefulness: 4/5.

### Q3. Which code reads or derives the current cursor position over the Unreal Landscape?

Verified answer:

- `FLandscapeHeightmapTrackerEdMode::InputKey()` handles left mouse press at `LandscapeHeightmapTrackerEdMode.cpp:14-25`.
- `TraceLandscapeClick()` reads the cursor world location/ray with `ViewportClient->GetCursorWorldLocationFromMousePos()` at line 71.
- It derives a valid trace segment with `FViewportTraceRayBuilder::BuildTraceSegment()` at lines 72-75; implementation is in `ViewportTraceRayBuilder.cpp:3-37`.
- It traces with `World->LineTraceSingleByChannel(...)` at `LandscapeHeightmapTrackerEdMode.cpp:110`.
- It stores `Hit.ImpactPoint` in `FViewportClickResult::WorldPosition` at line 124 and broadcasts at line 127.

Graphify validation:

- Correct files: partially verified; ed mode and panel/mapping downstream appeared, but `ViewportTraceRayBuilder.cpp` was not prominent.
- Correct symbols: partially verified; `InputKey`, `TraceLandscapeClick`, `OnViewportClick`, and `MapWorldPosition` are relevant, but mapper symbols are downstream rather than acquisition.
- Missed important files: verified; `ViewportTraceRayBuilder.cpp/h` matters for deriving the trace segment.
- False positives: partially verified; coordinate mapper nodes are not cursor acquisition.
- Misleading relationships: partially verified; Graphify did not capture external Unreal API `GetCursorWorldLocationFromMousePos` as a node.
- Additional source reads/searches needed: ed mode cpp/h, trace ray builder, `rg` for cursor/trace terms.
- Output/context volume: medium.
- Usefulness: 3/5.

CRG validation:

- Correct files: partially verified; exact `TraceLandscapeClick` lookup worked, but full path and trace builder required source reads.
- Correct symbols: partially verified.
- Missed important files: partially verified; trace ray builder was not a direct result of cursor query.
- False positives: none material.
- Misleading relationships: partially verified; `InputKey` flow was too shallow to prove `InputKey -> TraceLandscapeClick -> Broadcast`.
- Additional source reads/searches needed: ed mode cpp/h, trace ray builder, `rg` for `GetCursorWorldLocationFromMousePos`, `LineTraceSingleByChannel`, `Broadcast`.
- Output/context volume: low-medium.
- Usefulness: 3/5.

### Q4. How does world-space or local-space Landscape position become heightmap image or pixel coordinates?

Verified answer:

- `MapWorldPosition()` in `LandscapeCoordinateMapper.cpp:8-23` converts world to local via `LandscapeTransform.InverseTransformPosition(WorldPosition)` at line 17, then delegates to `MapLocalPosition()`.
- `MapLocalPosition()` in lines 25-82 validates bounds/image size, computes width/height from `LocalBounds.Max - LocalBounds.Min`, normalizes X/Y into U/V, checks tolerance, clamps, applies `bFlipX`/`bFlipY`, writes `NormalizedUV`, and calls `UVToPixel()`.
- `UVToPixel()` in lines 134-139 maps inclusive pixel endpoints with `RoundToInt(UV * (ImageSize - 1))`, then clamps.
- Reverse mapping is `MapUVToLocalPosition()` in lines 84-132: it validates/clamps display UV, applies inverse flips, and computes `LocalBounds.Min + U/V * Width/Height`.

Graphify validation:

- Correct files: verified.
- Correct symbols: verified for mapper functions and image click mapper adjacency.
- Missed important files: partially verified; formulas required direct source reads.
- False positives: partially verified; docs/module nodes in broad output were not formula evidence.
- Misleading relationships: none accepted as runtime flow without source verification.
- Additional source reads/searches needed: mapper cpp/h, panel call sites, exact conversion-name `rg`.
- Output/context volume: medium.
- Usefulness: 4/5.

CRG validation:

- Correct files: verified.
- Correct symbols: verified for exact mapper lookups.
- Missed important files: partially verified; panel was needed to see production inputs.
- False positives: partially verified; header `class` pseudo-function artifacts.
- Misleading relationships: partially verified; local CRG flow is useful but flattens branch/data transformation semantics.
- Additional source reads/searches needed: mapper cpp/h, panel call sites.
- Output/context volume: medium.
- Usefulness: 4/5.

### Q5. Trace the actual verified code path responsible for updating the visual marker when the cursor moves over the Landscape.

Verified answer:

- The question premise is false for current source if "moves" means hover/continuous cursor movement. The verified implementation updates on tracked left mouse click.
- Tracking enabled state is stored in module static state and toggled through `SetTrackingModeEnabled()` at `LandscapeHeightmapTrackerModule.cpp:65-69`; the panel subscribes to the click result delegate at `SLandscapeHeightmapTrackerPanel.cpp:176` and sets initial tracking at line 177.
- Runtime path:
  - `InputKey()` left click: `LandscapeHeightmapTrackerEdMode.cpp:14-25`.
  - `TraceLandscapeClick()` cursor ray and hit: lines 58-127.
  - Delegate broadcast: line 127.
  - Panel `OnViewportClick()` validates assigned landscape and hit actor/component at `SLandscapeHeightmapTrackerPanel.cpp:424-442`.
  - Bounds refresh and mapping: lines 444-449.
  - Marker state update: lines 452-454.
  - Image widget paint consumes `MarkerUV` and `HasMarker`: lines 87-105, with lambdas bound at lines 245-248.

Graphify validation:

- Correct files: partially verified across broader Q10, not direct Q5.
- Correct symbols: partially verified; Q5 output emphasized `OnHeightmapClicked()` and `TraceAssignedLandscapeSurface`, which are reverse path symbols.
- Missed important files: verified for direct Q5; ed mode and trace ray builder were missed or underweighted.
- False positives: verified; reverse image-click path is not the viewport-click marker update path.
- Misleading relationships: misleading; report included `TraceAssignedLandscapeSurface --calls --> OnHeightmapClicked`, but source shows `OnHeightmapClicked()` calls `TraceAssignedLandscapeSurface()`.
- Additional source reads/searches needed: ed mode, module, panel, trace builder, marker/cursor/delegate `rg`.
- Output/context volume: high.
- Usefulness: 2/5.

CRG validation:

- Correct files: partially verified; it found pieces separately.
- Correct symbols: partially verified.
- Missed important files: verified; no single CRG flow modeled delegate broadcast/subscription plus Slate attribute repaint.
- False positives: partially verified; `SetReverseMarker` flow is reverse path, not forward marker path.
- Misleading relationships: partially verified; top flows could lead reviewers toward the reverse path.
- Additional source reads/searches needed: all path files and exact marker/delegate search.
- Output/context volume: high.
- Usefulness: 2/5.

### Q6. Find all actual callers of the main coordinate-conversion function.

Verified answer:

Treating `FLandscapeCoordinateMapper::MapWorldPosition()` as the main world-to-heightmap conversion:

- Definition: `LandscapeCoordinateMapper.cpp:8`.
- Production caller: `SLandscapeHeightmapTrackerPanel::OnViewportClick()` at `SLandscapeHeightmapTrackerPanel.cpp:449`.
- Test callers: `LandscapeCoordinateMapperTests.cpp:59` and `LandscapeCoordinateMapperTests.cpp:65`.

If treating all mapper conversions as in scope:

- `MapLocalPosition()` is called internally by `MapWorldPosition()` at `LandscapeCoordinateMapper.cpp:15`, and by tests at `LandscapeCoordinateMapperTests.cpp:15,22,26,43,48,53,69,73`.
- `MapUVToLocalPosition()` is called by `OnHeightmapClicked()` at `SLandscapeHeightmapTrackerPanel.cpp:485`, and tests at `LandscapeCoordinateMapperTests.cpp:88,93,94,95,96,99,105,111,117,118`.
- `UVToPixel()` is called by `MapLocalPosition()` at `LandscapeCoordinateMapper.cpp:79` and by `OnHeightmapClicked()` at `SLandscapeHeightmapTrackerPanel.cpp:508`.

Graphify validation:

- Correct files: partially verified; mapper, panel, and tests surfaced.
- Correct symbols: partially verified; it identified likely test classes and production handler but did not enumerate caller sites precisely.
- Missed important files: partially verified; exact caller completeness needed `rg`.
- False positives: partially verified; some nearby test nodes did not call `MapWorldPosition`.
- Misleading relationships: partially verified; BFS proximity is not caller analysis.
- Additional source reads/searches needed: exact `rg` for function names.
- Output/context volume: medium.
- Usefulness: 3/5.

CRG validation:

- Correct files: partially verified only through source verification.
- Correct symbols: partially verified for definitions.
- Missed important files: verified; rename preview missed the real qualified static calls in panel and tests.
- False positives: none material in caller list, but omission is severe.
- Misleading relationships: misleading for caller discovery; rename preview looked precise but was incomplete.
- Additional source reads/searches needed: exact `rg` for function names.
- Output/context volume: low but insufficient.
- Usefulness: 1/5.

### Q7. Estimate the impact radius of changing coordinate-conversion semantics.

Verified answer:

Direct impact:

- `Public/LandscapeCoordinateMapper.h`: public structs/options/API.
- `Private/LandscapeCoordinateMapper.cpp`: core formulas and pixel conversion.
- `Private/SLandscapeHeightmapTrackerPanel.cpp`: forward marker update via `OnViewportClick()` at lines 424-463; reverse mapping via `OnHeightmapClicked()` at lines 465-527; diagnostics from `LastMapping` at lines 685-706.
- `Private/Tests/LandscapeCoordinateMapperTests.cpp`: direct expectations for UV, pixel, transform, bounds, flip, and reverse semantics.

Adjacent/conditional impact:

- `Private/HeightmapImageClickMapper.cpp`: display UV must remain compatible with mapper display UV.
- `Private/Tests/HeightmapImageClickMapperTests.cpp`: sensitive if display UV contract changes.
- `Private/LandscapeSurfaceTraceHelper.cpp`: affected only if reverse mapping changes world XY interpretation.

Likely not directly affected:

- module startup/menu/style code;
- viewport trace ray math, unless the meaning of input world position changes;
- reverse marker rendering height line, unless marker world point semantics change.

Graphify validation:

- Correct files: partially verified; mapper, panel, and tests surfaced.
- Correct symbols: partially verified.
- Missed important files: partially verified; image click mapper and surface trace helper required source reasoning.
- False positives: verified; `ViewportTraceRayBuilderTests.cpp` is not directly sensitive to coordinate conversion semantics.
- Misleading relationships: partially verified; graph proximity overstated trace-ray impact.
- Additional source reads/searches needed: mapper, panel, helper files, tests, exact call-site search.
- Output/context volume: medium-high.
- Usefulness: 3/5.

CRG validation:

- Correct files: partially verified; CRG impact included many relevant files.
- Correct symbols: partially verified.
- Missed important files: partially verified; relevant files were mixed with noise.
- False positives: verified; impact output overincluded startup/style/menu and other non-runtime mapper blast-radius nodes.
- Misleading relationships: misleading; `detect_changes` called many impacted nodes "changed" and contradicted `get_review_context` for mapper cpp only.
- Additional source reads/searches needed: exact callers and source filtering.
- Output/context volume: very high.
- Usefulness: 3/5.

### Q8. Find code sensitive to Landscape translation, rotation, or non-unit scale.

Verified answer:

- Forward mapping is transform-sensitive at `LandscapeCoordinateMapper.cpp:17` through `LandscapeTransform.InverseTransformPosition(WorldPosition)`.
- Production caller passes `AssignedLandscape->GetActorTransform()` at `SLandscapeHeightmapTrackerPanel.cpp:449`.
- Reverse path applies `LandscapeTransform.TransformPosition(ReverseMapping.LocalPosition)` at `SLandscapeHeightmapTrackerPanel.cpp:492-493`.
- Bounds come from `AssignedLandscape->GetBoundingRect()` at `SLandscapeHeightmapTrackerPanel.cpp:579-581`.
- UI displays actor location, rotation, and scale at `SLandscapeHeightmapTrackerPanel.cpp:679-681`.
- Tests cover translated/scaled and rotated transforms at `LandscapeCoordinateMapperTests.cpp:57-67`.

Graphify validation:

- Correct files: partially verified; mapper, panel, and tests surfaced.
- Correct symbols: partially verified; UI getters are display-only, while mapper/transform lines are executable mapping logic.
- Missed important code: verified; exact `InverseTransformPosition`, `TransformPosition`, and transform tests required source search.
- False positives: partially verified; display getters are not mapping semantics.
- Misleading relationships: partially verified; broad query mixed executable code and contextual docs/tests.
- Additional source reads/searches needed: mapper, panel, tests, transform API `rg`.
- Output/context volume: medium.
- Usefulness: 3/5.

CRG validation:

- Correct files: partially verified.
- Correct symbols: partially verified.
- Missed important code: verified; CRG did not directly identify transform-sensitive APIs.
- False positives: partially verified; `ComputeDesiredSize(float LayoutScaleMultiplier)` from source grep is UI layout scale, not Landscape scale.
- Misleading relationships: partially verified; CRG cannot distinguish data sensitivity from general call proximity.
- Additional source reads/searches needed: mapper, panel, tests, transform API `rg`.
- Output/context volume: medium.
- Usefulness: 2/5.

### Q9. Find dimensional assumptions involving width, height, resolution, vertex count, and component size.

Verified answer:

Coordinate dimensions:

- Bounds valid only if `Max.X > Min.X && Max.Y > Min.Y`: `LandscapeCoordinateMapper.cpp:3-6`.
- Width/height from `LocalBounds.Max - LocalBounds.Min`: `LandscapeCoordinateMapper.cpp:46-49`.
- Image size must be positive: `LandscapeCoordinateMapper.cpp:40-44`.
- Pixel indices use inclusive `ImageSize - 1`: `LandscapeCoordinateMapper.cpp:134-138`.

Landscape dimensions:

- `RefreshLandscapeBounds()` uses `AssignedLandscape->GetBoundingRect()` and maps rect min/max directly to local XY bounds at `SLandscapeHeightmapTrackerPanel.cpp:571-581`.
- Source search found no actual `ComponentSize`, `Subsection`, `SectionBase`, `Vertex`, or resolution metadata usage in plugin source.

Image/widget dimensions:

- `SHeightmapTrackerImageView::ComputeDesiredSize()` returns `512 x 512` at `SLandscapeHeightmapTrackerPanel.cpp:62-64`.
- Slate slot has `MinHeight(320.0f)` at `SLandscapeHeightmapTrackerPanel.cpp:240`.
- `FHeightmapImageClickMapper::CalculateFittedImageRect()` preserves aspect ratio using image and widget width/height at `HeightmapImageClickMapper.cpp:3-34`.
- Clicks are outside only when relative X/Y are `< 0` or `> DrawSize`, so exact bottom/right edge is accepted at `HeightmapImageClickMapper.cpp:49-59`.
- PNG width/height become `ImageSize` and texture/brush dimensions at `SLandscapeHeightmapTrackerPanel.cpp:627-643`.

Graphify validation:

- Correct files: verified.
- Correct symbols: mostly verified for mapper, image click mapper, panel, tests.
- Missed important files: partially verified; absence of component-size logic required source search.
- False positives: partially verified; collision hit/component nodes are not dimensional mapping assumptions.
- Misleading relationships: partially verified; BFS mixed collision dimensions with image/landscape dimensions.
- Additional source reads/searches needed: mapper, image click mapper, panel, tests, dimension keyword `rg`.
- Output/context volume: medium-high.
- Usefulness: 4/5.

CRG validation:

- Correct files: partially verified; found mapper and image-click mapper areas but not all assumptions.
- Correct symbols: partially verified.
- Missed important files: partially verified; header field defaults and tests needed source reads.
- False positives: partially verified; header `class` pseudo-functions and parser artifacts obscure type-level assumptions.
- Misleading relationships: partially verified; no semantic distinction for vertex/component-size absence.
- Additional source reads/searches needed: mapper, image click mapper, panel, tests, dimension keyword `rg`.
- Output/context volume: high.
- Usefulness: 2/5.

### Q10. Starting from the marker widget, identify the connection to Landscape coordinate acquisition.

Verified answer:

Forward display path:

1. `SHeightmapTrackerImageView::OnPaint()` draws marker from `MarkerUV` and `HasMarker` attributes at `SLandscapeHeightmapTrackerPanel.cpp:87-105`.
2. `SLandscapeHeightmapTrackerPanel::Construct()` binds those attributes to panel fields at lines 245-248.
3. The panel subscribes to the module click delegate at line 176.
4. The editor mode acquires the Landscape click with `InputKey()` and `TraceLandscapeClick()` at `LandscapeHeightmapTrackerEdMode.cpp:14-127`.
5. The module delegate broadcasts `FViewportClickResult` at `LandscapeHeightmapTrackerEdMode.cpp:127`.
6. `OnViewportClick()` maps `Click.WorldPosition` to UV and updates `MarkerUV` at `SLandscapeHeightmapTrackerPanel.cpp:449-454`.

Reverse widget-originated path:

1. `SHeightmapTrackerImageView::OnMouseButtonDown()` maps the widget-local click to display UV at `SLandscapeHeightmapTrackerPanel.cpp:111-138`.
2. `OnHeightmapClicked()` maps display UV to local Landscape XY at `SLandscapeHeightmapTrackerPanel.cpp:465-527`.
3. `TraceAssignedLandscapeSurface()` resolves surface Z and `SetReverseMarker()` stores the world marker.

Graphify validation:

- Correct files/symbols: partially verified; Q10 BFS included the right pieces.
- Missed important code: verified; `GetCursorWorldLocationFromMousePos` was absent as a graph node.
- False positives: partially verified; `OnUnavailableClicked` appeared in a shortest path but is not part of marker-to-acquisition runtime flow.
- Misleading relationships: misleading; the shortest path from `MarkerUV` to `TraceLandscapeClick` was structural proximity only, not executable control/data flow.
- Additional source reads/searches needed: panel, ed mode, module, mapper, marker/cursor/delegate `rg`.
- Output/context volume: high.
- Usefulness: 3/5.

CRG validation:

- Correct files/symbols: partially verified; CRG found widget, panel handlers, ed mode trace, mapper, and surface trace separately.
- Missed important relationships: verified; it did not connect delegate broadcast/subscription or Slate lambdas into a complete runtime path.
- False positives: partially verified; `SetReverseMarker` alone does not establish widget origin.
- Misleading relationships: partially verified; flows are same-file/local and do not prove cross-file Unreal event flow.
- Additional source reads/searches needed: panel, ed mode, module, mapper, surface trace helper.
- Output/context volume: high.
- Usefulness: 3/5.

## Dimension Review

### Architecture Discovery

Graphify: 4/5. It was strong for finding architectural regions: module, panel, mapper, editor mode, image click mapper, surface trace helper, docs, and tests. Weakness: stale graph and broad proximity.

CRG: 4/5. Exact symbol lookup and hub/bridge lists were efficient for current-source orientation. Weakness: graph indexed fewer files and omitted docs/plugin metadata context.

### Exact Symbol Lookup

Graphify: 3/5. It found many symbols but stale line numbers and external Unreal APIs missing from graph reduce confidence.

CRG: 4/5. Exact named lookup worked well for `StartupModule`, `SLandscapeHeightmapTrackerPanel`, `TraceLandscapeClick`, and `MapLocalPosition`.

### Caller/Callee Correctness

Graphify: 2/5. Useful for candidate neighborhoods, but BFS/paths are not caller lists. It produced at least one reversed/misleading relationship around `TraceAssignedLandscapeSurface` and `OnHeightmapClicked`.

CRG: 2/5. Local mapper flow was useful, but rename preview missed actual qualified static callers of `MapWorldPosition` and `MapUVToLocalPosition`.

### Dependency Analysis

Graphify: 3/5. Good for broad dependency/context discovery but mixes docs, tests, and runtime code.

CRG: 3/5. Hubs/bridges/communities helped orientation, but parser artifacts and overbroad import/call edges limited precision.

### Impact Analysis

Graphify: 3/5. Suggested most relevant areas but overstated trace-ray/test adjacency.

CRG: 3/5. Impact radius found relevant regions, but output was noisy and internally inconsistent: high-risk broad impact for mapper cpp/h versus low-risk/zero-impact review context for mapper cpp only.

### Cross-File Exploration

Graphify: 3/5. Stronger at broad cross-file exploration, weaker at distinguishing executable flow from graph proximity.

CRG: 2/5. Found cross-file pieces, but flows did not model Unreal delegates, tab spawners, or Slate attributes.

### Debugging Usefulness

Graphify: 3/5. Good initial map, but every runtime/debugging conclusion required source verification.

CRG: 3/5. Good for exact symbol navigation and local mapper flows, poor for event-driven runtime paths.

### Context Efficiency

Graphify: 3/5. Rich output, but BFS results often had medium-high volume and stale/noisy context.

CRG: 4/5 for exact lookup, 2/5 for impact. Exact queries were concise; impact/community outputs were large and noisy.

### Unreal C++ Parsing Quality

Graphify: 2/5. Missed external Unreal APIs as nodes and had stale line numbers; some call direction/proximity issues.

CRG: 2/5. Headers sometimes parsed as `c`; classes appeared as pseudo-functions named `class`; qualified static calls were missed by rename/caller proxy; delegates and Slate lambdas were not modeled as runtime flow.

### False Positives

Graphify:

- Reverse path (`OnHeightmapClicked`, `TraceAssignedLandscapeSurface`) surfaced as if it answered forward marker update.
- `ViewportTraceRayBuilderTests.cpp` appeared in mapper impact context despite not being directly sensitive to conversion semantics.
- `OnUnavailableClicked` appeared in marker-to-trace shortest path but is not runtime marker acquisition flow.
- README/docs/build nodes appeared as nearby context for runtime questions.

CRG:

- Header `class` pseudo-function nodes.
- Impact radius overincluded startup/style/menu nodes for mapper semantics.
- `SetReverseMarker`/reverse path flows can distract from forward marker path.
- `ComputeDesiredSize(float LayoutScaleMultiplier)` is UI layout scale, not Landscape transform scale.

### False Negatives

Graphify:

- Did not expose `GetCursorWorldLocationFromMousePos` as a node.
- Direct Q5 missed or underweighted ed mode and trace ray builder.
- Did not provide complete actual caller lists.
- Did not directly establish `SNew(SLandscapeHeightmapTrackerPanel) -> SNew(SHeightmapTrackerImageView)` ownership chain.

CRG:

- Multi-term search returned zero for relevant concepts due no embeddings.
- Rename preview missed `SLandscapeHeightmapTrackerPanel::OnViewportClick()` and tests calling `MapWorldPosition`.
- Rename preview missed panel/tests calling `MapUVToLocalPosition`.
- Flows missed `InputKey -> TraceLandscapeClick -> Broadcast -> OnViewportClick`.
- Flows missed tab spawner/panel/widget creation and Slate attribute relationships.

### Misleading Graph Relationships

Graphify:

- `Initialize --calls --> StartupModule` is directionally wrong against source.
- `TraceAssignedLandscapeSurface --calls --> OnHeightmapClicked` is reversed against source.
- Shortest path from `MarkerUV` to `TraceLandscapeClick` is graph proximity, not runtime flow.

CRG:

- Impact neighborhoods and `detect_changes` mix impacted and changed entities.
- Same-file flow output can look like a complete runtime path but omits delegate/Slate/module edges.
- Rename preview can look like precise caller discovery while missing qualified static C++ calls.

## Comparative Scores

| Dimension | Graphify | CRG | Reviewer assessment |
|---|---:|---:|---|
| Architecture discovery | 4 | 4 | Both useful; Graphify broader, CRG fresher and tighter. |
| Exact symbol lookup | 3 | 4 | CRG wins for current exact symbol lookup. |
| Caller/callee correctness | 2 | 2 | Both require `rg`; CRG rename preview was especially incomplete for qualified static calls. |
| Dependency analysis | 3 | 3 | Both orient, neither is proof of runtime dependency. |
| Impact analysis | 3 | 3 | Both noisy; source filtering required. |
| Cross-file exploration | 3 | 2 | Graphify explores more broadly, but paths require verification. |
| Debugging usefulness | 3 | 3 | Both help find starting points, not final answers. |
| Context efficiency | 3 | 3 | CRG exact lookup is efficient; Graphify can answer richer questions but with more noise. |
| Unreal C++ parsing quality | 2 | 2 | Both struggle with Unreal idioms: delegates, Slate lambdas, external APIs, qualified static calls. |
| False positive control | 2 | 2 | Both have notable misleading neighborhoods/flows. |
| False negative control | 2 | 2 | Both miss important runtime/caller relationships. |

Overall benchmark usefulness:

- Graphify: 3/5.
- CRG: 3/5.

## Evidence-Based Recommendation

Allowed recommendation selected: task-based routing between both tools.

No single-tool winner is supported by the evidence. Graphify was better for broad architectural discovery and finding likely file clusters, but its graph was stale, undirected, and sometimes misleading for runtime flow. CRG was better for current exact symbol lookup and local same-file mapper flows, but it missed actual qualified C++ callers and did not model Unreal delegates or Slate attribute/data flow. Both tools required direct source reads and exact `rg` searches for benchmark-grade answers.

Routing guidance for this codebase:

- Use CRG first for exact symbol lookup when the symbol name is known, for example `StartupModule`, `TraceLandscapeClick`, `MapLocalPosition`, `SLandscapeHeightmapTrackerPanel`.
- Use Graphify first for broad architecture discovery, unfamiliar feature orientation, and "what files should I inspect" questions, especially when docs and source context both matter.
- Use neither tool alone for actual caller lists. Use exact source search (`rg`) and line-numbered reads for `MapWorldPosition`, `MapUVToLocalPosition`, delegates, and Slate callbacks.
- Use neither tool alone for verified runtime flow in Unreal editor code. Delegate broadcasts/subscriptions, editor mode lifecycle, tab spawning, and Slate lambdas must be verified in source.
- Use CRG impact and Graphify proximity as candidate expansion only, then manually classify direct, adjacent, and false-positive impact.
- For transform and dimensional semantics, route to direct source inspection after either tool identifies the mapper/panel files; semantic claims about coordinate systems require formula-level verification.

Final reviewer conclusion:

Task-based routing between both tools is the defensible recommendation. CRG should be the primary entry point for exact current symbols and local flows. Graphify should be the primary entry point for broader architecture discovery and cross-file candidate discovery. For caller/callee correctness, impact radius, and Unreal runtime execution flow, both tools are subordinate to direct source verification.
