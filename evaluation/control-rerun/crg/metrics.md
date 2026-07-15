# CRG Control Rerun Metrics

## Tool Coverage

| Required area | Exercised | Evidence |
|---|---:|---|
| Symbol lookup | Yes | SQL node lookups for `TraceLandscapeClick`, `InputKey`, `OnViewportClick`, `MapWorldPosition`, `SHeightmapTrackerImageView`, `OnPaint`, `OnHeightmapClicked`. |
| Callers | Yes | `CALLS` target queries for `TraceLandscapeClick`, `MapWorldPosition`, `MapLocalPosition`, `OnViewportClick`, `OnHeightmapClicked`, `SetReverseMarker`. |
| Callees | Yes | `CALLS` source queries for `TraceLandscapeClick`, `OnViewportClick`, `MapWorldPosition`, `SetReverseMarker`, etc. |
| Dependencies | Yes | `IMPORTS_FROM` edge queries for key files. |
| Impact radius | Yes | Depth-2 BFS over `CALLS`/`IMPORTS_FROM` from `MapWorldPosition`, `TraceLandscapeClick`, `OnViewportClick`, `SHeightmapTrackerImageView`. |
| Semantic search | Yes | Local embedding cosine search over CRG `embeddings` for Q3/Q5/Q6/Q10 queries. |
| Change-related investigation | Yes | `code-review-graph detect-changes --repo . --brief`. |
| Source verification | Yes | `rg` validation and line-numbered reads of key source files. |

## Per-Question Metrics

| Question | CRG found before fallback? | First useful CRG result | Correct files surfaced by CRG | Correct symbols surfaced by CRG | Important misses | False positives / misleading results | Fallback searches required | Source files opened | Usefulness |
|---|---|---|---|---|---|---|---|---|---:|
| Q3 Cursor acquisition | Mostly yes | `TraceLandscapeClick` node at `LandscapeHeightmapTrackerEdMode.cpp:58-129`; `InputKey -> TraceLandscapeClick` edge | EdMode cpp; module header/cpp partly; trace builder by exact lookup | `InputKey`, `TraceLandscapeClick`, `GetCursorWorldLocationFromMousePos` target, `LineTraceSingleByChannel` target, `Broadcast` target | Did not strongly surface `BuildTraceSegment` in semantic ranking; engine calls are unqualified targets | Semantic ranked Build.cs/settings above true function; impact paths through `TEXT`, `GetName`, etc. are noisy | Yes, to verify impact point/result/broadcast | EdMode cpp, ViewportTraceRayBuilder cpp, Module h/cpp | 3 |
| Q5 Marker runtime path | Partially | `OnViewportClick`, `MapWorldPosition`, `OnPaint`, `TraceLandscapeClick` nodes; local `OnViewportClick` and `MapWorldPosition` flows | Panel cpp/h, EdMode cpp, mapper cpp, module cpp/h | `OnViewportClick`, `MapWorldPosition`, `OnPaint`, `OnMouseButtonDown`, `OnHeightmapClicked`, `SetReverseMarker` | No broadcast-to-callback edge; no Slate attribute data flow; missed static reverse-path calls | `SetReverseMarker` can be mistaken for forward marker update; `Construct -> OnHeightmapClicked` is binding approximation | Yes, required for delegate/broadcast/state writes/paint/hover absence | EdMode cpp, Panel cpp/h, Mapper cpp, Module cpp, SurfaceTrace cpp | 2 |
| Q6 Actual callers | No | `MapWorldPosition` definition node only | Mapper cpp/h | `MapWorldPosition`, `MapLocalPosition` | Missed production caller `SLandscapeHeightmapTrackerPanel.cpp:449`; missed test callers `LandscapeCoordinateMapperTests.cpp:59,65` | No exact caller false positives because no rows; semantic search did not find callers | Yes, exact `rg -n "MapWorldPosition"` | Mapper cpp, Panel cpp, CoordinateMapperTests cpp | 1 |
| Q10 Cross-subsystem connection | Partially for candidates, no for full flow | `SHeightmapTrackerImageView` class and `OnPaint`/`OnMouseButtonDown` nodes | Panel cpp/h, EdMode cpp, SurfaceTrace cpp, mapper cpp | `SHeightmapTrackerImageView`, `OnPaint`, `OnMouseButtonDown`, `OnViewportClick`, `TraceLandscapeClick`, `OnHeightmapClicked` | No delegate path from broadcast to callback; no `MarkerUV`/`bHasMarker` field modeling; no Slate lambda data flow | Semantic ranked Build.cs/settings/commands above key code; graph proximity confused forward vs reverse paths | Yes, for architecture/runtime correctness | Panel cpp/h, EdMode cpp/h, Module cpp/h, Mapper cpp/h, SurfaceTrace cpp/h | 3 |

