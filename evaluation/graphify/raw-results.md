# Graphify Raw Results

Scope: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

Isolation: used Graphify and direct source verification only. Did not use code-review-graph and did not read CRG or other benchmark reviewer artifacts.

## Tool and Graph Metadata

Command:

```powershell
Get-Command graphify -ErrorAction SilentlyContinue | Select-Object Source,Version | Format-List; graphify --version
```

Output:

```text
Source  : C:\Users\bodom\.local\bin\graphify.exe
Version : 0.0.0.0

graphify 0.9.8
```

Command:

```powershell
Test-Path -LiteralPath 'graphify-out\graph.json'; if (Test-Path -LiteralPath 'graphify-out\graph.json') { Get-Item -LiteralPath 'graphify-out\graph.json' | Select-Object FullName,Length,LastWriteTime | Format-List }
```

Output:

```text
True

FullName      : J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker\graphify-out\graph.json
Length        : 424496
LastWriteTime : 7/9/2026 2:21:09 PM
```

Command:

```powershell
$g = Get-Content -LiteralPath 'graphify-out\graph.json' -Raw | ConvertFrom-Json; "built_at_commit=$($g.built_at_commit)"; "node_count=$($g.nodes.Count)"; "link_count=$($g.links.Count)"; "directed=$($g.directed)"; "multigraph=$($g.multigraph)"
```

Output:

```text
built_at_commit=f17cf66490dc1af63aea15b651ba1fa069b9d69c
node_count=376
link_count=538
directed=False
multigraph=False
```

Command:

```powershell
Get-Content -LiteralPath 'graphify-out\.graphify_root' -ErrorAction SilentlyContinue; Get-Content -LiteralPath 'graphify-out\.graphify_python' -ErrorAction SilentlyContinue; git rev-parse HEAD
```

Output:

```text
.
93b214fbe2ea3f78286e2240d6b117eb6812c36d
```

Note: Graphify graph is stale relative to current `HEAD` (`f17cf664...` vs `93b214f...`). Findings below are therefore verified against current source.

Graphify report excerpt:

```text
# Graph Report - LandscapeHeightmapTracker  (2026-07-09)

## Corpus Check
- 30 files В· ~7,315 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 376 nodes В· 538 edges В· 18 communities (13 shown, 5 thin omitted)
- Extraction: 94% EXTRACTED В· 6% INFERRED В· 0% AMBIGUOUS В· INFERRED: 31 edges (avg confidence: 0.8)
- Token cost: 0 input В· 0 output

## Graph Freshness
- Built from commit: `f17cf664`
```

Source list command:

```powershell
$g = Get-Content -LiteralPath 'graphify-out\graph.json' -Raw | ConvertFrom-Json; $sources = $g.nodes | ForEach-Object { $_.source_file } | Where-Object { $_ } | Sort-Object -Unique; "sources=$($sources.Count)"; $sources
```

Relevant output:

```text
sources=31
Docs/ARCHITECTURE.md
Docs/TEST_PLAN.md
README.md
Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.cpp
Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h
Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp
Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.cpp
Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.h
Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
Source/LandscapeHeightmapTracker/Private/Tests/HeightmapImageClickMapperTests.cpp
Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp
Source/LandscapeHeightmapTracker/Private/Tests/LandscapeSurfaceTraceHelperTests.cpp
Source/LandscapeHeightmapTracker/Private/Tests/ViewportTraceRayBuilderTests.cpp
Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.cpp
Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.h
Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h
Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h
Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h
```

## Graphify Queries

Command:

```powershell
graphify query "Q1 Where does the LandscapeHeightmapTracker plugin initialize and register editor-facing functionality? Include files and symbols." --budget 1600
```

Relevant output:

