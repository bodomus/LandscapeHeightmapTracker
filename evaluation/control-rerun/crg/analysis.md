# CRG Control Rerun Analysis

## Scope

Evaluated only CRG for Q3, Q5, Q6, and Q10. Production plugin code was not modified. Graphify outputs were not read.

CRG was used first for every question through direct queries against `.code-review-graph/graph.db`, because no CRG MCP tools and no `search`/`query` CLI subcommands were available. Source validation was performed only after CRG produced results.

## Q3 - Cursor Acquisition

CRG found the answer before fallback validation: mostly yes.

Verified runtime path:

```text
FLandscapeHeightmapTrackerEdMode::InputKey
  -> TraceLandscapeClick
  -> FEditorViewportClient::GetCursorWorldLocationFromMousePos
  -> FViewportTraceRayBuilder::BuildTraceSegment
  -> UWorld::LineTraceSingleByChannel
  -> FViewportClickResult.WorldPosition = Hit.ImpactPoint
  -> FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result)
```

Verified source locations:

```text
LandscapeHeightmapTrackerEdMode.cpp:14-25 InputKey intercepts left mouse button press when tracking is enabled.
LandscapeHeightmapTrackerEdMode.cpp:58-129 TraceLandscapeClick performs cursor acquisition, ray build, trace, result fill, broadcast.
ViewportTraceRayBuilder.cpp:3-37 derives Start/End from origin/direction and trace distance.
LandscapeHeightmapTrackerModule.h:12-19 defines FViewportClickResult and delegate type.
LandscapeHeightmapTrackerModule.cpp:60-62 returns the static viewport click delegate.
```

Correct files:

```text
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.cpp
Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp
```

Correct symbols:

```text
FLandscapeHeightmapTrackerEdMode::InputKey
FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick
FViewportTraceRayBuilder::BuildTraceSegment
FLandscapeHeightmapTrackerModule::FViewportClickResult
FLandscapeHeightmapTrackerModule::OnViewportClickResult
```

Missed symbols:

```text
FViewportTraceRayBuilder::BuildTraceSegment was found by exact symbol lookup but not prominently by semantic search.
CRG does not model FEditorViewportClient::GetCursorWorldLocationFromMousePos or UWorld::LineTraceSingleByChannel as project symbols, only unqualified callee strings.
```

False positives:

```text
Semantic search ranked Build.cs and settings nodes above TraceLandscapeClick.
OnMouseButtonDown appeared in cursor semantic results, but it is image-widget click handling, not Unreal Landscape viewport cursor acquisition.
```

Misleading relationships:

```text
Impact BFS from TraceLandscapeClick reached unrelated functions through generic targets like TEXT, GetActor, and GetName. Those are graph proximity artifacts, not runtime flow.
```

Fallback searches required:

```text
rg validation required to verify Result.WorldPosition, ImpactPoint, and delegate broadcast details.
```

Usefulness score: 3/5.

## Q5 - Marker Runtime Path

CRG found the answer before fallback validation: partially.

Verified runtime path responsible for updating the image marker after Landscape interaction is click-driven, not hover/mouse-move-driven:

```text
InputKey(left mouse press)
  -> TraceLandscapeClick
  -> OnViewportClickResult().Broadcast(Result)
  -> SLandscapeHeightmapTrackerPanel::OnViewportClick
  -> RefreshLandscapeBounds
  -> FLandscapeCoordinateMapper::MapWorldPosition
  -> bHasMarker = true
  -> MarkerUV = LastMapping.NormalizedUV
  -> SHeightmapTrackerImageView::OnPaint reads HasMarker/MarkerUV Slate attributes and paints crosshair
```

Verified source locations:

```text
LandscapeHeightmapTrackerEdMode.cpp:16-20 click-only entry.
LandscapeHeightmapTrackerEdMode.cpp:71-127 cursor ray, trace, broadcast.
SLandscapeHeightmapTrackerPanel.cpp:176 delegate AddSP to OnViewportClick.
SLandscapeHeightmapTrackerPanel.cpp:245-251 image widget receives MarkerUV/HasMarker lambdas and OnHeightmapClicked delegate.
SLandscapeHeightmapTrackerPanel.cpp:424-463 OnViewportClick maps the world position and writes bHasMarker/MarkerUV.
SLandscapeHeightmapTrackerPanel.cpp:67-109 OnPaint draws marker if HasMarker.Get() is true.
```

