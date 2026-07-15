# CRG Control Rerun Raw Results

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

CRG status query:

```powershell
code-review-graph status --repo .
```

Raw output:

```text
Nodes: 125
Edges: 916
Files: 25
Languages: csharp, cpp, c
Last updated: 2026-07-10T11:48:49
Built on branch: main
Built at commit: 93b214fbe2ea
```

CRG query surface note: `code-review-graph search --help` and `code-review-graph query --help` both failed with `invalid choice`. MCP CRG tools were not exposed in this session. I queried CRG's own `.code-review-graph/graph.db` directly for graph/FTS/embedding data.

Schema/edge-kind query:

```python
SELECT name, type FROM sqlite_master WHERE type IN ('table','view') ORDER BY type,name;
SELECT kind, COUNT(*) FROM edges GROUP BY kind ORDER BY COUNT(*) DESC;
```

Relevant raw output:

```text
tables: communities, community_summaries, edges, embeddings, flow_memberships, flow_snapshots, flows, metadata, nodes, nodes_fts, risk_index
edge kinds:
('CALLS', 715)
('CONTAINS', 105)
('IMPORTS_FROM', 95)
('INHERITS', 1)
```

## Q3 - Cursor Acquisition

Primary CRG symbol lookup query:

```sql
SELECT id, kind, name, qualified_name, file_path, line_start, line_end, is_test
FROM nodes
WHERE name LIKE '%TraceLandscapeClick%' OR qualified_name LIKE '%TraceLandscapeClick%' OR signature LIKE '%TraceLandscapeClick%'
ORDER BY is_test, file_path, line_start LIMIT 30;
```

First useful raw result:

```text
{'id': 154, 'kind': 'Function', 'name': 'TraceLandscapeClick', 'qualified_name': '...\Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp::TraceLandscapeClick', 'file_path': '...\Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp', 'line_start': 58, 'line_end': 129, 'is_test': 0}
```

Caller/callee query:

```sql
SELECT kind, source_qualified, target_qualified, file_path, line, confidence_tier
FROM edges
WHERE kind='CALLS' AND (source_qualified=? OR target_qualified=?)
ORDER BY file_path,line;
-- ? = ...\LandscapeHeightmapTrackerEdMode.cpp::TraceLandscapeClick
```

Relevant raw output:

```text
CALLER:
InputKey -> TraceLandscapeClick at LandscapeHeightmapTrackerEdMode.cpp:20

CALLEES:
TraceLandscapeClick -> GetCursorWorldLocationFromMousePos at LandscapeHeightmapTrackerEdMode.cpp:71
TraceLandscapeClick -> IsPerspective at LandscapeHeightmapTrackerEdMode.cpp:73
TraceLandscapeClick -> GetOrigin at LandscapeHeightmapTrackerEdMode.cpp:74
TraceLandscapeClick -> GetDirection at LandscapeHeightmapTrackerEdMode.cpp:75
TraceLandscapeClick -> LineTraceSingleByChannel at LandscapeHeightmapTrackerEdMode.cpp:110
TraceLandscapeClick -> Broadcast at LandscapeHeightmapTrackerEdMode.cpp:127
```

Flow query:

```sql
SELECT id,name,depth,node_count,file_count,criticality,path_json
FROM flows
WHERE name LIKE '%InputKey%' OR path_json LIKE '%151%'
ORDER BY criticality DESC;
```

Raw output:

```text
{'id': 26, 'name': 'InputKey', 'depth': 1, 'node_count': 2, 'file_count': 1, 'criticality': 0.36, 'path_json': '[151, 154]'}
```

Dependency query:

```sql
SELECT kind,source_qualified,target_qualified,file_path,line
FROM edges
WHERE kind='IMPORTS_FROM'
AND file_path LIKE '%LandscapeHeightmapTrackerEdMode%'
ORDER BY line;
```

Relevant raw output included imports from `LandscapeHeightmapTrackerModule.h` and `ViewportTraceRayBuilder.h`.

Semantic search query:

```python
query = "code that reads current cursor position over Unreal Landscape viewport ray trace mouse position"
# embedded with sentence-transformers all-MiniLM-L6-v2, cosine-ranked against CRG embeddings
```

Top relevant raw output:

```text
rank 4: score 0.3839164674 Function TraceLandscapeClick ...\LandscapeHeightmapTrackerEdMode.cpp:58-129
rank 6: score 0.3798518479 Function OnMouseButtonDown ...\SLandscapeHeightmapTrackerPanel.cpp:111-139
rank 11: score 0.3438008428 Function MapWorldPosition ...\LandscapeCoordinateMapper.cpp:8-23
```

