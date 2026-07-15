# Graphify Analysis

Graphify was useful as a map of likely files and symbols, but the existing graph was stale (`built_at_commit=f17cf664...`, current `HEAD=93b214f...`) and several outputs were structural proximity rather than runtime flow. Every answer below is source-verified against the current checkout.

Relationship vocabulary used below:

- Call relationship: verified direct function call or delegate broadcast/subscription in source.
- Ownership relationship: widget/member/module constructs, stores, subscribes, or owns another object.
- Dependency relationship: include/module/API dependency or shared type dependency.
- Structural graph proximity: Graphify shortest path/BFS neighborhood only; not accepted as execution flow unless source-verified.

## Q1. Plugin Initialization and Editor-Facing Registration

Verified answer:

- `FLandscapeHeightmapTrackerModule` is declared in `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h:9`.
- Static IDs are `PluginTabName` and `EditorModeId` in `LandscapeHeightmapTrackerModule.h:21-22`, defined in `LandscapeHeightmapTrackerModule.cpp:17-18`.
- `FLandscapeHeightmapTrackerModule::StartupModule()` in `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp:125` initializes style/commands, maps `OpenPluginWindow`, registers `FLandscapeHeightmapTrackerEdMode`, registers `RegisterMenus`, and registers a Nomad tab spawner at `LandscapeHeightmapTrackerModule.cpp:129-148`.
- `RegisterMenus()` adds the Tools menu entry at `LandscapeHeightmapTrackerModule.cpp:184-190`.
- `OnSpawnPluginTab()` creates the panel with `SNew(SLandscapeHeightmapTrackerPanel)` at `LandscapeHeightmapTrackerModule.cpp:169-176`.
- `ShutdownModule()` unregisters menus, tab spawner, editor mode, commands, and style at `LandscapeHeightmapTrackerModule.cpp:151-166`.

Graphify quality:

- Correct files: `LandscapeHeightmapTrackerModule.cpp`, `LandscapeHeightmapTrackerModule.h`, command/style headers.
- Correct symbols: `StartupModule`, `RegisterMenus`, `PluginTabName`, `EditorModeId`, `OnSpawnPluginTab`.
- Missed/weak: Graphify reported `StartupModule()` at line `L110`; verified current line is `125` due stale graph.
- False positives: README/build nodes nearby but not registration code.
- Misleading relationships: `Initialize --calls --> StartupModule` is directionally suspicious; source says `StartupModule()` calls `FLandscapeHeightmapTrackerStyle::Initialize()`.
- Additional source reads: module `.cpp` and `.h`.
- Additional searches: registration/menu/spawner `rg`.
- Approx output/context volume: medium.
- Usefulness: 4/5.

## Q2. Main Heightmap Visualization Widget and Ownership

Verified answer:

- Public panel class: `SLandscapeHeightmapTrackerPanel` in `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h:11`.
- Internal image visualization widget: anonymous-namespace `SHeightmapTrackerImageView : public SCompoundWidget` in `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:40`.
- It owns Slate attributes/events for `ImageBrush`, `MarkerUV`, `HasMarker`, and image click callbacks at `SLandscapeHeightmapTrackerPanel.cpp:43-59`.
- It draws the heightmap and marker in `OnPaint()` at `SLandscapeHeightmapTrackerPanel.cpp:67-108`.
- It maps image-widget clicks in `OnMouseButtonDown()` at `SLandscapeHeightmapTrackerPanel.cpp:111-138`.
- `SLandscapeHeightmapTrackerPanel::Construct()` creates it with `SNew(SHeightmapTrackerImageView)` at `SLandscapeHeightmapTrackerPanel.cpp:245-251`.
- The panel is created/owned by the module tab content: `SNew(SLandscapeHeightmapTrackerPanel)` in `LandscapeHeightmapTrackerModule.cpp:172-176`.

Graphify quality:

- Correct files: panel header and panel cpp; module cpp.
- Correct symbols: `SLandscapeHeightmapTrackerPanel`, `SHeightmapTrackerImageView`, `Construct`, `.OnPaint`, `.OnMouseButtonDown`.
- Missed/weak: ownership chain requires source verification; Graphify listed both widgets but did not clearly state that `SHeightmapTrackerImageView` is an internal child created inside the panel.
- False positives: `LandscapeSurfaceTraceHelper.h` was nearby but not needed for visualization widget ownership.
- Misleading relationships: proximity from `SCompoundWidget` broadened output substantially.
- Additional source reads: panel `.cpp`, module `.cpp`, panel `.h`.
- Additional searches: `SNew(SLandscapeHeightmapTrackerPanel)`, `SNew(SHeightmapTrackerImageView)`.
- Approx output/context volume: medium-high.
- Usefulness: 4/5.

