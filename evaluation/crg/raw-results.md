# CRG Raw Results

This file preserves the exact queries/commands and relevant raw outputs used for the CRG benchmark. Long outputs are excerpted only where they were too large for concise reporting; the exact retained excerpts are copied verbatim.

## Version And Scope

Command:

```powershell
code-review-graph --version
```

Output:

```text
code-review-graph 2.3.6
```

Command:

```powershell
code-review-graph status
```

Output:

```text
Nodes: 125
Edges: 916
Files: 25
Languages: csharp, cpp, c
Last updated: 2026-07-09T20:17:25
Built on branch: main
Built at commit: 93b214fbe2ea
```

MCP `list_graph_stats_tool`:

```json
{"status":"ok","summary":"Graph statistics for LandscapeHeightmapTracker:\n  Files: 25\n  Total nodes: 125\n  Total edges: 916\n  Languages: csharp, cpp, c\n  Last updated: 2026-07-09T20:17:25\n\nNodes by kind:\n  Class: 12\n  File: 25\n  Function: 88\n\nEdges by kind:\n  CALLS: 715\n  CONTAINS: 105\n  IMPORTS_FROM: 95\n  INHERITS: 1\n\nEmbeddings: 0 nodes embedded\n  (install sentence-transformers for semantic search)","total_nodes":125,"total_edges":916,"nodes_by_kind":{"Class":12,"File":25,"Function":88},"edges_by_kind":{"CALLS":715,"CONTAINS":105,"IMPORTS_FROM":95,"INHERITS":1},"languages":["csharp","cpp","c"],"files_count":25,"last_updated":"2026-07-09T20:17:25","embeddings_count":0}
```

## CRG Search Queries

Query:

```json
{"query":"Landscape heightmap marker cursor position coordinate conversion widget editor module register tab","limit":20,"detail_level":"standard"}
```

Output:

```json
{"status":"ok","query":"Landscape heightmap marker cursor position coordinate conversion widget editor module register tab","search_mode":"keyword","summary":"Found 0 node(s) matching 'Landscape heightmap marker cursor position coordinate conversion widget editor module register tab'","results":[]}
```

Query:

```json
{"query":"StartupModule","limit":10,"detail_level":"standard"}
```

Relevant output:

```json
{"status":"ok","query":"StartupModule","search_mode":"hybrid","summary":"Found 1 node(s) matching 'StartupModule'","results":[{"name":"StartupModule","qualified_name":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeHeightmapTrackerModule.cpp::StartupModule","kind":"Function","file_path":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeHeightmapTrackerModule.cpp","line_start":125,"line_end":149,"language":"cpp","params":null,"return_type":null,"signature":"def StartupModule()","score":0.032787}]}
```

Query:

```json
{"query":"SLandscapeHeightmapTrackerPanel","limit":20,"detail_level":"standard"}
```

Relevant output:

```json
{"status":"ok","query":"SLandscapeHeightmapTrackerPanel","search_mode":"hybrid","summary":"Found 20 node(s) matching 'SLandscapeHeightmapTrackerPanel'","results":[{"name":"~SLandscapeHeightmapTrackerPanel","kind":"Function","line_start":331,"line_end":337},{"name":"SHeightmapTrackerImageView","kind":"Class","line_start":40,"line_end":148},{"name":"Construct","kind":"Function","line_start":164,"line_end":329},{"name":"OnViewportClick","kind":"Function","line_start":424,"line_end":463},{"name":"OnHeightmapClicked","kind":"Function","line_start":465,"line_end":527}]}
```

Query:

```json
{"query":"TraceLandscapeClick","limit":20,"detail_level":"standard"}
```

Relevant output:

```json
{"status":"ok","query":"TraceLandscapeClick","search_mode":"hybrid","summary":"Found 1 node(s) matching 'TraceLandscapeClick'","results":[{"name":"TraceLandscapeClick","qualified_name":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeHeightmapTrackerEdMode.cpp::TraceLandscapeClick","kind":"Function","file_path":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeHeightmapTrackerEdMode.cpp","line_start":58,"line_end":129,"language":"cpp","params":null,"return_type":null,"signature":"def TraceLandscapeClick()","score":0.032787}]}
```