Source validation searches:

```powershell
rg -n "TraceLandscapeClick|InputKey|GetCursorWorldLocationFromMousePos|BuildTraceSegment|LineTraceSingleByChannel|OnViewportClickResult|Broadcast|WorldPosition|ImpactPoint" Source\LandscapeHeightmapTracker
```

Relevant raw output:

```text
LandscapeHeightmapTrackerEdMode.cpp:14:bool FLandscapeHeightmapTrackerEdMode::InputKey(...)
LandscapeHeightmapTrackerEdMode.cpp:20: TraceLandscapeClick(ViewportClient, Viewport);
LandscapeHeightmapTrackerEdMode.cpp:71: const FViewportCursorLocation Cursor = ViewportClient->GetCursorWorldLocationFromMousePos();
LandscapeHeightmapTrackerEdMode.cpp:72: const FViewportTraceSegment TraceSegment = FViewportTraceRayBuilder::BuildTraceSegment(
LandscapeHeightmapTrackerEdMode.cpp:110: if (!World->LineTraceSingleByChannel(...)
LandscapeHeightmapTrackerEdMode.cpp:124: Result.WorldPosition = Hit.ImpactPoint;
LandscapeHeightmapTrackerEdMode.cpp:127: FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
```

Source files opened:

```text
LandscapeHeightmapTrackerEdMode.cpp:14-129
ViewportTraceRayBuilder.cpp:3-37
LandscapeHeightmapTrackerModule.h:12-24
LandscapeHeightmapTrackerModule.cpp:60-62
```

## Q5 - Marker Runtime Path

Primary CRG symbol/callee queries:

```sql
SELECT id, kind, name, qualified_name, file_path, line_start, line_end, is_test
FROM nodes
WHERE name IN ('OnViewportClick','MapWorldPosition','OnPaint','OnMouseButtonDown','OnHeightmapClicked','SetReverseMarker','TraceAssignedLandscapeSurface')
ORDER BY file_path,line_start;

SELECT source_qualified,target_qualified,file_path,line
FROM edges
WHERE kind='CALLS' AND source_qualified=? ORDER BY line,target_qualified;
```

First useful raw results:

```text
OnViewportClick: ...\SLandscapeHeightmapTrackerPanel.cpp::OnViewportClick line_start 424 line_end 463
MapWorldPosition: ...\LandscapeCoordinateMapper.cpp::MapWorldPosition line_start 8 line_end 23
OnPaint: ...\SLandscapeHeightmapTrackerPanel.cpp::SHeightmapTrackerImageView.OnPaint line_start 67 line_end 109
TraceLandscapeClick: ...\LandscapeHeightmapTrackerEdMode.cpp::TraceLandscapeClick line_start 58 line_end 129
```

Relevant CRG raw caller/callee output:

```text
InputKey -> TraceLandscapeClick at LandscapeHeightmapTrackerEdMode.cpp:20
TraceLandscapeClick -> Broadcast at LandscapeHeightmapTrackerEdMode.cpp:127
OnViewportClick -> MapWorldPosition at SLandscapeHeightmapTrackerPanel.cpp:449
MapWorldPosition -> MapLocalPosition at LandscapeCoordinateMapper.cpp:15
MapLocalPosition -> UVToPixel at LandscapeCoordinateMapper.cpp:79
Construct -> OnHeightmapClicked at SLandscapeHeightmapTrackerPanel.cpp:245
SetReverseMarker -> UpdateEditorModeActivation at LandscapeHeightmapTrackerModule.cpp:82
SetReverseMarker -> RequestViewportRedraw at LandscapeHeightmapTrackerModule.cpp:83
```

Flow query:

```sql
SELECT id,name,depth,node_count,file_count,criticality,path_json
FROM flows
WHERE name LIKE '%OnViewportClick%' OR name LIKE '%MapWorldPosition%' OR name LIKE '%SetReverseMarker%'
ORDER BY criticality DESC;
```

Raw output:

```text
{'id': 20, 'name': 'OnViewportClick', 'depth': 1, 'node_count': 4, 'file_count': 1, 'criticality': 0.4225, 'path_json': '[200, 209, 206, 210]'}
{'id': 22, 'name': 'MapWorldPosition', 'depth': 2, 'node_count': 4, 'file_count': 1, 'criticality': 0.37, 'path_json': '[143, 144, 142, 146]'}
{'id': 17, 'name': 'SetReverseMarker', 'depth': 1, 'node_count': 3, 'file_count': 1, 'criticality': 0.4433, 'path_json': '[162, 158, 159]'}
```

Semantic search query:

```python
query = "update image marker after landscape interaction click hover mouse move delegate broadcast slate paint marker uv"
```

Top relevant raw output:

```text
rank 1: score 0.4707984924 Function OnPaint ...\SLandscapeHeightmapTrackerPanel.cpp:67-109
rank 2: score 0.4274241924 Function OnMouseButtonDown ...\SLandscapeHeightmapTrackerPanel.cpp:111-139
rank 8: score 0.3153145611 Class SHeightmapTrackerImageView ...\SLandscapeHeightmapTrackerPanel.cpp:40-148
rank 10: score 0.3073243797 Function ClearMarker ...\SLandscapeHeightmapTrackerPanel.cpp:408-417
rank 11: score 0.3051953912 Function TraceLandscapeClick ...\LandscapeHeightmapTrackerEdMode.cpp:58-129
```

Source validation searches:

```powershell
rg -n "OnViewportClick|OnHeightmapClicked|MarkerUV|bHasMarker|OnPaint|OnMouseButtonDown|MapWorldPosition|MapUVToLocalPosition|TraceAssignedLandscapeSurface|SetReverseMarker|AddRaw|RemoveAll|Broadcast" Source\LandscapeHeightmapTracker
rg -n "MouseMove|OnMouseMove|IE_Repeat|Mouse|Hover|Cursor|Tick\(" Source\LandscapeHeightmapTracker\Private Source\LandscapeHeightmapTracker\Public
```

Relevant raw output:

```text
SLandscapeHeightmapTrackerPanel.cpp:176: ClickDelegateHandle = ...OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
SLandscapeHeightmapTrackerPanel.cpp:247: .MarkerUV_Lambda([this]() { return MarkerUV; })
SLandscapeHeightmapTrackerPanel.cpp:248: .HasMarker_Lambda([this]() { return bHasMarker; })
SLandscapeHeightmapTrackerPanel.cpp:424:void SLandscapeHeightmapTrackerPanel::OnViewportClick(...)
SLandscapeHeightmapTrackerPanel.cpp:449: LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(...)
SLandscapeHeightmapTrackerPanel.cpp:452: bHasMarker = true;
SLandscapeHeightmapTrackerPanel.cpp:454: MarkerUV = LastMapping.NormalizedUV;
SLandscapeHeightmapTrackerPanel.cpp:67: virtual int32 OnPaint(...)
SLandscapeHeightmapTrackerPanel.cpp:87: if (HasMarker.Get())
SLandscapeHeightmapTrackerPanel.cpp:89: const FVector2D UV = MarkerUV.Get();
SLandscapeHeightmapTrackerPanel.cpp:111: virtual FReply OnMouseButtonDown(...)
SLandscapeHeightmapTrackerPanel.cpp:465:void SLandscapeHeightmapTrackerPanel::OnHeightmapClicked(FVector2D DisplayUV)
SLandscapeHeightmapTrackerPanel.cpp:485: ...MapUVToLocalPosition(...)
SLandscapeHeightmapTrackerPanel.cpp:495: ...TraceAssignedLandscapeSurface(...)
SLandscapeHeightmapTrackerPanel.cpp:512: ...SetReverseMarker(...)
LandscapeHeightmapTrackerEdMode.cpp:16: if (Key == EKeys::LeftMouseButton && Event == IE_Pressed)
```

Source files opened:

```text
LandscapeHeightmapTrackerEdMode.cpp:14-129
SLandscapeHeightmapTrackerPanel.cpp:40-148, 164-177, 245-251, 331-333, 424-527
LandscapeCoordinateMapper.cpp:8-82, 84-139
LandscapeHeightmapTrackerModule.cpp:20-84, 151-157
LandscapeSurfaceTraceHelper.cpp:49-111
```

## Q6 - Actual Callers

Primary CRG caller query:

```sql
SELECT source_qualified,target_qualified,file_path,line,confidence_tier
FROM edges
WHERE kind='CALLS' AND target_qualified = '...\LandscapeCoordinateMapper.cpp::MapWorldPosition'
ORDER BY file_path,line;
```

Raw output:

```text
CALLERS by exact qualified target
<no rows>
```

Fallback graph query:

```sql
SELECT source_qualified,target_qualified,file_path,line,confidence_tier
FROM edges
WHERE kind='CALLS' AND target_qualified LIKE '%MapWorldPosition%'
ORDER BY file_path,line;
```

Raw output:

```text
CALL EDGES where target LIKE MapWorldPosition
<no rows>
```

Related CRG callee query for internal call:

```sql
SELECT source_qualified,file_path,line,target_qualified
FROM edges
WHERE kind='CALLS' AND target_qualified = '...\LandscapeCoordinateMapper.cpp::MapLocalPosition'
ORDER BY file_path,line;
```

Raw output:

```text
MapWorldPosition -> MapLocalPosition at LandscapeCoordinateMapper.cpp:15
```

Semantic search query:

```python
query = "callers of FLandscapeCoordinateMapper MapWorldPosition coordinate mapper production tests"
```

Top relevant raw output:

```text
rank 3: score 0.5386905670 Function MapWorldPosition ...\LandscapeCoordinateMapper.cpp:8-23
rank 5: score 0.4744347930 Function MapLocalPosition ...\LandscapeCoordinateMapper.cpp:25-82
rank 7: score 0.4334684312 Function MapUVToLocalPosition ...\LandscapeCoordinateMapper.cpp:84-132
rank 11: score 0.3836262226 Function IsAssignedLandscapeHit ...\SLandscapeHeightmapTrackerPanel.cpp:659-667
```

Source validation search:

```powershell
rg -n "MapWorldPosition" Source\LandscapeHeightmapTracker
```

Raw output:

```text
Source\LandscapeHeightmapTracker\Public\LandscapeCoordinateMapper.h:44: static FLandscapeTrackerMappingResult MapWorldPosition(
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:8:FLandscapeTrackerMappingResult FLandscapeCoordinateMapper::MapWorldPosition(
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449: LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(...)
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:59: const FLandscapeTrackerMappingResult Transformed = FLandscapeCoordinateMapper::MapWorldPosition(...)
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:65: const FLandscapeTrackerMappingResult RotatedResult = FLandscapeCoordinateMapper::MapWorldPosition(...)
```

Source files opened:

```text
LandscapeCoordinateMapper.cpp:8-23
SLandscapeHeightmapTrackerPanel.cpp:424-463
Tests/LandscapeCoordinateMapperTests.cpp:56-67
```

## Q10 - Cross-Subsystem Connection

Primary CRG symbol lookup query:

```sql
SELECT id, kind, name, qualified_name, file_path, line_start, line_end
FROM nodes
WHERE name LIKE '%SHeightmapTrackerImageView%' OR qualified_name LIKE '%SHeightmapTrackerImageView%'
ORDER BY file_path,line_start;
```

First useful raw result:

```text
Class SHeightmapTrackerImageView ...\SLandscapeHeightmapTrackerPanel.cpp:40-148
Function SHeightmapTrackerImageView.OnPaint ...\SLandscapeHeightmapTrackerPanel.cpp:67-109
Function SHeightmapTrackerImageView.OnMouseButtonDown ...\SLandscapeHeightmapTrackerPanel.cpp:111-139
```

Cross-subsystem CRG edge queries:

```sql
SELECT source_qualified,target_qualified,file_path,line
FROM edges
WHERE kind='CALLS'
AND (source_qualified LIKE '%SHeightmapTrackerImageView%' OR target_qualified LIKE '%SHeightmapTrackerImageView%' OR source_qualified LIKE '%OnViewportClick%' OR target_qualified LIKE '%OnViewportClick%')
ORDER BY file_path,line;
```

Relevant raw output:

```text
OnViewportClick -> MapWorldPosition at SLandscapeHeightmapTrackerPanel.cpp:449
Construct -> OnHeightmapClicked at SLandscapeHeightmapTrackerPanel.cpp:245
<no direct graph edge from TraceLandscapeClick/Broadcast to OnViewportClick>
<no direct graph edge from SHeightmapTrackerImageView.OnPaint to OnViewportClick>
```

Semantic search query:

```python
query = "heightmap marker widget connection to landscape coordinate acquisition viewport trace panel delegate"
```

Top relevant raw output:

```text
rank 6: score 0.4096074104 Function TraceAssignedLandscapeSurface ...\LandscapeSurfaceTraceHelper.cpp:49-111
rank 8: score 0.3927943707 Function TraceLandscapeClick ...\LandscapeHeightmapTrackerEdMode.cpp:58-129
rank 9: score 0.3707966805 Function OnHeightmapClicked ...\SLandscapeHeightmapTrackerPanel.cpp:465-527
rank 10: score 0.3624250591 Function RefreshLandscapeBounds ...\SLandscapeHeightmapTrackerPanel.cpp:571-582
rank 12: score 0.3347584009 Function OnPaint ...\SLandscapeHeightmapTrackerPanel.cpp:67-109
```

Source validation searches:

```powershell
rg -n "MarkerUV|HasMarker|OnHeightmapClicked|OnViewportClickResult|TraceLandscapeClick|SetReverseMarker" Source\LandscapeHeightmapTracker
```

Relevant raw output:

```text
LandscapeHeightmapTrackerEdMode.cpp:20: TraceLandscapeClick(ViewportClient, Viewport);
LandscapeHeightmapTrackerEdMode.cpp:127: FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
SLandscapeHeightmapTrackerPanel.cpp:176: ...OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
SLandscapeHeightmapTrackerPanel.cpp:245: SNew(SHeightmapTrackerImageView)
SLandscapeHeightmapTrackerPanel.cpp:247: .MarkerUV_Lambda([this]() { return MarkerUV; })
SLandscapeHeightmapTrackerPanel.cpp:248: .HasMarker_Lambda([this]() { return bHasMarker; })
SLandscapeHeightmapTrackerPanel.cpp:251: .OnHeightmapClicked(...CreateSP(this, &SLandscapeHeightmapTrackerPanel::OnHeightmapClicked))
SLandscapeHeightmapTrackerPanel.cpp:449: ...MapWorldPosition(...)
SLandscapeHeightmapTrackerPanel.cpp:452: bHasMarker = true;
SLandscapeHeightmapTrackerPanel.cpp:454: MarkerUV = LastMapping.NormalizedUV;
```

Change-related investigation query:

```powershell
$env:PYTHONIOENCODING='utf-8'; code-review-graph detect-changes --repo . --brief
```

Raw output:

```text
Analyzed 22 changed file(s):
  - 44 changed function(s)/class(es)
  - 0 affected flow(s)
  - 44 test gap(s)
  - Overall risk score: 0.85
  - Untested: CalculateFittedImageRect, MapLocalPositionToDisplayUV, FHeightmapFittedImageRect, FHeightmapClickMappingResult, class
Token Savings:
Full context would be: 28,966 tokens
Graph context used: 11,668 tokens
Saved: 17,298 tokens (~60%)
```

Impact radius query:

```python
# BFS to depth 2 over CRG CALLS/IMPORTS_FROM edges from MapWorldPosition, TraceLandscapeClick, OnViewportClick, SHeightmapTrackerImageView
```

Relevant raw output:

```text
from MapWorldPosition depth 1:
  -> InverseTransformPosition at LandscapeCoordinateMapper.cpp:17
  -> MapLocalPosition at LandscapeCoordinateMapper.cpp:15
from MapWorldPosition depth 2:
  MapLocalPosition -> IsValid at LandscapeCoordinateMapper.cpp:34
  MapLocalPosition -> UVToPixel at LandscapeCoordinateMapper.cpp:79

from TraceLandscapeClick depth 1:
  -> InputKey at LandscapeHeightmapTrackerEdMode.cpp:20
  -> Broadcast at LandscapeHeightmapTrackerEdMode.cpp:127
  -> GetCursorWorldLocationFromMousePos at LandscapeHeightmapTrackerEdMode.cpp:71
  -> LineTraceSingleByChannel at LandscapeHeightmapTrackerEdMode.cpp:110

from OnViewportClick depth 1:
  -> MapWorldPosition at SLandscapeHeightmapTrackerPanel.cpp:449
  -> RefreshLandscapeBounds at SLandscapeHeightmapTrackerPanel.cpp:444
  -> IsAssignedLandscapeHit at SLandscapeHeightmapTrackerPanel.cpp:438
```

Source files opened:

```text
SLandscapeHeightmapTrackerPanel.cpp:40-148, 164-177, 245-251, 424-527
LandscapeHeightmapTrackerEdMode.cpp:14-129
LandscapeHeightmapTrackerModule.cpp:20-84
LandscapeCoordinateMapper.cpp:8-139
LandscapeSurfaceTraceHelper.cpp:49-111
```
