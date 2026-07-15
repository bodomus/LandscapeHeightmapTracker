# CRG Benchmark Results: LandscapeHeightmapTracker

## Scope And Tooling

- Role: independent `crg_researcher`.
- Tool: code-review-graph / CRG.
- Version/build info: `code-review-graph 2.3.6`; CLI executable `C:\Python312\Scripts\code-review-graph.exe`.
- Indexing scope used: repository root `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`.
- Graph status: 25 files, 125 nodes, 916 edges; languages `csharp`, `cpp`, `c`; 0 embedded nodes; last updated `2026-07-09T20:17:25`, branch `main`, commit `93b214fbe2ea`.
- CRG methods exercised: symbol lookup, search, communities/dependencies, flows, affected flows, impact radius, change detection, rename preview as caller/callee proxy.
- Source verification: `rg` and line-numbered reads under `Source\LandscapeHeightmapTracker\...` only.
- Isolation: Graphify and benchmark reviewer artifacts were not read. Production code was not modified.

Important CRG limitation: no direct caller/callee MCP query was exposed in this session. CRG hints mention `query_graph`, but that tool was unavailable. I used flows, impact, rename preview, and source verification to evaluate caller/callee behavior.

## Verified Runtime Flow Summary

Viewport click marker path:

`FLandscapeHeightmapTrackerEdMode::InputKey`
-> `TraceLandscapeClick`
-> `ViewportClient->GetCursorWorldLocationFromMousePos`
-> `World->LineTraceSingleByChannel`
-> `FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast`
-> `SLandscapeHeightmapTrackerPanel::OnViewportClick`
-> `FLandscapeCoordinateMapper::MapWorldPosition`
-> `MarkerUV = LastMapping.NormalizedUV`, `bHasMarker = true`
-> `SHeightmapTrackerImageView::OnPaint` draws the marker.

There is no verified hover or continuous cursor-move update path. The code updates the marker from left mouse button clicks while tracking mode is enabled.

Reverse image-to-landscape marker path:

`SHeightmapTrackerImageView::OnMouseButtonDown`
-> `FHeightmapImageClickMapper::MapLocalPositionToDisplayUV`
-> `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked`
-> `FLandscapeCoordinateMapper::MapUVToLocalPosition`
-> `LandscapeTransform.TransformPosition`
-> `FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface`
-> `FLandscapeHeightmapTrackerModule::SetReverseMarker`
-> `FLandscapeHeightmapTrackerEdMode::Render` draws a world marker line.

## Per-Question Results

### Q1. Where does the plugin initialize and register editor-facing functionality?

| Field | Result |
|---|---|
| Correct files | `Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp`, `Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h` |
| Correct symbols | `FLandscapeHeightmapTrackerModule::StartupModule`, `ShutdownModule`, `RegisterMenus`, `PluginButtonClicked`, `OnSpawnPluginTab`, `IMPLEMENT_MODULE` |
| Verified answer | `StartupModule` initializes style and commands, maps the open-window command, registers `FLandscapeHeightmapTrackerEdMode`, registers menu startup callback, and registers the nomad tab spawner. `RegisterMenus` adds the Tools menu entry. `OnSpawnPluginTab` creates the Slate panel. |
| CRG result | Exact lookup for `StartupModule` found `LandscapeHeightmapTrackerModule.cpp::StartupModule` at lines 125-149. Hub/bridge nodes also included `StartupModule` and `UpdateEditorModeActivation`. |
| Missed important files | CRG did not surface `.uplugin` or Build.cs as part of initialization; source verification says runtime editor-facing registration is in the module cpp/h. |
| False positives | None for exact lookup. Broad multi-term search returned zero, which is a false negative rather than a false positive. |
| Misleading relationships | CRG impact from mapper files later pulled module startup into the blast radius even though Q1 initialization is not a runtime coordinate path. |
| Additional source reads | Module cpp/h. |
| Additional searches | `rg -n "OnViewportClickResult|SetReverseMarker|RegisterMode|RegisterNomadTabSpawner" Source` style checks. |
| Output/context volume | CRG exact lookup tiny; source read ~240 lines. |
| Usefulness | 4/5 |

### Q2. Which class implements the main heightmap visualization widget, and what code creates or owns it?

