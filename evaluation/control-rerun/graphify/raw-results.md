# Graphify Raw Results - Controlled Rerun

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

Constraint note: Graphify's refreshed graph contains older `evaluation/crg/*` and original evaluation artifacts. Some `graphify query` output surfaced those nodes. I did not use or copy CRG content as evidence; the relevant raw excerpts below are limited to Graphify commands and source-code nodes/relationships, plus validation searches under `Source/`.

## Graph Presence

Command:
```powershell
Test-Path -LiteralPath 'graphify-out\graph.json'; if (Test-Path -LiteralPath 'graphify-out\graph.json') { Get-Item -LiteralPath 'graphify-out\graph.json' | Select-Object FullName,Length,LastWriteTime | Format-List }
```

Relevant output:
```text
True

FullName      : J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\graphify-out\graph.json
Length        : 522714
LastWriteTime : 7/10/2026 11:46:38 AM
```

## Q3 - Cursor Acquisition

Primary Graphify command:
```powershell
graphify query "Q3 Cursor acquisition: Which code reads or derives the current cursor position over the Unreal Landscape? Identify exact entry point, viewport interaction, cursor ray acquisition, tracing, and result propagation." --budget 3000
```

Relevant Graphify output excerpt:
```text
Traversal: BFS depth=2 | ... | 193 nodes found

NODE SLandscapeHeightmapTrackerPanel [src=Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h loc=L11 community=0]
NODE SLandscapeHeightmapTrackerPanel.cpp [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L1 community=2]
NODE LandscapeHeightmapTrackerModule.cpp [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L1 community=3]
NODE SHeightmapTrackerImageView [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L40 community=2]
NODE SLandscapeHeightmapTrackerPanel::Construct() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L164 community=2]
NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
NODE LandscapeSurfaceTraceHelper.h [src=Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.h loc=L1 community=5]
NODE InputKey [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L9 community=7]
NODE FLandscapeCoordinateMapper::MapLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L25 community=1]
NODE BuildTraceSegment [src=Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.h loc=L20 community=9]
NODE FViewportTraceSegment [src=Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.h loc=L5 community=9]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE TraceLandscapeClick [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L14 community=7]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=1]
NODE FLandscapeHeightmapTrackerEdMode [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L6 community=7]
NODE SLandscapeHeightmapTrackerPanel::IsAssignedLandscapeHit() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L659 community=5]
```

Validation commands:
```powershell
rg -n -C 8 "InputKey|TraceLandscapeClick|GetCursorWorldLocationFromMousePos|BuildTraceSegment|LineTraceSingleByChannel|Broadcast" Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp
rg -n -C 6 "FViewportTraceSegment|BuildTraceSegment" Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.h Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp
rg -n -C 6 "DECLARE_MULTICAST_DELEGATE|FViewportClickResult|OnViewportClickResult|SetTrackingModeEnabled|SetReverseMarker|GetReverseMarker" Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp
```

Relevant validation output:
```text
14:bool FLandscapeHeightmapTrackerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
16-	if (Key == EKeys::LeftMouseButton && Event == IE_Pressed)
18-		if (FLandscapeHeightmapTrackerModule::IsTrackingModeEnabled())
20:			TraceLandscapeClick(ViewportClient, Viewport);

58:bool FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick(FEditorViewportClient* ViewportClient, FViewport* Viewport) const
71:	const FViewportCursorLocation Cursor = ViewportClient->GetCursorWorldLocationFromMousePos();
72:	const FViewportTraceSegment TraceSegment = FViewportTraceRayBuilder::BuildTraceSegment(
73-		ViewportClient->IsPerspective(),
74-		Cursor.GetOrigin(),
75-		Cursor.GetDirection());
110:	if (!World->LineTraceSingleByChannel(Hit, TraceSegment.Start, TraceSegment.End, ECC_Visibility, QueryParams))
123-	FLandscapeHeightmapTrackerModule::FViewportClickResult Result;
124-	Result.WorldPosition = Hit.ImpactPoint;
125-	Result.HitActor = Hit.GetActor();
126-	Result.HitComponent = Hit.GetComponent();
127:	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);

Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp:3:FViewportTraceSegment FViewportTraceRayBuilder::BuildTraceSegment(
Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp-9-	FViewportTraceSegment Segment;
Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp-20-	Segment.Direction = Direction.GetSafeNormal();
Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp-21-	Segment.Start = Origin;
Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp-22-	Segment.End = Origin + Segment.Direction * TraceDistance;
Source\LandscapeHeightmapTracker\Private\ViewportTraceRayBuilder.cpp-24-	if (!bIsPerspective)

Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h:12:	struct FViewportClickResult
Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h:19:	DECLARE_MULTICAST_DELEGATE_OneParam(FOnViewportClickResult, const FViewportClickResult&);
Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h:24:	static FOnViewportClickResult& OnViewportClickResult();
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:20:static FLandscapeHeightmapTrackerModule::FOnViewportClickResult GOnViewportClickResult;
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:60:FLandscapeHeightmapTrackerModule::FOnViewportClickResult& FLandscapeHeightmapTrackerModule::OnViewportClickResult()
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:62:	return GOnViewportClickResult;
```