```text
Traversal: BFS depth=2 | Start: ['Initialize', 'LandscapeHeightmapTracker', 'LandscapeHeightmapTracker', 'EditorModeId', 'PluginTabName', 'RegisterMenus'] | 43 nodes found

NODE FLandscapeHeightmapTrackerModule [src=Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h loc=L9 community=7]
NODE LandscapeHeightmapTrackerModule.cpp [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L1 community=3]
NODE FLandscapeHeightmapTrackerModule::StartupModule() [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L110 community=3]
NODE EditorModeId [src=Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h loc=L22 community=7]
NODE PluginTabName [src=Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h loc=L21 community=7]
NODE RegisterMenus [src=Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h loc=L35 community=7]
EDGE Initialize --calls [EXTRACTED context=call]--> FLandscapeHeightmapTrackerModule::StartupModule()
```

Command:

```powershell
graphify query "Q2 Which class implements the main heightmap visualization widget, and what code creates or owns it? Include exact files and symbols." --budget 1600
```

Relevant output:

```text
Traversal: BFS depth=2 | Start: ['HeightmapBrush', 'HeightmapTexture', 'HeightmapImageClickMapper.h', 'SCompoundWidget'] | 78 nodes found

NODE SLandscapeHeightmapTrackerPanel [src=Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h loc=L11 community=0]
NODE SHeightmapTrackerImageView [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L40 community=6]
NODE SLandscapeHeightmapTrackerPanel::Construct() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L164 community=6]
NODE .OnPaint() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L67 community=6]
NODE .OnMouseButtonDown() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L111 community=12]
NODE LandscapeHeightmapTrackerModule.cpp [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp loc=L1 community=3]
```

Command:

```powershell
graphify query "Q3 Which code reads or derives the current cursor position over the Unreal Landscape? Include files and symbols." --budget 1600
```

Relevant output:

```text
Traversal: BFS depth=2 | Start: ['LandscapeUV', 'LandscapeHeightmapTracker', 'LandscapeTrackerSettings.h', 'LandscapeHeightmapTrackerModule.cpp', 'FLandscapeCoordinateMapper::MapLocalPosition()'] | 122 nodes found

NODE InputKey [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L9 community=7]
NODE TraceLandscapeClick [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L13 community=7]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=2]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
```

Command:

```powershell
graphify query "Q4 How does world-space or local-space Landscape position become heightmap image or pixel coordinates? Focus on coordinate conversion functions and formulas." --budget 1800
```

Relevant output:

```text
NODE FLandscapeCoordinateMapper::MapLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L25 community=1]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE FLandscapeCoordinateMapper::MapUVToLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L84 community=1]
NODE MapLocalPositionToDisplayUV [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L26 community=12]
NODE CalculateFittedImageRect [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L22 community=12]
```

Command:

```powershell
graphify query "Q5 Trace the verified code path responsible for updating the visual marker when the cursor moves over the Landscape. Include functions and call relationships." --budget 1800
```

Relevant output:

```text
Traversal: BFS depth=2 | Start: ['MarkerUV', 'MarkerUV', 'TraceAssignedLandscapeSurface', 'LandscapeHeightmapTrackerModule.cpp', 'SLandscapeHeightmapTrackerPanel::GetImagePathText()'] | Context: call (heuristic) | 16 nodes found

NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
NODE TraceAssignedLandscapeSurface [src=Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.h loc=L40 community=5]
NODE MarkerUV [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L143 community=6]
EDGE TraceAssignedLandscapeSurface --calls [EXTRACTED context=call]--> SLandscapeHeightmapTrackerPanel::OnHeightmapClicked()
EDGE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() --calls [EXTRACTED context=call]--> MapUVToLocalPosition
EDGE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() --calls [EXTRACTED context=call]--> UVToPixel
```

Finding: this is misleading for Q5. It describes heightmap-click reverse mapping, not cursor/click acquisition from the landscape viewport.

Command:

```powershell
graphify query "Q6 Find all callers of FLandscapeCoordinateMapper::MapWorldPosition or the main coordinate conversion function. Include source locations." --budget 1800
```

Relevant output:

```text
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=2]
NODE FLandscapeCoordinateMapperOptionsTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L35 community=1]
NODE FLandscapeCoordinateMapperBasicTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L8 community=1]
NODE FLandscapeCoordinateMapperReverseMappingTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L82 community=1]
```