## Q6 Caller Classification

| Classification | Location | Found by CRG before fallback? | Verification |
|---|---|---:|---|
| Production actual caller | `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:449` | No | `LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(...)` inside `OnViewportClick`. |
| Test actual caller | `Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp:59` | No | Translated/scaled transform test. |
| Test actual caller | `Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp:65` | No | Rotated transform test. |
| Declaration, not caller | `Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h:44` | Yes as source search only | Function declaration. |
| Definition, not caller | `Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp:8` | Yes | Function definition. |
| Indirect callee relationship | `LandscapeCoordinateMapper.cpp:15` | Yes | `MapWorldPosition -> MapLocalPosition`; not a caller. |

CRG caller recall for `MapWorldPosition`: 0/3 actual call sites before fallback validation.

CRG caller precision for exact caller query: not meaningful; query returned 0 rows.

## Verified Runtime Path Metrics

| Path element | Verified source | CRG support | Notes |
|---|---|---|---|
| Entry point | `LandscapeHeightmapTrackerEdMode.cpp:14-20` | Exact node and edge | Left mouse button press only. |
| Viewport interaction | `LandscapeHeightmapTrackerEdMode.cpp:16`, `:71` | Callee target strings | No hover/move path found. |
| Cursor ray acquisition | `LandscapeHeightmapTrackerEdMode.cpp:71-75`, `ViewportTraceRayBuilder.cpp:3-37` | Partial | `BuildTraceSegment` exact node exists; semantic ranking weak. |
| Tracing | `LandscapeHeightmapTrackerEdMode.cpp:105-110` | Callee target string | Engine call not resolved as project symbol. |
| Result propagation | `LandscapeHeightmapTrackerEdMode.cpp:123-127`, `Module.h:12-19`, `Panel.cpp:176` | Partial | Broadcast and AddSP not connected by graph. |
| Mapping | `Panel.cpp:449`, `Mapper.cpp:8-23` | Partial | CRG missed actual caller edge. |
| State writes | `Panel.cpp:452-454` | No | Fields not modeled. |
| Paint usage | `Panel.cpp:87-105` | Exact `OnPaint` node | Attribute reads not connected to state writes. |
| Reverse path | `Panel.cpp:111-131`, `:465-512`, `SurfaceTraceHelper.cpp:49-111` | Partial/noisy | Must not be treated as forward marker update. |

## Failure Counts

| Failure type | Count / examples |
|---|---|
| Missed actual callers | 3 for `MapWorldPosition`. |
| Missed delegate relationships | At least 1 critical relationship: `Broadcast(Result)` to panel `OnViewportClick` via `AddSP`. |
| Missed Slate data relationships | `MarkerUV` and `bHasMarker` writes to `OnPaint` reads. |
| Misleading execution-flow candidates | `SetReverseMarker` flow for Q5 forward marker path; semantic `OnMouseButtonDown` for viewport cursor acquisition; `Construct -> OnHeightmapClicked` binding as call. |
| Noisy impact relationships | Paths through `TEXT`, `Get`, `IsValid`, `GetName`, `GetActor`, `Broadcast`. |
| Parser/source-range issues | `LandscapeCoordinateMapperTests.cpp` has incorrect `RunTest` node ownership/ranges for earlier test lines. |

## Overall Score

Overall usefulness for this controlled rerun: 2.25/5 average across Q3/Q5/Q6/Q10.

Interpretation: CRG is useful as an orientation and local-symbol lookup tool, but insufficient as a sole authority for runtime-flow and caller analysis in this Unreal/Slate C++ plugin. Source verification was necessary for every benchmark question.
