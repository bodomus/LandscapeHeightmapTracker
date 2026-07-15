# Graphify Metrics - Controlled Rerun

Scale: usefulness score 1-5, where 1 = mostly misleading/noisy, 3 = useful candidate discovery but source validation required, 5 = near-complete and directly verifiable.

| Question | Graphify found answer before fallback? | First useful Graphify result | Correct files | Correct symbols | Missed symbols / facts | False positives | Misleading relationships | Fallback searches required | Source files opened | Score |
|---|---|---|---|---|---|---|---|---|---|---:|
| Q3 Cursor acquisition | Partial yes: found core files/symbols, not exact full flow | `InputKey`, `TraceLandscapeClick`, `BuildTraceSegment`, `OnViewportClick`, `MapWorldPosition` nodes | `LandscapeHeightmapTrackerEdMode.cpp`, `ViewportTraceRayBuilder.cpp/.h`, `LandscapeHeightmapTrackerModule.cpp/.h`, `SLandscapeHeightmapTrackerPanel.cpp`, `LandscapeCoordinateMapper.cpp/.h` | `FLandscapeHeightmapTrackerEdMode::InputKey`, `TraceLandscapeClick`, `FViewportTraceRayBuilder::BuildTraceSegment`, `FViewportClickResult`, `OnViewportClickResult().Broadcast`, `SLandscapeHeightmapTrackerPanel::OnViewportClick`, `FLandscapeCoordinateMapper::MapWorldPosition` | Concrete `GetCursorWorldLocationFromMousePos`, `LineTraceSingleByChannel`, exact result-field propagation lines were not clearly returned in first Graphify output | Evaluation/CRG artifact nodes; `OnHeightmapClicked` near cursor acquisition | Shared-type proximity and evaluation-doc proximity could be mistaken for flow | Yes: `rg` for input/trace/broadcast and source reads | EdMode cpp/h; viewport ray builder cpp/h; module cpp/h; panel cpp/h; mapper cpp/h | 3 |
| Q5 Marker runtime path | Partial: found marker/widget/module symbols but mixed forward and reverse paths | `SHeightmapTrackerImageView`, `.OnPaint()`, `.OnMouseButtonDown()`, `OnViewportClick`, `OnHeightmapClicked`, `SetReverseMarker` | `SLandscapeHeightmapTrackerPanel.cpp/.h`, `LandscapeHeightmapTrackerEdMode.cpp`, `LandscapeHeightmapTrackerModule.cpp/.h`, `LandscapeCoordinateMapper.cpp/.h`, `HeightmapImageClickMapper.cpp`, `LandscapeSurfaceTraceHelper.cpp` | `OnViewportClickResult().AddSP`, `TraceLandscapeClick`, `Broadcast`, `OnViewportClick`, `MapWorldPosition`, `bHasMarker`, `MarkerUV`, `MarkerUV_Lambda`, `HasMarker_Lambda`, `OnPaint` | Absence of hover/mouse-move path; exact distinction that `OnHeightmapClicked` does not set image marker state | `OnMouseButtonDown`, `TraceAssignedLandscapeSurface`, `SetReverseMarker` as candidates for image marker update | Reverse image-to-Landscape path presented close to Landscape-click marker path; old graph question wording suggested cursor movement | Yes: marker-state searches, hover absence search, source reads | Panel cpp/h; EdMode cpp; module cpp/h; mapper cpp/h; image click mapper cpp; surface trace helper cpp | 2 |
| Q6 Actual callers | Partial: found real caller candidates, not exact caller edges/list | `FLandscapeCoordinateMapper::MapWorldPosition`, `SLandscapeHeightmapTrackerPanel::OnViewportClick`, `FLandscapeCoordinateMapperOptionsTest::RunTest` | `SLandscapeHeightmapTrackerPanel.cpp`, `LandscapeCoordinateMapperTests.cpp`, `LandscapeCoordinateMapper.cpp/.h` | Production caller `SLandscapeHeightmapTrackerPanel::OnViewportClick`; test caller `FLandscapeCoordinateMapperOptionsTest::RunTest`; callee `MapLocalPosition` | Incoming call edges into `MapWorldPosition`; exact line-level caller list | `FLandscapeCoordinateMapperBasicTest::RunTest`, `FLandscapeCoordinateMapperReverseMappingTest::RunTest`, declaration, definition, `MapLocalPosition` if treated as caller | `graphify explain` showed only outgoing refs/inferred callee, not actual incoming callers | Yes: `rg -n "MapWorldPosition" Source` | Panel cpp; tests cpp; mapper cpp/h | 3 |
| Q10 Cross-subsystem connection | Partial: good orientation, incorrect shortest paths as runtime flow | `SHeightmapTrackerImageView`, `OnPaint`, `OnMouseButtonDown`, `OnViewportClick`, `TraceLandscapeClick`, module nodes | `SLandscapeHeightmapTrackerPanel.cpp/.h`, `LandscapeHeightmapTrackerEdMode.cpp`, `LandscapeHeightmapTrackerModule.cpp/.h`, `LandscapeCoordinateMapper.cpp/.h`, `HeightmapImageClickMapper.cpp`, `LandscapeSurfaceTraceHelper.cpp` | `SHeightmapTrackerImageView`, `MarkerUV_Lambda`, `HasMarker_Lambda`, `OnPaint`, `OnViewportClickResult`, `InputKey`, `TraceLandscapeClick`, `OnViewportClick`, `MapWorldPosition` | Actual bridge is module delegate plus panel-owned state; Graphify did not explain this without source validation | `FVector2D`, `FLandscapeTrackerMappingResult`, `OnHeightmapClicked`, reverse helper symbols as bridge nodes | Shortest paths through shared types/results were not execution paths; target match for `TraceLandscapeClick` was ambiguous | Yes: path verification, marker-state source reads, reverse-path source reads | Panel cpp/h; EdMode cpp/h; module cpp/h; mapper cpp/h; image click mapper cpp; surface trace helper cpp | 2 |

## Caller Measurement for Q6

Direct source truth:

| Category | Symbol / file | Lines | Count |
|---|---|---:|---:|
| Declaration | `FLandscapeCoordinateMapper::MapWorldPosition` in `LandscapeCoordinateMapper.h` | 44 | 0 callers |
| Definition | `FLandscapeCoordinateMapper::MapWorldPosition` in `LandscapeCoordinateMapper.cpp` | 8 | 0 callers |
| Production caller | `SLandscapeHeightmapTrackerPanel::OnViewportClick` | 449 | 1 |
| Test caller | `FLandscapeCoordinateMapperOptionsTest::RunTest` | 59, 65 | 2 call sites |

Graphify before fallback:

- Production caller candidate found: yes.
- Test caller candidate found: yes, partial.
- Exact actual caller list found: no.
- Incoming call edges found by `graphify explain`: no.

## Aggregate

| Metric | Value |
|---|---:|
| Questions evaluated | 4 |
| Average usefulness score | 2.5 / 5 |
| Questions needing fallback source search | 4 / 4 |
| Questions with misleading structural/path relationships | 4 / 4 |
| Questions where Graphify found at least one correct source candidate before fallback | 4 / 4 |
| Questions where Graphify alone produced a complete verified answer | 0 / 4 |

Main failure mode: Graphify is a useful index of nearby files and symbols, but it does not reliably distinguish actual calls, delegate runtime flow, ownership, dependencies, and structural graph proximity. Shortest paths were especially unsafe for Q10.