Hover/mouse-move distinction:

```text
No OnMouseMove, MouseMove, hover, or IE_Repeat update path was found. rg only found Tick for reverse marker cleanup and OnMouseButtonDown for image-widget clicks.
```

Reverse path, distinct from viewport marker update:

```text
SHeightmapTrackerImageView::OnMouseButtonDown
  -> FHeightmapImageClickMapper::MapLocalPositionToDisplayUV
  -> SLandscapeHeightmapTrackerPanel::OnHeightmapClicked
  -> FLandscapeCoordinateMapper::MapUVToLocalPosition
  -> LandscapeTransform.TransformPosition
  -> FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface
  -> FLandscapeHeightmapTrackerModule::SetReverseMarker
  -> editor viewport render draws world marker
```

Correct files:

```text
LandscapeHeightmapTrackerEdMode.cpp
LandscapeHeightmapTrackerModule.cpp/h
SLandscapeHeightmapTrackerPanel.cpp/h
LandscapeCoordinateMapper.cpp/h
LandscapeSurfaceTraceHelper.cpp/h for reverse path distinction
```

Correct symbols:

```text
InputKey
TraceLandscapeClick
OnViewportClickResult
SLandscapeHeightmapTrackerPanel::OnViewportClick
FLandscapeCoordinateMapper::MapWorldPosition
SHeightmapTrackerImageView::OnPaint
SHeightmapTrackerImageView::OnMouseButtonDown
SLandscapeHeightmapTrackerPanel::OnHeightmapClicked
SetReverseMarker
```

Missed symbols:

```text
CRG had no edge from Broadcast/AddSP delegate mechanics to OnViewportClick.
CRG had no data-flow edge from OnViewportClick field writes to OnPaint Slate attribute reads.
CRG missed exact qualified static-call edges to MapUVToLocalPosition, TraceAssignedLandscapeSurface, and SetReverseMarker.
```

False positives:

```text
SetReverseMarker flow is relevant only to reverse image-to-landscape behavior, not the click-to-image marker update.
Semantic search ranked OnMouseButtonDown highly for marker update; this can mislead if graph proximity is treated as execution flow.
```

Misleading relationships:

```text
CRG presented local flows such as OnViewportClick and SetReverseMarker, but not the actual cross-file delegate and Slate-attribute runtime flow.
Construct -> OnHeightmapClicked at line 245 is a Slate delegate binding approximation, not an immediate execution call.
```

Fallback searches required:

```text
Source validation was required for delegates, broadcasts, callbacks, state writes, Slate attributes, paint usage, and absence of hover/mouse-move behavior.
```

Usefulness score: 2/5.

## Q6 - Actual Callers

CRG found the real callers before fallback validation: no.

Verified actual callers of `FLandscapeCoordinateMapper::MapWorldPosition`:

```text
Production:
SLandscapeHeightmapTrackerPanel::OnViewportClick at SLandscapeHeightmapTrackerPanel.cpp:449

Tests:
LandscapeCoordinateMapperTests.cpp:59
LandscapeCoordinateMapperTests.cpp:65
```

Definition/declaration, not callers:

```text
LandscapeCoordinateMapper.h:44 declaration.
LandscapeCoordinateMapper.cpp:8 definition.
```

Indirect relationships that are not actual callers:

```text
MapWorldPosition -> MapLocalPosition at LandscapeCoordinateMapper.cpp:15 is a callee edge, not a caller.
MapLocalPosition, UVToPixel, MapUVToLocalPosition are related mapper functions but not callers of MapWorldPosition.
OnViewportClick flow membership is relevant because source line 449 calls MapWorldPosition, but CRG exact caller query returned no rows.
```

CRG false negatives:

```text
Missed production caller SLandscapeHeightmapTrackerPanel.cpp:449.
Missed test callers LandscapeCoordinateMapperTests.cpp:59 and :65.
Did not emit CALLS edges whose target qualified name or target string contains MapWorldPosition.
```

CRG false positives:

```text
No caller false positives for exact MapWorldPosition caller query, because it returned no rows.
Semantic search results were orientation-only and did not identify callers.
```

Parser limitations:

```text
The test file was represented as one RunTest node starting at line 82, while CRG attached TestEqual/TestTrue/TestFalse edges from earlier lines to that node. This line/function association is wrong for the first test bodies.
Qualified static calls of the form FLandscapeCoordinateMapper::MapWorldPosition(...) were not resolved as CALLS edges.
```

