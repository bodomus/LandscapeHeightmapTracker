# Graphify Metrics

## Run Metadata

| Item | Value |
|---|---|
| Tool | Graphify |
| Version | `graphify 0.9.8` |
| Binary | `C:\Users\bodom\.local\bin\graphify.exe` |
| Existing graph | `graphify-out/graph.json` |
| Graph built commit | `f17cf66490dc1af63aea15b651ba1fa069b9d69c` |
| Current `HEAD` | `93b214fbe2ea3f78286e2240d6b117eb6812c36d` |
| Freshness | stale; all answers source-verified |
| Graph nodes | 376 |
| Graph links | 538 |
| Directed | `False` |
| Multigraph | `False` |
| Corpus from report | 30 files, ~7,315 words |
| Source files observed in graph nodes | 31 unique source_file values |
| Token cost in Graphify report | 0 input, 0 output |

## Per-Question Scores

| Q | Correct Files | Correct Symbols | Missed Important Files | False Positives | Misleading Relationships | Extra Source Reads | Extra Searches | Approx Output Volume | Usefulness |
|---|---|---|---|---|---|---|---|---|---|
| Q1 | Yes | Mostly | None significant | README/build nodes | `Initialize -> StartupModule` direction suspicious | module `.cpp/.h` | registration `rg` | Medium | 4/5 |
| Q2 | Yes | Yes | Module ownership needed verification | surface trace helper | proximity from `SCompoundWidget` | panel/module | `SNew` patterns | Medium-high | 4/5 |
| Q3 | Partial | Partial | `ViewportTraceRayBuilder.cpp` | mapper nodes downstream | external cursor API missing | ed mode, ray builder | cursor/viewport patterns | Medium | 3/5 |
| Q4 | Yes | Yes | formulas not in graph output | docs/module nodes | none accepted | mapper source | conversion names | Medium | 4/5 |
| Q5 | Partial | Misfocused | ed mode, ray builder | reverse-mapping path | reversed/misleading `TraceAssignedLandscapeSurface` relationship | ed mode/module/panel | marker/cursor/viewport | High | 2/5 |
| Q6 | Partial | Partial | exact caller set | non-calling test nodes | BFS proximity as caller analysis | mapper tests/panel | exact call regex | Medium | 3/5 |
| Q7 | Partial | Partial | image click mapper, surface trace helper | viewport trace tests | overstated trace-ray impact | mapper/panel/helpers/tests | call/transform searches | Medium-high | 3/5 |
| Q8 | Partial | Partial | exact transform API lines | display-only getters | docs/test scenario proximity | mapper/panel/tests | transform API search | Medium | 3/5 |
| Q9 | Yes | Mostly | component-size absence required search | hit/component nodes | collision dimensions mixed in | mapper/image/panel/tests | dimension keyword search | Medium-high | 4/5 |
| Q10 | Partial | Yes in BFS | external Unreal cursor API node missing | `OnUnavailableClicked` path | shortest path not runtime flow | panel/ed/module/mapper | marker/cursor search | High | 3/5 |

Average usefulness score: 3.3/5.

## Verified Impact Radius Counts

| Category | Files |
|---|---|
| Core coordinate API/implementation | `Public/LandscapeCoordinateMapper.h`, `Private/LandscapeCoordinateMapper.cpp` |
| Production forward marker path | `Private/LandscapeHeightmapTrackerEdMode.cpp`, `Private/ViewportTraceRayBuilder.cpp`, `Private/LandscapeHeightmapTrackerModule.cpp`, `Private/SLandscapeHeightmapTrackerPanel.cpp` |
| Production reverse marker path | `Private/SLandscapeHeightmapTrackerPanel.cpp`, `Private/LandscapeSurfaceTraceHelper.cpp` |
| Image display/click mapping | `Private/SLandscapeHeightmapTrackerPanel.cpp`, `Private/HeightmapImageClickMapper.cpp` |
| Tests directly sensitive to coordinate semantics | `Private/Tests/LandscapeCoordinateMapperTests.cpp`, `Private/Tests/HeightmapImageClickMapperTests.cpp` |
| Tests adjacent but not direct coordinate conversion | `Private/Tests/ViewportTraceRayBuilderTests.cpp`, `Private/Tests/LandscapeSurfaceTraceHelperTests.cpp` |

## Relationship Reliability

| Relationship Type | Reliability Observed | Notes |
|---|---|---|
| Structural file/symbol proximity | High for discovery | Good at finding likely areas. |
| Ownership relationships | Medium | Found panel and image widget, but ownership needed source reads. |
| Call relationships | Medium-low | Some extracted calls correct, some inferred/directional issues. |
| Dependency relationships | Medium | Includes and shared structs surfaced well enough for orientation. |
| Shortest path as runtime flow | Low | `MarkerUV -> TraceLandscapeClick` path was not executable flow. |
| External Unreal API calls | Low | `GetCursorWorldLocationFromMousePos` was not a graph node. |

## Overall Assessment

Graphify was useful for initial navigation and symbol discovery, especially for Q1, Q2, Q4, and Q9. It was not sufficient for benchmark-grade answers without direct source verification. Stale line numbers, broad BFS neighborhoods, missing external API nodes, and misleading shortest paths were the main failure modes.