| Field | Result |
|---|---|
| Correct files | `Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp`, `Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h`, module cpp |
| Correct symbols | `SHeightmapTrackerImageView`, `SHeightmapTrackerImageView::OnPaint`, `SHeightmapTrackerImageView::OnMouseButtonDown`, `SLandscapeHeightmapTrackerPanel::Construct`, `FLandscapeHeightmapTrackerModule::OnSpawnPluginTab` |
| Verified answer | Private class `SHeightmapTrackerImageView` implements the image view and marker drawing. It is created by `SLandscapeHeightmapTrackerPanel::Construct` using `SNew(SHeightmapTrackerImageView)` and fed lambdas for `MarkerUV` and `HasMarker`. The panel is created by `OnSpawnPluginTab` via `SNew(SLandscapeHeightmapTrackerPanel)`. |
| CRG result | Exact `SLandscapeHeightmapTrackerPanel` search found the panel file, `SHeightmapTrackerImageView` at lines 40-148, panel `Construct`, `OnViewportClick`, `OnHeightmapClicked`, etc. Hub/bridge lists ranked panel `Construct`, image `OnPaint`, and image view class. |
| Missed important files | None after exact query; broad multi-token widget/marker search returned zero. |
| False positives | Header `class` pseudo-functions are parser artifacts. |
| Misleading relationships | CRG flows did not connect module tab spawn to panel construction as a meaningful UI ownership path. |
| Additional source reads | Panel cpp/h and module cpp. |
| Additional searches | `rg -n "SHeightmapTrackerImageView|MarkerUV|HasMarker|OnPaint" Source`. |
| Output/context volume | CRG search medium; source read large, ~780 lines across panel files. |
| Usefulness | 4/5 |

### Q3. Which code reads or derives the current cursor position over the Unreal Landscape?

| Field | Result |
|---|---|
| Correct files | `Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp`, `Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp`, module h/cpp |
| Correct symbols | `FLandscapeHeightmapTrackerEdMode::InputKey`, `TraceLandscapeClick`, `FViewportTraceRayBuilder::BuildTraceSegment`, `FLandscapeHeightmapTrackerModule::FViewportClickResult` |
| Verified answer | `InputKey` intercepts left mouse button press. `TraceLandscapeClick` reads `ViewportClient->GetCursorWorldLocationFromMousePos()`, builds a trace segment from cursor origin/direction, line-traces visibility, then stores `Hit.ImpactPoint` in `FViewportClickResult::WorldPosition` and broadcasts it. |
| CRG result | Exact lookup for `TraceLandscapeClick` found the function. Hub/bridge nodes ranked it highly. Flow list had `InputKey` as a 2-node flow but did not connect it to `TraceLandscapeClick` and then to the module broadcast. |
| Missed important files | `ViewportTraceRayBuilder.cpp/h` was only surfaced by impact/community context, not by exact cursor query. |
| False positives | None material. |
| Misleading relationships | CRG flow depth for `InputKey` was too shallow and did not represent the verified click-to-broadcast runtime flow. |
| Additional source reads | Ed mode cpp/h and viewport trace builder. |
| Additional searches | `rg -n "GetCursorWorldLocationFromMousePos|LineTraceSingleByChannel|Broadcast|ImpactPoint" Source\LandscapeHeightmapTracker`. |
| Output/context volume | CRG small; source read ~130 lines for ed mode plus search output. |
| Usefulness | 3/5 |

### Q4. How does world-space or local-space Landscape position become heightmap image or pixel coordinates?