Finding: Graphify surfaced likely relevant tests but did not itself enumerate actual callers precisely; `rg` was required.

Command:

```powershell
graphify query "Q7 Estimate the impact radius of changing coordinate-conversion semantics in FLandscapeCoordinateMapper. Include dependent files, tests, UI code, and reverse mapping." --budget 2000
```

Relevant output:

```text
NODE FLandscapeTrackerMappingResult [src=Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h loc=L21 community=1]
NODE FLandscapeTrackerReverseMappingResult [src=Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h loc=L32 community=1]
NODE FLandscapeCoordinateMapper::MapLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L25 community=1]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=2]
NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
NODE FLandscapeCoordinateMapperOptionsTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L35 community=1]
NODE FLandscapeCoordinateMapperBasicTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L8 community=1]
NODE FLandscapeCoordinateMapperReverseMappingTest::RunTest() [src=Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp loc=L82 community=1]
NODE ViewportTraceRayBuilderTests.cpp [src=Source/LandscapeHeightmapTracker/Private/Tests/ViewportTraceRayBuilderTests.cpp loc=L1 community=9]
```

Finding: `ViewportTraceRayBuilderTests.cpp` is a false positive for coordinate conversion semantics.

Command:

```powershell
graphify query "Q8 Find code sensitive to Landscape translation, rotation, or non-unit scale. Include files and symbols." --budget 2000
```

Relevant output:

```text
NODE SLandscapeHeightmapTrackerPanel::GetActorRotationText() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L680 community=2]
NODE SLandscapeHeightmapTrackerPanel::GetActorScaleText() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L681 community=2]
NODE FLandscapeCoordinateMapper::MapUVToLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L84 community=1]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=2]
NODE SLandscapeHeightmapTrackerPanel::OnHeightmapClicked() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L465 community=1]
```

Finding: Graphify found UI display getters and relevant mapping functions but did not directly identify `InverseTransformPosition` / `TransformPosition`; source search was required.

Command:

```powershell
graphify query "Q9 Find dimensional assumptions involving width, height, resolution, vertex count, image size, pixel coordinates, component size, bounding rect. Include files and symbols." --budget 2200
```

Relevant output:

```text
NODE .OnPaint() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L67 community=6]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE FLandscapeCoordinateMapper::MapLocalPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L25 community=1]
NODE MapLocalPositionToDisplayUV [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L26 community=12]
NODE FHeightmapFittedImageRect [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L5 community=12]
NODE CalculateFittedImageRect [src=Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.h loc=L22 community=12]
NODE SLandscapeHeightmapTrackerPanel::LoadPngTexture() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L594 community=6]
NODE SLandscapeHeightmapTrackerPanel::RefreshLandscapeBounds() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L571 community=1]
```

Command:

```powershell
graphify path "MarkerUV" "GetCursorWorldLocationFromMousePos"
```

Output:

```text
No node matching 'GetCursorWorldLocationFromMousePos' found.
```

Command:

```powershell
graphify path "MarkerUV" "TraceLandscapeClick"
```

Output:

```text
Shortest path (5 hops):
  MarkerUV <--defines [EXTRACTED]-- SHeightmapTrackerImageView --defines [EXTRACTED]--> OnUnavailableClicked <--calls [EXTRACTED]-- SLandscapeHeightmapTrackerPanel::Construct() --references [EXTRACTED]--> FLandscapeHeightmapTrackerModule <--references [EXTRACTED]-- TraceLandscapeClick
warning: source match was ambiguous (top score 53228.2, runner-up 53228.2)
```

Finding: this shortest path is structural proximity only and is not verified runtime execution flow.

Command:

```powershell
graphify query "Q10 Starting from the marker widget, identify the connection to Landscape coordinate acquisition. Distinguish MarkerUV display, panel OnViewportClick, and editor mode TraceLandscapeClick." --budget 2200
```

Relevant output:

```text
NODE SHeightmapTrackerImageView [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L40 community=6]
NODE .OnPaint() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L67 community=6]
NODE SLandscapeHeightmapTrackerPanel::Construct() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L164 community=6]
NODE InputKey [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L9 community=7]
NODE TraceLandscapeClick [src=Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.h loc=L13 community=7]
NODE FLandscapeCoordinateMapper::MapWorldPosition() [src=Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp loc=L8 community=1]
NODE SLandscapeHeightmapTrackerPanel::OnViewportClick() [src=Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp loc=L424 community=2]
```

## Source Verification Commands

Command:

```powershell
rg -n "SNew\(SLandscapeHeightmapTrackerPanel\)|RegisterNomadTabSpawner|RegisterMenus|RegisterMode|UnregisterMode|PluginButtonClicked|StartupModule|ShutdownModule|OnSpawnPluginTab|MapAction|OpenPluginWindow" Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp
```

Relevant output:

```text
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:125:void FLandscapeHeightmapTrackerModule::StartupModule()
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:133:	PluginCommands->MapAction(
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:138:	FEditorModeRegistry::Get().RegisterMode<FLandscapeHeightmapTrackerEdMode>(
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:144:	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLandscapeHeightmapTrackerModule::RegisterMenus));
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:146:	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PluginTabName, FOnSpawnTab::CreateRaw(this, &FLandscapeHeightmapTrackerModule::OnSpawnPluginTab))
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:169:TSharedRef<SDockTab> FLandscapeHeightmapTrackerModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:175:			SNew(SLandscapeHeightmapTrackerPanel)
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:184:void FLandscapeHeightmapTrackerModule::RegisterMenus()
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp:190:	Section.AddMenuEntryWithCommandList(FLandscapeHeightmapTrackerCommands::Get().OpenPluginWindow, PluginCommands);
```

Command:

```powershell
rg -n "OnViewportClick|MarkerUV|SetMarker|ClearMarker|TraceLandscapeClick|InputKey|Render|SetTracking|SetReverseMarker|GetReverseMarker|Viewport|Mouse|Cursor|Hit" Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp Source\LandscapeHeightmapTracker\Public\SLandscapeHeightmapTrackerPanel.h Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.h Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerModule.cpp Source\LandscapeHeightmapTracker\Public\LandscapeHeightmapTrackerModule.h
```

Relevant output:

```text
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:14:bool FLandscapeHeightmapTrackerEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:20:			TraceLandscapeClick(ViewportClient, Viewport);
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:71:	const FViewportCursorLocation Cursor = ViewportClient->GetCursorWorldLocationFromMousePos();
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:110:	if (!World->LineTraceSingleByChannel(Hit, TraceSegment.Start, TraceSegment.End, ECC_Visibility, QueryParams))
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:123:	FLandscapeHeightmapTrackerModule::FViewportClickResult Result;
Source\LandscapeHeightmapTracker\Private\LandscapeHeightmapTrackerEdMode.cpp:127:	FLandscapeHeightmapTrackerModule::OnViewportClickResult().Broadcast(Result);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:176:	ClickDelegateHandle = FLandscapeHeightmapTrackerModule::OnViewportClickResult().AddSP(this, &SLandscapeHeightmapTrackerPanel::OnViewportClick);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:247:					.MarkerUV_Lambda([this]() { return MarkerUV; })
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:424:void SLandscapeHeightmapTrackerPanel::OnViewportClick(const FLandscapeHeightmapTrackerModule::FViewportClickResult& Click)
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:454:		MarkerUV = LastMapping.NormalizedUV;
```

Command:

```powershell
rg -n "MapWorldPosition\(|MapLocalPosition\(|MapUVToLocalPosition\(|UVToPixel\(" Source\LandscapeHeightmapTracker -g "*.h" -g "*.cpp"
```

Relevant output:

```text
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:8:FLandscapeTrackerMappingResult FLandscapeCoordinateMapper::MapWorldPosition(
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:15:	FLandscapeTrackerMappingResult Result = MapLocalPosition(
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:25:FLandscapeTrackerMappingResult FLandscapeCoordinateMapper::MapLocalPosition(
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:79:	Result.Pixel = UVToPixel(Result.NormalizedUV, ImageSize);
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:84:FLandscapeTrackerReverseMappingResult FLandscapeCoordinateMapper::MapUVToLocalPosition(
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:134:FIntPoint FLandscapeCoordinateMapper::UVToPixel(const FVector2D& UV, const FIntPoint& ImageSize)
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:59:	const FLandscapeTrackerMappingResult Transformed = FLandscapeCoordinateMapper::MapWorldPosition(Translated, Bounds, World, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:65:	const FLandscapeTrackerMappingResult RotatedResult = FLandscapeCoordinateMapper::MapWorldPosition(Rotated, Bounds, RotatedWorld, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:485:	const FLandscapeTrackerReverseMappingResult ReverseMapping = FLandscapeCoordinateMapper::MapUVToLocalPosition(LocalBounds, DisplayUV, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:508:	LastMapping.Pixel = FLandscapeCoordinateMapper::UVToPixel(ReverseMapping.DisplayUV, ImageSize);
```

Command:

```powershell
rg -n "GetActorTransform|InverseTransformPosition|TransformPosition|GetActorLocation|GetActorRotation|GetActorScale3D|GetBoundingRect|ImageSize|GetWidth|GetHeight|CreateTransient|ImageAspect|WidgetAspect|DrawSize|DrawOffset|ComputeDesiredSize|512|320|RoundToInt|ImageSize\.X|ImageSize\.Y|SectionBase|ComponentSize|NumSubsections|Subsection" Source\LandscapeHeightmapTracker
```

Relevant output:

```text
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:17:		LandscapeTransform.InverseTransformPosition(WorldPosition),
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:40:	if (ImageSize.X <= 0 || ImageSize.Y <= 0)
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:137:		FMath::Clamp(FMath::RoundToInt(UV.X * static_cast<double>(ImageSize.X - 1)), 0, ImageSize.X - 1),
Source\LandscapeHeightmapTracker\Private\LandscapeCoordinateMapper.cpp:138:		FMath::Clamp(FMath::RoundToInt(UV.Y * static_cast<double>(ImageSize.Y - 1)), 0, ImageSize.Y - 1));
Source\LandscapeHeightmapTracker\Private\HeightmapImageClickMapper.cpp:14:	const double ImageAspect = ImageSize.X / ImageSize.Y;
Source\LandscapeHeightmapTracker\Private\HeightmapImageClickMapper.cpp:15:	const double WidgetAspect = WidgetSize.X / WidgetSize.Y;
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:62:	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:64:		return FVector2D(512.0f, 512.0f);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:240:			+ SVerticalBox::Slot().FillHeight(1.0f).MinHeight(320.0f).Padding(8.0f)
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:449:	LastMapping = FLandscapeCoordinateMapper::MapWorldPosition(AssignedLandscape->GetActorTransform(), LocalBounds, Click.WorldPosition, ImageSize, Options);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:492:	const FTransform& LandscapeTransform = AssignedLandscape->GetActorTransform();
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:493:	const FVector WorldXY = LandscapeTransform.TransformPosition(ReverseMapping.LocalPosition);
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:579:	const FIntRect Rect = AssignedLandscape->GetBoundingRect();
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:627:	ImageSize = FIntPoint(ImageWrapper->GetWidth(), ImageWrapper->GetHeight());
Source\LandscapeHeightmapTracker\Private\SLandscapeHeightmapTrackerPanel.cpp:628:	HeightmapTexture = UTexture2D::CreateTransient(ImageSize.X, ImageSize.Y, PF_B8G8R8A8);
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:57:	const FTransform Translated(FRotator::ZeroRotator, FVector(10000.0, -3000.0, 0.0), FVector(2.0, 2.0, 1.0));
Source\LandscapeHeightmapTracker\Private\Tests\LandscapeCoordinateMapperTests.cpp:63:	const FTransform Rotated(FRotator(0.0, 90.0, 0.0), FVector::ZeroVector, FVector::OneVector);
```