## Q3. Cursor Position Over Landscape

Verified answer:

- There is no continuous cursor-move acquisition path in current source. The implemented path is click-driven.
- `FLandscapeHeightmapTrackerEdMode::InputKey()` handles left mouse press at `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp:14-25`.
- If tracking mode is enabled, it calls `TraceLandscapeClick(ViewportClient, Viewport)` at `LandscapeHeightmapTrackerEdMode.cpp:18-20`.
- `TraceLandscapeClick()` reads the editor viewport cursor ray using `ViewportClient->GetCursorWorldLocationFromMousePos()` at `LandscapeHeightmapTrackerEdMode.cpp:71`.
- It derives a trace segment with `FViewportTraceRayBuilder::BuildTraceSegment()` at `LandscapeHeightmapTrackerEdMode.cpp:72-75`.
- It performs `World->LineTraceSingleByChannel(...)` at `LandscapeHeightmapTrackerEdMode.cpp:110` and broadcasts `FViewportClickResult` at `LandscapeHeightmapTrackerEdMode.cpp:123-127`.

Graphify quality:

- Correct files: `LandscapeHeightmapTrackerEdMode.cpp/.h`, panel callback, coordinate mapper.
- Correct symbols: `InputKey`, `TraceLandscapeClick`, `OnViewportClick`, `MapWorldPosition`.
- Missed important file: `ViewportTraceRayBuilder.cpp` was not prominent in Q3 output but is part of cursor ray derivation.
- False positives: coordinate mapper nodes are downstream, not cursor acquisition.
- Misleading relationships: Graphify did not identify external Unreal API `GetCursorWorldLocationFromMousePos` as a node.
- Additional source reads: ed mode `.cpp`, trace ray builder `.cpp`.
- Additional searches: `Cursor`, `Mouse`, `Viewport`, `TraceLandscapeClick`.
- Approx output/context volume: medium.
- Usefulness: 3/5.

## Q4. World/Local Landscape Position to Heightmap Coordinates

Verified answer:

- `FLandscapeCoordinateMapper::MapWorldPosition()` is in `Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp:8-23`.
- It converts world to local via `LandscapeTransform.InverseTransformPosition(WorldPosition)` at `LandscapeCoordinateMapper.cpp:17`, then delegates to `MapLocalPosition()`.
- `MapLocalPosition()` validates bounds and image size at `LandscapeCoordinateMapper.cpp:34-44`.
- It computes `Width = Max.X - Min.X`, `Height = Max.Y - Min.Y`, then normalized `U` and `V` at `LandscapeCoordinateMapper.cpp:46-49`.
- It checks bounds tolerance at `LandscapeCoordinateMapper.cpp:51-59`, optionally clamps at `65-66`, flips at `68-76`, stores `NormalizedUV` at `78`, and stores `Pixel` with `UVToPixel()` at `79`.
- `UVToPixel()` maps `[0,1]` to inclusive image indices using `RoundToInt(UV * (ImageSize - 1))` and clamps at `LandscapeCoordinateMapper.cpp:134-138`.
- Reverse path: `MapUVToLocalPosition()` clamps/flips display UV and maps back to local XY at `LandscapeCoordinateMapper.cpp:84-131`.

Graphify quality:

- Correct files: coordinate mapper `.cpp/.h`, panel, heightmap image click mapper.
- Correct symbols: `MapWorldPosition`, `MapLocalPosition`, `MapUVToLocalPosition`, `UVToPixel`.
- Missed/weak: formulas required direct source read; Graphify only returned symbols.
- False positives: docs and module nodes appeared in broad BFS output.
- Misleading relationships: none accepted as flow without source verification.
- Additional source reads: `LandscapeCoordinateMapper.cpp/.h`, panel reverse-mapping lines.
- Additional searches: conversion function names.
- Approx output/context volume: medium.
- Usefulness: 4/5.

## Q5. Actual Code Path Updating Visual Marker on Landscape Cursor Interaction

Verified answer:

- The premise says "when the cursor moves"; current code updates the heightmap marker on landscape click, not on mouse move.
- Tracking is enabled by panel construction and checkbox:
  - panel subscribes to module delegate at `SLandscapeHeightmapTrackerPanel.cpp:176`;
  - panel sets tracking enabled at `SLandscapeHeightmapTrackerPanel.cpp:177`;
  - checkbox calls `SetTrackingEnabled()` at `SLandscapeHeightmapTrackerPanel.cpp:260-262` and `529-538`;
  - module toggles ed mode activation at `LandscapeHeightmapTrackerModule.cpp:65-69` and `37-57`.
- Runtime path:
  - `InputKey()` handles left click and calls `TraceLandscapeClick()` at `LandscapeHeightmapTrackerEdMode.cpp:14-20`.
  - `TraceLandscapeClick()` gets cursor ray, line traces, fills `FViewportClickResult`, and broadcasts at `LandscapeHeightmapTrackerEdMode.cpp:71-127`.
  - `SLandscapeHeightmapTrackerPanel::OnViewportClick()` receives the delegate, validates assigned landscape, refreshes bounds, calls `MapWorldPosition()`, and sets `MarkerUV = LastMapping.NormalizedUV` at `SLandscapeHeightmapTrackerPanel.cpp:424-456`.
  - `SHeightmapTrackerImageView::OnPaint()` reads `MarkerUV` and draws crosshair marker at `SLandscapeHeightmapTrackerPanel.cpp:87-105`.

Graphify quality:

- Correct files from broader Q10: ed mode, module, panel, mapper.
- Q5-specific Graphify false positive: it emphasized `OnHeightmapClicked()` and `TraceAssignedLandscapeSurface`, which are reverse mapping from image click to landscape marker, not landscape cursor/click to heightmap marker.
- Missed important files: `LandscapeHeightmapTrackerEdMode.cpp`, `ViewportTraceRayBuilder.cpp` in the direct Q5 query.
- Misleading relationships: `TraceAssignedLandscapeSurface --calls --> OnHeightmapClicked` is backward/misleading for runtime; source shows `OnHeightmapClicked()` calls `TraceAssignedLandscapeSurface()`.
- Additional source reads: ed mode, module, panel, trace builder.
- Additional searches: marker/cursor/viewport patterns.
- Approx output/context volume: high because misleading result required correction.
- Usefulness: 2/5.

## Q6. Actual Callers of Main Coordinate Conversion

Verified answer:

Treating `MapWorldPosition()` as the main world-to-heightmap conversion:

- Production caller: `SLandscapeHeightmapTrackerPanel::OnViewportClick()` at `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:449`.
- Test callers: `FLandscapeCoordinateMapperOptionsTest::RunTest()` at `Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp:59` and `:65`.
- Internal implementation call: `MapWorldPosition()` calls `MapLocalPosition()` at `LandscapeCoordinateMapper.cpp:15-19`.

If including all coordinate conversion functions:

- `MapLocalPosition()` is called by `MapWorldPosition()` and many tests at `LandscapeCoordinateMapperTests.cpp:15,22,26,43,48,53,69,73`.
- `MapUVToLocalPosition()` is called by `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked()` at `SLandscapeHeightmapTrackerPanel.cpp:485` and tests at `LandscapeCoordinateMapperTests.cpp:88,93-99,105,111,117-118`.
- `UVToPixel()` is called by `MapLocalPosition()` at `LandscapeCoordinateMapper.cpp:79` and `OnHeightmapClicked()` at `SLandscapeHeightmapTrackerPanel.cpp:508`.

Graphify quality:

- Correct files: mapper, panel, mapper tests.
- Correct symbols: `MapWorldPosition`, `OnViewportClick`, coordinate mapper tests.
- Missed/weak: actual caller completeness needed `rg`; Graphify listed tests but did not separate real callers from nearby test class nodes.
- False positives: basic/reverse test nodes appeared in the `MapWorldPosition` query even where they do not call `MapWorldPosition`.
- Misleading relationships: BFS proximity around `FLandscapeCoordinateMapper` is not caller analysis.
- Additional source reads: mapper tests and panel.
- Additional searches: exact function call regex.
- Approx output/context volume: medium.
- Usefulness: 3/5.

## Q7. Impact Radius of Changing Coordinate-Conversion Semantics

Verified answer:

High-impact files/symbols:

- `Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h`: result structs, options, public conversion API.
- `Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp`: core semantics for world/local/UV/pixel mapping.
- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`:
  - `OnViewportClick()` depends on forward mapping and marker UV at `424-456`.
  - `OnHeightmapClicked()` depends on reverse mapping, world transform, surface trace, and `UVToPixel()` at `465-527`.
  - diagnostic text reads `LastMapping` at `685-706`.
- `Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp`: expected UV/pixel/local results throughout.
- Related but indirect:
  - `HeightmapImageClickMapper.cpp` maps widget local coordinates to display UV; display UV semantics must remain compatible with mapper display UV.
  - `LandscapeSurfaceTraceHelper.cpp` handles reverse-mapped world XY to surface Z; affected if reverse mapping changes local/world XY interpretation.

Estimated radius: medium-high within this plugin. The change would affect the central mapper, the Slate panel marker, reverse landscape marker, diagnostics, settings flips, and mapper tests. It probably would not affect command registration, style, build rules, or trace ray math unless semantics change the meaning of input world position acquisition.

Graphify quality:

- Correct files: mapper, panel, tests, docs.
- Missed/weak: impact to `HeightmapImageClickMapper.cpp` and `LandscapeSurfaceTraceHelper.cpp` needed reasoning/source verification.
- False positives: `ViewportTraceRayBuilderTests.cpp` appeared but is not directly sensitive to coordinate conversion semantics.
- Misleading relationships: community proximity overstated trace-ray impact.
- Additional source reads: mapper, panel, image click mapper, surface trace helper, tests.
- Additional searches: function-call and transform/dimension searches.
- Approx output/context volume: medium-high.
- Usefulness: 3/5.

## Q8. Code Sensitive to Translation, Rotation, or Non-Unit Scale

Verified answer:

- Forward mapping is transform-sensitive through `LandscapeTransform.InverseTransformPosition(WorldPosition)` at `LandscapeCoordinateMapper.cpp:17`.
- Production caller passes `AssignedLandscape->GetActorTransform()` at `SLandscapeHeightmapTrackerPanel.cpp:449`.
- Reverse mapping applies actor transform with `LandscapeTransform.TransformPosition(ReverseMapping.LocalPosition)` at `SLandscapeHeightmapTrackerPanel.cpp:492-493`.
- Tests explicitly cover translation/scale and rotation:
  - translated/scaled transform at `LandscapeCoordinateMapperTests.cpp:57-61`;
  - rotated transform at `LandscapeCoordinateMapperTests.cpp:63-67`.
- UI displays actor transform:
  - location/rotation/scale labels at `SLandscapeHeightmapTrackerPanel.cpp:208-216`;
  - getters at `679-681`.
- Bounds source is `AssignedLandscape->GetBoundingRect()` at `SLandscapeHeightmapTrackerPanel.cpp:579-581`, stored as local XY bounds. This is sensitive to how Unreal reports landscape local bounds, but not itself transform-aware.

Graphify quality:

- Correct files: panel, mapper, tests.
- Correct symbols: transform-sensitive UI getters and mapper functions.
- Missed important code: exact `InverseTransformPosition`, `TransformPosition`, and test transform literals required `rg`.
- False positives: UI getters are display-only, not mapping logic.
- Misleading relationships: broad query mixed docs/test plan scenario text with executable code.
- Additional source reads: mapper, panel, tests.
- Additional searches: transform API names.
- Approx output/context volume: medium.
- Usefulness: 3/5.

## Q9. Dimensional Assumptions

Verified answer:

Coordinate dimensions:

- `FLandscapeTrackerBounds::IsValid()` requires `Max.X > Min.X && Max.Y > Min.Y` at `LandscapeCoordinateMapper.cpp:3-6`.
- `MapLocalPosition()` assumes width/height from `LocalBounds.Max - Min` at `LandscapeCoordinateMapper.cpp:46-49`.
- Image size must be positive at `LandscapeCoordinateMapper.cpp:40-44`.
- Pixel conversion assumes inclusive max pixel index `ImageSize - 1` and round-to-nearest at `LandscapeCoordinateMapper.cpp:134-138`.

Landscape dimensions:

- `RefreshLandscapeBounds()` uses `AssignedLandscape->GetBoundingRect()` and maps rect min/max directly into local XY bounds at `SLandscapeHeightmapTrackerPanel.cpp:571-581`.
- No source references to component size, section base, subsection count, or vertex count were found by `rg` in `Source\LandscapeHeightmapTracker`; Graphify did not surface any actual component-size logic either.

Image/widget dimensions:

- `SHeightmapTrackerImageView::ComputeDesiredSize()` returns fixed `512 x 512` at `SLandscapeHeightmapTrackerPanel.cpp:62-64`.
- UI gives image slot `MinHeight(320.0f)` at `SLandscapeHeightmapTrackerPanel.cpp:240`.
- `CalculateFittedImageRect()` assumes aspect-preserving fit using `ImageAspect = ImageSize.X / ImageSize.Y`, `WidgetAspect = WidgetSize.X / WidgetSize.Y`, `DrawSize`, and centered `DrawOffset` at `HeightmapImageClickMapper.cpp:3-34`.
- Image click mapping treats clicks on `RelativePosition.X > DrawSize.X` or `Y > DrawSize.Y` as outside, but equality is inside at `HeightmapImageClickMapper.cpp:49-59`.
- PNG dimensions become `ImageSize = FIntPoint(ImageWrapper->GetWidth(), ImageWrapper->GetHeight())` at `SLandscapeHeightmapTrackerPanel.cpp:627`, used to create transient texture at `628` and brush size at `643`.

Graphify quality:

- Correct files: mapper, image click mapper, panel, tests.
- Missed/weak: no vertex/component-size findings; source search confirmed absence.
- False positives: `UPrimitiveComponent`, `HitCount`, and surface trace nodes are not dimensional coordinate assumptions.
- Misleading relationships: BFS mixed collision-hit dimensions with image/landscape dimensions.
- Additional source reads: mapper, image click mapper, panel, tests.
- Additional searches: `ImageSize`, `GetBoundingRect`, `ComponentSize`, `Subsection`, `RoundToInt`.
- Approx output/context volume: medium-high.
- Usefulness: 4/5.

## Q10. Marker Widget to Landscape Coordinate Acquisition

Verified answer:

Actual connection, source-verified:

1. `SHeightmapTrackerImageView` draws marker from its `MarkerUV` attribute in `OnPaint()` at `SLandscapeHeightmapTrackerPanel.cpp:87-105`.
2. `SLandscapeHeightmapTrackerPanel::Construct()` binds `.MarkerUV_Lambda([this]() { return MarkerUV; })` at `SLandscapeHeightmapTrackerPanel.cpp:245-248`.
3. The same `Construct()` subscribes to `FLandscapeHeightmapTrackerModule::OnViewportClickResult()` with `AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick)` at `SLandscapeHeightmapTrackerPanel.cpp:176`.
4. `FLandscapeHeightmapTrackerEdMode::InputKey()` calls `TraceLandscapeClick()` on left click when tracking is enabled at `LandscapeHeightmapTrackerEdMode.cpp:14-20`.
5. `TraceLandscapeClick()` acquires viewport cursor ray and landscape/world hit at `LandscapeHeightmapTrackerEdMode.cpp:71-127`, then broadcasts the module delegate.
6. `OnViewportClick()` maps `Click.WorldPosition` to `LastMapping.NormalizedUV` and updates the panel field `MarkerUV` at `SLandscapeHeightmapTrackerPanel.cpp:449-454`.

Graphify quality:

- Correct files/symbols in Q10 BFS: `SHeightmapTrackerImageView`, `.OnPaint`, `Construct`, `InputKey`, `TraceLandscapeClick`, `MapWorldPosition`, `OnViewportClick`.
- Important Graphify failure: `graphify path "MarkerUV" "GetCursorWorldLocationFromMousePos"` returned `No node matching 'GetCursorWorldLocationFromMousePos' found.`
- Misleading shortest path: `MarkerUV -> SHeightmapTrackerImageView -> OnUnavailableClicked -> Construct -> FLandscapeHeightmapTrackerModule -> TraceLandscapeClick` is structural graph proximity, not runtime flow.
- Additional source reads: panel, ed mode, module, mapper.
- Additional searches: marker/cursor/viewport patterns.
- Approx output/context volume: high.
- Usefulness: 3/5.

## Final Graphify Usefulness Summary

Graphify was strongest for orientation: it quickly identified the module, panel, mapper, ed mode, image click mapper, surface trace helper, and mapper tests. It was weaker as a caller/path tool in this C++/Unreal codebase: external Unreal API calls were absent as nodes, line numbers were stale, call directions were sometimes wrong or inferred, and shortest paths could be structural rather than executable.

The best workflow was Graphify-first discovery followed by exact `rg` and line-numbered source reads. On this benchmark, Graphify reduced initial file discovery effort but could not be trusted alone for runtime flow, impact radius, or caller completeness.