| Field | Result |
|---|---|
| Correct files | `Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp`, `Source\LandscapeHeightmapTracker\Public\LandscapeCoordinateMapper.h`, panel cpp |
| Correct symbols | `FLandscapeCoordinateMapper::MapWorldPosition`, `MapLocalPosition`, `MapUVToLocalPosition`, `UVToPixel`, `FLandscapeTrackerBounds`, `FLandscapeTrackerMappingOptions` |
| Verified answer | `MapWorldPosition` converts world to local with `LandscapeTransform.InverseTransformPosition(WorldPosition)` and delegates to `MapLocalPosition`. `MapLocalPosition` computes width/height from local bounds, normalizes X/Y into U/V, applies tolerance/clamp/flip, then calls `UVToPixel`, which rounds `UV * (ImageSize - 1)` and clamps to pixel bounds. Reverse mapping flips/clamps display UV and reconstructs local X/Y as `Min + U * Width`, `Min + V * Height`. |
| CRG result | Exact `LandscapeCoordinateMapper` and `MapLocalPosition` searches found mapper structs/functions. `get_affected_flows` identified CRG flows `MapWorldPosition -> MapLocalPosition -> IsValid -> UVToPixel` and `MapUVToLocalPosition -> IsValid`. |
| Missed important files | Panel cpp is needed to see real input values: actor transform, bounds from `GetBoundingRect`, `ImageSize` from PNG load. |
| False positives | Header parser produced a `LandscapeCoordinateMapper.h::class` function artifact. |
| Misleading relationships | CRG flow order lists `IsValid` and `UVToPixel` as a flat path, not all branch behavior or data transformations. |
| Additional source reads | Mapper cpp/h and panel cpp. |
| Additional searches | `rg -n "MapWorldPosition|MapLocalPosition|MapUVToLocalPosition|UVToPixel|GetBoundingRect|ImageSize" Source`. |
| Output/context volume | CRG medium; source read ~210 lines. |
| Usefulness | 4/5 |

### Q5. Trace the actual verified code path responsible for updating the visual marker when the cursor moves over the Landscape.

| Field | Result |
|---|---|
| Correct files | Ed mode cpp/h, module cpp/h, panel cpp/h, mapper cpp/h |
| Correct symbols | `InputKey`, `TraceLandscapeClick`, `OnViewportClickResult`, `SLandscapeHeightmapTrackerPanel::OnViewportClick`, `MapWorldPosition`, `SHeightmapTrackerImageView::OnPaint` |
| Verified answer | The actual verified path is click-based, not cursor-move based: left click -> trace hit -> broadcast `FViewportClickResult` -> panel handler validates assigned landscape -> `RefreshLandscapeBounds` -> `MapWorldPosition` -> set `bHasMarker = true` and `MarkerUV = LastMapping.NormalizedUV` -> image view `OnPaint` draws the crosshair marker at fitted image coordinates. |
| CRG result | CRG found each important symbol separately and ranked several as hubs/bridges, but no single CRG flow connected the cross-file runtime path. |
| Missed important files | CRG flows missed module delegate wiring and image view repaint dependency. Source reads were required. |
| False positives | CRG `SetReverseMarker` flow is the reverse path and not the viewport-click image marker path. |
| Misleading relationships | The flow list can mislead by presenting `SetReverseMarker` and `TraceAssignedLandscapeSurface` as top flows; those relate to image-to-landscape reverse marker, not cursor/click-to-image marker. |
| Additional source reads | Required across all four areas. |
| Additional searches | `rg -n "OnViewportClickResult|Broadcast|MarkerUV|bHasMarker|OnPaint|MapWorldPosition" Source`. |
| Output/context volume | High; no single graph answer, several source files. |
| Usefulness | 2/5 |

### Q6. Find all actual callers of the main coordinate-conversion function.

| Field | Result |
|---|---|
| Correct files | Mapper cpp/h, panel cpp, coordinate mapper tests |
| Correct symbols | Main public world conversion: `FLandscapeCoordinateMapper::MapWorldPosition`. Internal core conversion: `MapLocalPosition`. |
| Verified callers | `MapWorldPosition`: production caller `SLandscapeHeightmapTrackerPanel::OnViewportClick` at panel cpp line 449; test callers `LandscapeCoordinateMapperTests.cpp` lines 59 and 65. `MapLocalPosition`: internal caller `MapWorldPosition` line 15; test callers lines 15, 22, 26, 43, 48, 53, 69, 73. No other production callers found by `rg`. |
| CRG result | Rename preview for `MapWorldPosition` returned only the definition edit. Rename preview for `MapLocalPosition` returned definition plus the internal call at mapper cpp line 15. It missed qualified external static calls in panel/tests. |
| Missed important files | Panel cpp and tests were missed by CRG rename/caller proxy for `MapWorldPosition`. |
| False positives | None in source `rg`; CRG absence is false negative. |
| Misleading relationships | CRG impact radius listed many impacted files but did not reliably enumerate actual callers. |
| Additional source reads | Needed `rg` source search to answer correctly. |
| Additional searches | Exact `rg -n "MapWorldPosition|MapLocalPosition" Source`. |
| Output/context volume | CRG tiny but incomplete; source search medium. |
| Usefulness | 1/5 |

### Q7. Estimate the impact radius of changing coordinate-conversion semantics.