## Q5 - Marker Runtime Path

Primary Graphify command:
```powershell
graphify query "Q5 Marker runtime path: Trace the verified runtime path responsible for updating the image marker after Landscape interaction. Distinguish click-driven behavior from hover mouse-move behavior; verify delegates broadcasts callbacks state writes Slate attributes paint usage." --budget 4000
```

Relevant Graphify output excerpt:
```text
Traversal: BFS depth=2 | ... | 233 nodes found

NODE SLandscapeHeightmapTrackerPanel [src=Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h loc=L11 community=0]
NODE SLandscapeHeightmapTrackerPanel.cpp [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L1 community=2]
NODE LandscapeHeightmapTrackerModule.cpp [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L1 community=3]
NODE SHeightmapTrackerImageView [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L40 community=2]
NODE SLandscapeHeightmapTrackerPanel::Construct() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L164 community=2]
NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
NODE TraceAssignedLandscapeSurface [src=Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.h loc=L40 community=5]
NODE .OnPaint() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L67 community=12]
NODE InputKey [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L9 community=7]
NODE CalculateFittedImageRect [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L22 community=12]
NODE MapLocalPositionToDisplayUV [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L26 community=12]
NODE TraceLandscapeClick [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L14 community=7]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=1]
NODE .OnMouseButtonDown() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L111 community=12]
NODE FLandscapeHeightmapTrackerModule::SetReverseMarker() [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L76 community=3]
NODE FLandscapeHeightmapTrackerModule::ClearReverseMarker() [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L86 community=3]
```

Validation commands:
```powershell
rg -n -C 8 "AddSP|OnViewportClick\(|MarkerUV|bHasMarker|OnPaint|OnMouseButtonDown|OnHeightmapClicked|SetReverseMarker|GetReverseMarker|Render\(" Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp
rg -n "OnMouseMove|MouseMove|IE_Repeat|IE_Axis|MouseMove|Moved|Hover|OnCursor" Source
```

Relevant validation output:
```text
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:176:	ClickDelegateHandle = FLandscapeHeightmapTrackerModule::OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:247:					.MarkerUV_Lambda([this]() { return MarkerUV; })
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:248:					.HasMarker_Lambda([this]() { return bHasMarker; })
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:251:					.OnHeightmapClicked(TDelegate<void(FVector2D)>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::OnHeightmapClicked))
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:424:void SLandscapeHeightmapTrackerPanel::OnViewportClick(const FLandscapeHeightmapTrackerModule::FViewportClickResult& Click)
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp-449-	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:452:		bHasMarker = true;
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:454:		MarkerUV = LastMapping.NormalizedUV;
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:67:	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:89:			const FVector2D UV = MarkerUV.Get();
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp-90-			const FVector2D MarkerCenter = ImageRect.DrawOffset + FVector2D(UV.X * ImageRect.DrawSize.X, UV.Y * ImageRect.DrawSize.Y);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:111:	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:127:		const FHeightmapClickMappingResult ClickMapping = FHeightmapImageClickMapper::MapLocalPositionToDisplayUV(LocalPosition, ImageRect);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:131:			OnHeightmapClicked.ExecuteIfBound(ClickMapping.DisplayUV);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:465:void SLandscapeHeightmapTrackerPanel::OnHeightmapClicked(FVector2D DisplayUV)
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:495:	const FLandscapeSurfaceTraceResult SurfaceTrace = FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface(World, AssignedLandscape.Get(), WorldXY);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:512:	FLandscapeHeightmapTrackerModule::SetReverseMarker(SurfaceTrace.WorldPosition, AssignedLandscape.Get());
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:38:void FLandscapeHeightmapTrackerEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:48:	if (!FLandscapeHeightmapTrackerModule::GetReverseMarker(MarkerBase))
```