Query:

```json
{"query":"MapLocalPosition","limit":20,"detail_level":"standard"}
```

Relevant output:

```json
{"status":"ok","query":"MapLocalPosition","search_mode":"hybrid","summary":"Found 1 node(s) matching 'MapLocalPosition'","results":[{"name":"MapLocalPosition","qualified_name":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeCoordinateMapper.cpp::MapLocalPosition","kind":"Function","file_path":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeCoordinateMapper.cpp","line_start":25,"line_end":82,"language":"cpp","params":null,"return_type":null,"signature":"def MapLocalPosition()","score":0.032787}]}
```

Query:

```json
{"query":"SetMarker","limit":20,"detail_level":"standard"}
```

Output:

```json
{"status":"ok","query":"SetMarker","search_mode":"keyword","summary":"Found 0 node(s) matching 'SetMarker'","results":[]}
```

## Flow And Impact Outputs

MCP `list_flows_tool`:

```json
{"status":"ok","summary":"Found 15 execution flow(s)","flows":[{"id":1,"name":"TraceAssignedLandscapeSurface","depth":1,"node_count":3,"file_count":1,"criticality":0.5267},{"id":2,"name":"SetReverseMarker","depth":1,"node_count":3,"file_count":1,"criticality":0.4433},{"id":3,"name":"UseSelectedLandscape","depth":2,"node_count":4,"file_count":1,"criticality":0.4325},{"id":5,"name":"OnViewportClick","depth":1,"node_count":4,"file_count":1,"criticality":0.4225},{"id":7,"name":"MapWorldPosition","depth":2,"node_count":4,"file_count":1,"criticality":0.37},{"id":9,"name":"Construct","depth":2,"node_count":7,"file_count":1,"criticality":0.37},{"id":11,"name":"InputKey","depth":1,"node_count":2,"file_count":1,"criticality":0.36},{"id":12,"name":"MapUVToLocalPosition","depth":1,"node_count":2,"file_count":1,"criticality":0.32}]}
```

MCP `get_affected_flows_tool` for mapper cpp/h:

```json
{"status":"ok","summary":"2 flow(s) affected by changes in 2 file(s)","changed_files":["Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp","Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h"],"affected_flows":[{"id":7,"name":"MapWorldPosition","depth":2,"node_count":4,"file_count":1,"criticality":0.37,"steps":[{"name":"MapWorldPosition","line_start":8,"line_end":23},{"name":"MapLocalPosition","line_start":25,"line_end":82},{"name":"IsValid","line_start":3,"line_end":6},{"name":"UVToPixel","line_start":134,"line_end":139}]},{"id":12,"name":"MapUVToLocalPosition","depth":1,"node_count":2,"file_count":1,"criticality":0.32,"steps":[{"name":"MapUVToLocalPosition","line_start":84,"line_end":132},{"name":"IsValid","line_start":3,"line_end":6}]}],"total":2}
```

MCP `get_impact_radius_tool` for mapper cpp/h:

```json
{"status":"ok","summary":"Blast radius for 2 changed file(s):\n  - 12 nodes directly changed\n  - 35 nodes impacted (within 2 hops)\n  - 21 additional files affected","changed_files":["Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp","Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h"],"total_impacted":35}
```

MCP `detect_changes_tool` for mapper cpp/h:

```json
{"status":"ok","changed_files":["Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp","Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h"],"summary":"Analyzed 2 changed file(s):\n  - 44 changed function(s)/class(es)\n  - 2 affected flow(s)\n  - 44 test gap(s)\n  - Overall risk score: 0.85\n  - Untested: CalculateFittedImageRect, MapLocalPositionToDisplayUV, FHeightmapFittedImageRect, FHeightmapClickMappingResult, class","risk_score":0.85}
```

MCP `get_review_context_tool` for mapper cpp only:

```json
{"status":"ok","summary":"Review context for 1 changed file(s):\n  - Risk: low\n  - 0 impacted nodes in 0 files","risk":"low","changed_file_count":1,"impacted_file_count":0,"key_entities":["J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeCoordinateMapper.cpp","IsValid","MapLocalPosition","MapUVToLocalPosition","MapWorldPosition"],"test_gaps":5}
```

## Rename Preview Caller/Callee Proxy

Query:

```json
{"mode":"rename","old_name":"MapWorldPosition","new_name":"MapWorldPosition_CRGBenchmarkPreview"}
```

Output:

```json
{"status":"ok","summary":"Rename preview: MapWorldPosition -> MapWorldPosition_CRGBenchmarkPreview, 1 edit(s). Use apply_refactor_tool(refactor_id='0781f6df') to apply.","edits":[{"file":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeCoordinateMapper.cpp","line":8,"old":"MapWorldPosition","new":"MapWorldPosition_CRGBenchmarkPreview","confidence":"high"}],"stats":{"high":1,"medium":0,"low":0}}
```

Query:

```json
{"mode":"rename","old_name":"MapLocalPosition","new_name":"MapLocalPosition_CRGBenchmarkPreview"}
```

Output:

```json
{"status":"ok","summary":"Rename preview: MapLocalPosition -> MapLocalPosition_CRGBenchmarkPreview, 2 edit(s). Use apply_refactor_tool(refactor_id='668e71d9') to apply.","edits":[{"file":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeCoordinateMapper.cpp","line":25,"old":"MapLocalPosition","new":"MapLocalPosition_CRGBenchmarkPreview","confidence":"high"},{"file":"J:\\Projects\\UE_Projects\\UE57Editor\\Plugins\\LandscapeHeightmapTracker\\Source\\LandscapeHeightmapTracker\\Private\\LandscapeCoordinateMapper.cpp","line":15,"old":"MapLocalPosition","new":"MapLocalPosition_CRGBenchmarkPreview","confidence":"high"}],"stats":{"high":2,"medium":0,"low":0}}
```

## Source Verification Searches

Command:

```powershell
rg -n "MapWorldPosition|MapLocalPosition|MapUVToLocalPosition|UVToPixel|MapLocalPositionToDisplayUV|CalculateFittedImageRect" Source
```

Relevant output:

```text
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:485:	const FLandscapeTrackerReverseMappingResult ReverseMapping = FLandscapeCoordinateMapper::MapUVToLocalPosition(LocalBounds, DisplayUV, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:508:	LastMapping.Pixel = FLandscapeCoordinateMapper::UVToPixel(ReverseMapping.DisplayUV, ImageSize);
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:59:	const FLandscapeTrackerMappingResult Transformed = FLandscapeCoordinateMapper::MapWorldPosition(Translated, Bounds, World, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:65:	const FLandscapeTrackerMappingResult RotatedResult = FLandscapeCoordinateMapper::MapWorldPosition(Rotated, Bounds, RotatedWorld, ImageSize, Options);
```

Command:

```powershell
rg -n "GetCursorWorldLocationFromMousePos|LineTraceSingleByChannel|LineTraceMultiByChannel|Broadcast|OnViewportClickResult|InputKey|TraceLandscapeClick|ImpactPoint|WorldPosition|MarkerUV|bHasMarker|SetReverseMarker|GetReverseMarker" Source\LandscapeHeightmapTracker
```

Relevant output:

```text
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:14:bool FLandscapeHeightmapTrackerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:20:			TraceLandscapeClick(ViewportClient, Viewport);
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:71:	const FViewportCursorLocation Cursor = ViewportClient->GetCursorWorldLocationFromMousePos();
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:110:	if (!World->LineTraceSingleByChannel(Hit, TraceSegment.Start, TraceSegment.End, ECC_Visibility, QueryParams))
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:124:	Result.WorldPosition = Hit.ImpactPoint;
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:127:	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:176:	ClickDelegateHandle = FLandscapeHeightmapTrackerModule::OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:452:		bHasMarker = true;
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:454:		MarkerUV = LastMapping.NormalizedUV;
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:512:	FLandscapeHeightmapTrackerModule::SetReverseMarker(SurfaceTrace.WorldPosition, AssignedLandscape.Get());
```