| Field | Result |
|---|---|
| Correct files | Mapper cpp/h, panel cpp/h, `HeightmapImageClickMapper.*` for display UV adjacency, tests |
| Correct symbols | `MapWorldPosition`, `MapLocalPosition`, `MapUVToLocalPosition`, `UVToPixel`, `OnViewportClick`, `OnHeightmapClicked`, `GetUvText`, `GetPixelText`, mapper tests |
| Verified answer | Changing `MapWorldPosition` or `MapLocalPosition` affects viewport click-to-heightmap marker placement, diagnostics (`LastMapping`, UV/pixel text), and coordinate mapper tests. Changing shared flip/clamp/bounds semantics also affects reverse image-to-landscape behavior through `MapUVToLocalPosition` and `UVToPixel` if the contract changes broadly. |
| CRG result | `get_impact_radius` for mapper cpp/h reported 12 directly changed nodes, 35 impacted nodes within 2 hops, 21 additional files. `get_affected_flows` reported flows `MapWorldPosition` and `MapUVToLocalPosition`. `detect_changes` reported 44 changed functions/classes, 2 affected flows, 44 test gaps, risk 0.85. `get_review_context` for cpp only contradicted this with low risk and 0 impacted nodes. |
| Missed important files | CRG did include panel/tests in impact, but with broad noisy edges. |
| False positives | Impact included unrelated editor startup/style/menu and trace helper files for a mapper-only semantic change. `detect_changes` treated many non-changed impacted nodes as changed functions. |
| Misleading relationships | CRG overestimated file blast radius and mixed dependency proximity with runtime impact. |
| Additional source reads | Required to separate actual runtime consumers from graph proximity. |
| Additional searches | Exact mapper call-site `rg`, transform/dimension searches. |
| Output/context volume | Very high; impact output was large and noisy. |
| Usefulness | 3/5 |

### Q8. Find code sensitive to Landscape translation, rotation, or non-unit scale.

| Field | Result |
|---|---|
| Correct files | Mapper cpp/h, panel cpp, coordinate mapper tests |
| Correct symbols | `MapWorldPosition`, `MapUVToLocalPosition`, `OnViewportClick`, `OnHeightmapClicked`, `RefreshLandscapeBounds`, actor diagnostic getters |
| Verified answer | `MapWorldPosition` uses `LandscapeTransform.InverseTransformPosition`, so translation/rotation/scale affect world-to-local mapping. `OnViewportClick` passes `AssignedLandscape->GetActorTransform()`. Reverse path uses `LandscapeTransform.TransformPosition(ReverseMapping.LocalPosition)` before vertical tracing. `RefreshLandscapeBounds` uses `AssignedLandscape->GetBoundingRect()` for local XY bounds. Diagnostics display actor location/rotation/scale. Tests cover translated/scaled and rotated transforms. |
| CRG result | CRG exact mapper lookup found the functions; hub/impact search surfaced panel functions. It did not directly identify transform sensitivity without source `rg`. |
| Missed important files | Tests proving transform assumptions were not surfaced by semantic transform search except through source `rg`. |
| False positives | `ComputeDesiredSize(float LayoutScaleMultiplier)` appeared in transform/scale grep but is UI layout scale, not Landscape transform. |
| Misleading relationships | CRG cannot distinguish data sensitivity from any call edge. |
| Additional source reads | Mapper, panel, tests. |
| Additional searches | `rg -n "GetActorTransform|InverseTransformPosition|TransformPosition|GetActorLocation|GetActorRotation|GetActorScale3D|GetBoundingRect|Scale|Rotation|Location|Transform" Source`. |
| Output/context volume | Medium. |
| Usefulness | 2/5 |

### Q9. Find dimensional assumptions involving width, height, resolution, vertex count, and component size.