Hover/mouse-move absence check:
```text
rg exited 1 with no matches.
```

## Q6 - Actual Callers

Primary Graphify command:
```powershell
graphify query "Q6 Actual callers: Find all actual callers of FLandscapeCoordinateMapper::MapWorldPosition. Classify production callers test callers indirect relationships not actual callers false positives missed callers." --budget 3000
```

Relevant Graphify output excerpt:
```text
Traversal: BFS depth=2 | ... | 93 nodes found

NODE SLandscapeHeightmapTrackerPanel.cpp [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L1 community=2]
NODE FLandscapeTrackerMappingResult [src=Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h loc=L21 community=1]
NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE FLandscapeCoordinateMapper::MapLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L25 community=1]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=1]
NODE FLandscapeCoordinateMapper [src=Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h loc=L41 community=1]
NODE LandscapeCoordinateMapperTests.cpp [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L1 community=1]
NODE FLandscapeCoordinateMapperOptionsTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L35 community=1]
NODE FLandscapeCoordinateMapperBasicTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L8 community=1]
NODE MapWorldPosition [src=Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h loc=L44 community=1]
NODE FLandscapeCoordinateMapperReverseMappingTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L82 community=1]
```

Additional Graphify command:
```powershell
graphify explain "FLandscapeCoordinateMapper::MapWorldPosition"
```

Output:
```text
Node: FLandscapeCoordinateMapper::MapWorldPosition()
  ID:        source_landscapeheightmaptracker_private_landscapecoordinatemapper_flandscapecoordinatemapper_mapworldposition
  Source:    Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp L8
  Type:      code
  Community: 1
  Degree:    8

Connections (8):
  --> FLandscapeTrackerMappingResult [references] [EXTRACTED]
  --> FLandscapeTrackerBounds [references] [EXTRACTED]
  --> FLandscapeTrackerMappingOptions [references] [EXTRACTED]
  <-- LandscapeCoordinateMapper.cpp [contains] [EXTRACTED]
  --> MapLocalPosition [calls] [INFERRED]
  --> FIntPoint [references] [EXTRACTED]
  --> FVector [references] [EXTRACTED]
  --> FTransform [references] [EXTRACTED]
```

Validation command:
```powershell
rg -n "MapWorldPosition" Source
```

Validation output:
```text
Source\LandscapeHeightmapTracker\Public\LandscapeCoordinateMapper.h:44:	static FLandscapeTrackerMappingResult MapWorldPosition(
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:8:FLandscapeTrackerMappingResult FLandscapeCoordinateMapper::MapWorldPosition(
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:59:	const FLandscapeTrackerMappingResult Transformed = FLandscapeCoordinateMapper::MapWorldPosition(Translated, Bounds, World, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:65:	const FLandscapeTrackerMappingResult RotatedResult = FLandscapeCoordinateMapper::MapWorldPosition(Rotated, Bounds, RotatedWorld, ImageSize, Options);
```

## Q10 - Cross-Subsystem Connection

Primary Graphify command:
```powershell
graphify query "Q10 Cross-subsystem connection: Starting from the heightmap marker widget, identify the actual connection to Landscape coordinate acquisition. Evaluate candidate discovery architecture orientation runtime-flow correctness irrelevant files additional source reads needed." --budget 4000
```

Relevant Graphify output excerpt:
```text
Traversal: BFS depth=2 | ... | 181 nodes found

NODE SLandscapeHeightmapTrackerPanel [src=Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h loc=L11 community=0]
NODE SLandscapeHeightmapTrackerPanel.cpp [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L1 community=2]
NODE LandscapeHeightmapTrackerModule.cpp [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L1 community=3]
NODE SHeightmapTrackerImageView [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L40 community=2]
NODE SLandscapeHeightmapTrackerPanel::Construct() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L164 community=2]
NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
NODE .OnPaint() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L67 community=12]
NODE LandscapeSurfaceTraceHelper.h [src=Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.h loc=L1 community=5]
NODE InputKey [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L9 community=7]
NODE FHeightmapClickMappingResult [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L12 community=12]
NODE CalculateFittedImageRect [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L22 community=12]
NODE MapLocalPositionToDisplayUV [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L26 community=12]
NODE TraceLandscapeClick [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L14 community=7]
NODE .OnMouseButtonDown() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L111 community=12]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=1]
NODE FLandscapeHeightmapTrackerModule::SetReverseMarker() [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L76 community=3]
```