Fallback searches required:

```text
Exact rg search was required:
rg -n "MapWorldPosition" Source\LandscapeHeightmapTracker
```

Usefulness score: 1/5.

## Q10 - Cross-Subsystem Connection

CRG found the answer before fallback validation: partially for candidate discovery, no for complete runtime flow.

Verified forward connection from marker widget to Landscape coordinate acquisition:

```text
SHeightmapTrackerImageView is passive for viewport-acquired markers.
Panel construction passes .MarkerUV_Lambda and .HasMarker_Lambda to the widget.
Panel subscribes to FLandscapeHeightmapTrackerModule::OnViewportClickResult with AddSP(...OnViewportClick).
Ed mode acquires Landscape click hit and broadcasts FViewportClickResult.
Panel OnViewportClick calls MapWorldPosition and writes MarkerUV/bHasMarker.
Widget OnPaint reads those attributes and paints marker.
```

Verified reverse connection starting from the widget:

```text
SHeightmapTrackerImageView::OnMouseButtonDown maps image local click to display UV and executes OnHeightmapClicked.
Panel OnHeightmapClicked maps UV to local Landscape position, transforms to world XY, traces assigned Landscape surface, and sets reverse marker.
```

Architecture orientation:

```text
EdMode owns viewport click acquisition.
Module owns shared delegate/tracking/reverse-marker state.
Panel bridges module events, assigned Landscape, coordinate mapper, and image widget state.
Image widget paints and reports image clicks through Slate delegates.
Coordinate mapper is pure conversion logic.
Surface trace helper is only for reverse image-to-landscape resolution.
```

Correct files:

```text
SLandscapeHeightmapTrackerPanel.cpp/h
LandscapeHeightmapTrackerEdMode.cpp/h
LandscapeHeightmapTrackerModule.cpp/h
LandscapeCoordinateMapper.cpp/h
LandscapeSurfaceTraceHelper.cpp/h
```

Correct symbols:

```text
SHeightmapTrackerImageView
SHeightmapTrackerImageView::OnPaint
SHeightmapTrackerImageView::OnMouseButtonDown
SLandscapeHeightmapTrackerPanel::Construct
SLandscapeHeightmapTrackerPanel::OnViewportClick
SLandscapeHeightmapTrackerPanel::OnHeightmapClicked
FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick
FLandscapeCoordinateMapper::MapWorldPosition
FLandscapeCoordinateMapper::MapUVToLocalPosition
FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface
FLandscapeHeightmapTrackerModule::SetReverseMarker
```

Missed symbols:

```text
CRG did not connect delegate broadcast to panel callback.
CRG did not connect Slate attribute lambdas to OnPaint reads.
CRG did not model MarkerUV and bHasMarker as nodes.
```

False positives:

```text
Semantic search ranked Build.cs, command constructor, and settings API macro above key cross-subsystem symbols.
TraceAssignedLandscapeSurface is relevant to reverse widget click path, not the forward viewport-acquired marker path.
```

Misleading relationships:

```text
Graph proximity through generic callees (`TEXT`, `Get`, `IsValid`) creates noisy impact paths and can imply unrelated subsystem links.
Flow and impact outputs are not execution-flow proof for Unreal delegates or Slate attributes.
```

Fallback searches required:

```text
Source reads were required for architecture correctness, callback wiring, runtime-flow correctness, and to separate forward marker display from reverse image click behavior.
```

Usefulness score: 3/5.

## Overall CRG Assessment

CRG was useful for finding candidate files and symbols by exact lookup and for local callee orientation inside a file. Embeddings were active and returned some useful candidates, but ranking was noisy. CRG was weak for the benchmark's critical requirements: actual caller enumeration for qualified static C++ calls, delegate/broadcast runtime flow, Slate attribute data flow, and distinguishing graph proximity from execution flow.

Main limitations observed:

```text
Qualified static calls missed as CALLS edges.
Delegates/broadcasts are not connected to callbacks.
Slate attribute lambdas and field reads/writes are not modeled.
Generic targets such as TEXT, Get, IsValid, GetName, Broadcast, and GetActor create noisy impact paths.
Header parser artifacts and C/C++ language labeling are confusing.
Test function ranges/ownership are inaccurate in LandscapeCoordinateMapperTests.cpp.
Embedding search can rank irrelevant Build.cs/settings nodes above true code-path nodes.
```