| Field | Result |
|---|---|
| Correct files | Mapper cpp/h, `HeightmapImageClickMapper.cpp/h`, panel cpp/h, coordinate/image mapper tests |
| Correct symbols | `FLandscapeTrackerBounds::IsValid`, `MapLocalPosition`, `MapUVToLocalPosition`, `UVToPixel`, `CalculateFittedImageRect`, `LoadPngTexture`, `RefreshLandscapeBounds`, `ImageSize` fields |
| Verified answer | Bounds validity assumes `Max.X > Min.X && Max.Y > Min.Y`. Coordinate width/height are `Max - Min`. Image size must be positive. Pixel coordinates use `ImageSize.X - 1` and `ImageSize.Y - 1`, implying UV endpoints map to last pixel. Fitted display assumes nonzero widget/image dimensions and preserves image aspect ratio. Loaded PNG width/height become both `ImageSize` and Slate brush image size. No direct component size, vertex count, or resolution metadata usage was found beyond `GetBoundingRect` and image dimensions. |
| CRG result | CRG found mapper and image-click mapper functions in community/impact results, but no semantic query reliably found all dimensional assumptions. |
| Missed important files | Header field defaults and tests needed source reads. |
| False positives | Many lines containing "Heightmap" or class names made raw dimension grep noisy. |
| Misleading relationships | CRG parser labels headers as language `c` and emits `class` pseudo-functions, making type-level dimension assumptions harder to read. |
| Additional source reads | Mapper, image click mapper, panel, tests. |
| Additional searches | `rg -n "ImageSize|Width|Height|Resolution|Vertex|ComponentSize|Section|GetBoundingRect|ImageWrapper->GetWidth|ImageWrapper->GetHeight|ImageAspect|WidgetAspect|DrawSize" Source`. |
| Output/context volume | High due noisy grep. |
| Usefulness | 2/5 |

### Q10. Starting from the marker widget, identify the connection to Landscape coordinate acquisition.

| Field | Result |
|---|---|
| Correct files | Panel cpp/h, ed mode cpp/h, module cpp/h, mapper cpp/h, surface trace helper for reverse path |
| Correct symbols | `SHeightmapTrackerImageView`, `OnPaint`, `OnMouseButtonDown`, `SLandscapeHeightmapTrackerPanel::Construct`, `OnViewportClick`, `OnHeightmapClicked`, `TraceLandscapeClick`, module delegate |
| Verified answer | Forward marker display: image view is passive and reads `MarkerUV`/`HasMarker` attributes supplied by the owning panel; those fields are updated by the panel's module delegate handler after ed mode acquires a Landscape hit from a viewport click. Reverse path from the widget: `OnMouseButtonDown` maps display click to UV and calls panel `OnHeightmapClicked`, which maps UV to local Landscape coordinates, transforms to world, traces the assigned Landscape surface, and sets a reverse marker for editor viewport rendering. |
| CRG result | CRG found widget class, panel handlers, ed mode trace, and surface trace as separate nodes/flows. It did not produce the complete cross-file connection. |
| Missed important files | Module delegate storage and subscription/removal were only obvious from source. |
| False positives | CRG flow `SetReverseMarker` alone is not enough to infer widget origin. |
| Misleading relationships | Graph proximity between widget and mapper is not the same as runtime execution; lambdas/delegates and Slate attributes require source verification. |
| Additional source reads | Panel, module, ed mode, mapper, surface trace helper. |
| Additional searches | `rg -n "MarkerUV|HasMarker|OnHeightmapClicked|OnViewportClickResult|TraceLandscapeClick|SetReverseMarker" Source`. |
| Output/context volume | High. |
| Usefulness | 3/5 |

## Final CRG Usefulness Summary

CRG was useful for orientation: it quickly identified the major files and symbols after exact symbol queries, provided graph stats, ranked hubs/bridges, and highlighted mapper-related flows. It was much less reliable for this benchmark's harder questions: cross-file runtime paths through Unreal delegates/Slate lambdas, actual callers of qualified static C++ methods, and transform/dimensional semantic sensitivity all required source reads and exact `rg` searches.

Most useful CRG outputs:

- `list_graph_stats`: confirmed graph freshness and scope.
- Exact `semantic_search_nodes_tool` lookups: `StartupModule`, `SLandscapeHeightmapTrackerPanel`, `TraceLandscapeClick`, `MapLocalPosition`.
- `list_flows` / `get_affected_flows`: good local mapper flows, but incomplete cross-file runtime flows.
- `get_hub_nodes` / `get_bridge_nodes`: good orientation to high-importance functions.

Least reliable outputs:

- Multi-term "semantic" searches with no embeddings: several returned zero despite obvious relevant symbols.
- Rename preview for caller discovery: missed production/test qualified call sites.
- Impact/change analysis: overbroad and internally inconsistent for mapper-only changes.

Overall agent usefulness score for CRG on this benchmark: **3/5**.