Targeted Graphify path commands:
```powershell
graphify path "SHeightmapTrackerImageView" "FLandscapeCoordinateMapper::MapWorldPosition"
graphify path "SHeightmapTrackerImageView" "FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick"
graphify explain "SHeightmapTrackerImageView"
```

Outputs:
```text
Shortest path (4 hops):
  SHeightmapTrackerImageView --references [EXTRACTED]--> FVector2D <--references [EXTRACTED]-- SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() --calls [INFERRED]--> FLandscapeTrackerMappingResult <--references [EXTRACTED]-- FLandscapeCoordinateMapper::MapWorldPosition()

Shortest path (4 hops):
  SHeightmapTrackerImageView --defines [EXTRACTED]--> OnHeightmapClicked <--calls [EXTRACTED]-- SLandscapeHeightmapTrackerPanel::Construct() --references [EXTRACTED]--> FLandscapeHeightmapTrackerModule <--references [EXTRACTED]-- TraceLandscapeClick
warning: target match was ambiguous (top score 5531.41, runner-up 5531.41)

Node: SHeightmapTrackerImageView
  ID:        source_landscapeheightmaptracker_private_slandscapeheightmaptrackerpanel_sheightmaptrackerimageview
  Source:    Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp L40
  Type:      code
  Community: 2
  Degree:    18

Connections (18):
  <-- SLandscapeHeightmapTrackerPanel.cpp [contains] [EXTRACTED]
  --> .OnPaint() [method] [EXTRACTED]
  --> FVector2D [references] [EXTRACTED]
  --> .OnMouseButtonDown() [method] [EXTRACTED]
  --> SCompoundWidget [inherits] [EXTRACTED]
  --> .ComputeDesiredSize() [method] [EXTRACTED]
  --> .Construct() [method] [EXTRACTED]
  --> OnHeightmapClicked [defines] [EXTRACTED]
  --> OnOutsideImageClicked [defines] [EXTRACTED]
  --> OnUnavailableClicked [defines] [EXTRACTED]
  --> TAttribute [references] [EXTRACTED]
  --> FSimpleDelegate [references] [EXTRACTED]
  --> FSlateBrush [references] [EXTRACTED]
  --> HasMarker [defines] [EXTRACTED]
  --> ImageBrush [defines] [EXTRACTED]
  --> MarkerUV [defines] [EXTRACTED]
  --> TDelegate [references] [EXTRACTED]
  <-- .SLATE_BEGIN_ARGS() [references] [EXTRACTED]
```

Relevant validation commands:
```powershell
rg -n -C 5 "class SLandscapeHeightmapTrackerPanel|OnViewportClick|OnHeightmapClicked|MarkerUV|bHasMarker|LastMapping|ClickDelegateHandle" Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h
rg -n -C 8 "AddSP|OnViewportClick\(|MarkerUV|bHasMarker|OnPaint|OnMouseButtonDown|OnHeightmapClicked|SetReverseMarker|GetReverseMarker|Render\(" Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp
```

Relevant validation output:
```text
Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h:25:	void OnViewportClick(const FLandscapeHeightmapTrackerModule::FViewportClickResult& Click);
Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h:26:	void OnHeightmapClicked(FVector2D DisplayUV);
Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h:55:	FLandscapeTrackerMappingResult LastMapping;
Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h:56:	FVector2D MarkerUV = FVector2D::ZeroVector;
Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h:62:	bool bHasMarker = false;
Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h:70:	FDelegateHandle ClickDelegateHandle;

Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:176:	ClickDelegateHandle = FLandscapeHeightmapTrackerModule::OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:245:					SNew(SHeightmapTrackerImageView)
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:247:					.MarkerUV_Lambda([this]() { return MarkerUV; })
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:248:					.HasMarker_Lambda([this]() { return bHasMarker; })
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:251:					.OnHeightmapClicked(TDelegate<void(FVector2D)>::CreateSP(this, &SLandscapeHeightmapTrackerPanel::OnHeightmapClicked))
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:424:void SLandscapeHeightmapTrackerPanel::OnViewportClick(const FLandscapeHeightmapTrackerModule::FViewportClickResult& Click)
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:452:		bHasMarker = true;
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:454:		MarkerUV = LastMapping.NormalizedUV;
```
