# Controlled Rerun Reviewer Delta

Scope: original `evaluation/` reports, controlled rerun outputs under `evaluation/control-rerun/graphify/` and `evaluation/control-rerun/crg/`, normalization reports, and direct source inspection under `Source/`.

I did not rerun the experiment. I used the researcher artifacts as inputs and verified important runtime/caller claims against source with targeted reads/searches. Production plugin code was not modified.

## Source Baseline

Verified forward viewport-click marker path:

1. `FLandscapeHeightmapTrackerEdMode::InputKey` handles only left mouse button press and calls `TraceLandscapeClick`: `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp:14-20`.
2. `TraceLandscapeClick` reads the viewport cursor ray with `GetCursorWorldLocationFromMousePos`, builds a trace segment, line traces, fills `FViewportClickResult`, and broadcasts: `LandscapeHeightmapTrackerEdMode.cpp:58-129`, specifically `:71-75`, `:110`, `:123-127`.
3. `FViewportTraceRayBuilder::BuildTraceSegment` derives `Start`/`End`: `Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.cpp:3-37`.
4. The module owns the static multicast delegate: `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h:12-19`, `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp:20`, `:60-62`.
5. The panel subscribes to the delegate with `AddSP`: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:176`.
6. `SLandscapeHeightmapTrackerPanel::OnViewportClick` calls `FLandscapeCoordinateMapper::MapWorldPosition` and writes marker state: `SLandscapeHeightmapTrackerPanel.cpp:424-463`, specifically `:449`, `:452-454`.
7. `SHeightmapTrackerImageView::OnPaint` reads `HasMarker` and `MarkerUV`; the panel binds those attributes with lambdas: `SLandscapeHeightmapTrackerPanel.cpp:67-105`, `:245-248`.

Verified reverse image-click path, separate from the forward viewport-click marker update:

- `SHeightmapTrackerImageView::OnMouseButtonDown` maps image-local clicks and executes `OnHeightmapClicked`: `SLandscapeHeightmapTrackerPanel.cpp:111-131`.
- `OnHeightmapClicked` calls `MapUVToLocalPosition`, transforms local to world XY, calls `TraceAssignedLandscapeSurface`, and then `SetReverseMarker`: `SLandscapeHeightmapTrackerPanel.cpp:465-512`.
- `SetReverseMarker` stores module reverse-marker state; editor mode `Render` reads it with `GetReverseMarker`: `LandscapeHeightmapTrackerModule.cpp:76-83`, `LandscapeHeightmapTrackerEdMode.cpp:38-55`.

Verified callers of `FLandscapeCoordinateMapper::MapWorldPosition`:

- Declaration, not caller: `Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h:44`.
- Definition, not caller: `Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp:8`.
- Production call: `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:449`.
- Test calls: `Source/LandscapeHeightmapTracker/Private/Tests/LandscapeCoordinateMapperTests.cpp:59`, `:65`.

No hover/mouse-move marker update path was verified. Source search for `OnMouseMove`, `MouseMove`, `IE_Repeat`, `IE_Axis`, `Hover`, and cursor-move handlers found only `Tick`, which handles reverse-marker cleanup, not forward marker updates.

## Score Delta

Scores are researcher metric scores where provided.

| Q | Topic | Graphify original | Graphify controlled | Delta | CRG original | CRG controlled | Delta |
|---|---|---:|---:|---:|---:|---:|---:|
| Q3 | Cursor acquisition | 3 | 3 | 0 | 3 | 3 | 0 |
| Q5 | Marker runtime path | 2 | 2 | 0 | 2 | 2 | 0 |
| Q6 | Actual callers | 3 | 3 | 0 | 1 | 1 | 0 |
| Q10 | Cross-subsystem connection | 3 | 2 | -1 | 3 | 3 | 0 |

Interpretation: normalization changed freshness/availability conditions, but it did not produce a material score improvement on the controlled questions. Graphify Q10 was scored lower in the rerun because shortest paths remained unsafe as runtime-flow evidence.

## Claim Validation

| Claim | Classification | Reviewer finding |
|---|---|---|
| Graphify was stale in the original run and fresh after `graphify update .` | verified | Original metrics report Graphify built at `f17cf664...` while current `HEAD` was `93b214f...`; refresh report says `GRAPH_REPORT.md` now reports `93b214fb`, matching current `HEAD` prefix. |
| Graphify freshness materially improved Q3 | partially verified | Fresh Graphify surfaced `BuildTraceSegment`, `FViewportTraceSegment`, `TraceLandscapeClick`, `InputKey`, `OnViewportClick`, and `MapWorldPosition`. This fixed part of the original Q3 discovery gap, especially trace-builder visibility, but score stayed 3/5 and source reads were still required for `GetCursorWorldLocationFromMousePos`, `LineTraceSingleByChannel`, and broadcast/result fields. |
| Graphify freshness materially improved Q5 | false | Controlled Graphify still mixed forward viewport-click marker update with reverse image-click symbols: `OnMouseButtonDown`, `OnHeightmapClicked`, `TraceAssignedLandscapeSurface`, and `SetReverseMarker`. Source shows those are reverse-path symbols, not the forward image marker update after a Landscape click. Score stayed 2/5. |
| Graphify freshness materially improved Q6 | false | Controlled Graphify found candidate nodes including `OnViewportClick` and `FLandscapeCoordinateMapperOptionsTest::RunTest`, but `graphify explain "FLandscapeCoordinateMapper::MapWorldPosition"` showed outgoing references/callee `MapLocalPosition`, not incoming caller edges. Exact caller list still required `rg`. Score stayed 3/5. |
| Graphify freshness materially improved Q10 | false | Controlled Graphify found relevant candidates but produced shortest paths through shared types, `OnHeightmapClicked`, and module references. Those paths are structural proximity, not runtime flow. Score dropped from 3/5 to 2/5. |
| CRG embedding availability materially improved semantic discovery | partially verified | Embeddings changed CRG from zero embedded nodes to 100 embedded Function/Class nodes and enabled cosine-ranked semantic search. It did surface relevant candidates such as `TraceLandscapeClick`, `OnPaint`, `SHeightmapTrackerImageView`, and `OnHeightmapClicked`. However rankings remained noisy (`OnMouseButtonDown`, settings/build/commands, reverse-path helpers), and no controlled question score improved. |
| CRG embeddings solved semantic discovery | false | Q6 semantic search ranked mapper definitions and related mapper functions, not actual callers; Q10 semantic search ranked reverse-path `TraceAssignedLandscapeSurface` above `TraceLandscapeClick` and `OnPaint`. Semantic discovery improved operationally, not materially enough for benchmark-grade answers. |
| Either tool improved runtime-flow reconstruction | false | Neither tool reconstructed the complete Unreal/Slate runtime path: `InputKey -> TraceLandscapeClick -> Broadcast -> AddSP/OnViewportClick -> MapWorldPosition -> MarkerUV/bHasMarker writes -> OnPaint reads`. Both still required direct source verification for delegate wiring, Slate attributes, and state flow. |
| CRG caller discovery improved | false | Controlled CRG could show `OnViewportClick -> MapWorldPosition` when querying from `OnViewportClick`, but caller lookup from `MapWorldPosition` returned no rows. Q6 metrics report 0/3 caller recall before fallback: missed `SLandscapeHeightmapTrackerPanel.cpp:449` and test calls at `LandscapeCoordinateMapperTests.cpp:59,65`. |
| Graphify still confuses structural proximity with runtime flow | verified | Controlled Q10 shortest paths used `FVector2D`, `OnHeightmapClicked`, `FLandscapeTrackerMappingResult`, and module references to bridge the marker widget to coordinate acquisition. Source shows the forward bridge is the module delegate plus panel state, not those shortest paths. |
| Previous routing recommendation still holds | verified | The controlled rerun does not support a single winner. Graphify remains better for broad candidate discovery; CRG remains better for compact exact/local symbol lookup. Both are subordinate to source verification for callers, Unreal runtime flow, impact classification, and absence claims. |

## Per-Question Delta

### Q3 Cursor Acquisition

Graphify freshness produced a real but limited discovery improvement: the controlled output included `BuildTraceSegment` and `FViewportTraceSegment`, which the original Graphify report said was missed or underweighted. This is a freshness/candidate-coverage improvement, not a material answer-quality improvement. The verified source-critical calls remain `GetCursorWorldLocationFromMousePos` at `LandscapeHeightmapTrackerEdMode.cpp:71`, `BuildTraceSegment` at `:72-75`, `LineTraceSingleByChannel` at `:110`, and broadcast at `:127`; Graphify still did not make those lines sufficient without fallback.

CRG with embeddings found `TraceLandscapeClick` semantically, but also ranked `OnMouseButtonDown` for cursor acquisition. CRG's exact/local graph evidence for `InputKey -> TraceLandscapeClick` was useful; runtime details still needed source reads.

### Q5 Marker Runtime Path

No material improvement. The verified forward path is click-driven and goes through the module delegate and panel state writes: `InputKey` at `LandscapeHeightmapTrackerEdMode.cpp:14-20`, broadcast at `:127`, panel subscription at `SLandscapeHeightmapTrackerPanel.cpp:176`, marker writes at `:452-454`, and paint reads at `:87-105`.

Both controlled tools still mixed this with the reverse image-click path. `OnMouseButtonDown`, `OnHeightmapClicked`, `TraceAssignedLandscapeSurface`, and `SetReverseMarker` are real symbols, but source places them in the reverse path at `SLandscapeHeightmapTrackerPanel.cpp:111-131`, `:465-512`, not the forward marker update.

### Q6 Actual Callers

No material improvement. Source truth is three call sites: production at `SLandscapeHeightmapTrackerPanel.cpp:449`, tests at `LandscapeCoordinateMapperTests.cpp:59` and `:65`.

Graphify found plausible caller candidates but not incoming call edges. CRG's exact caller query for `MapWorldPosition` returned no rows, despite CRG being able to show a callee edge when starting from `OnViewportClick`. This is not usable caller discovery for the benchmark question.

### Q10 Cross-Subsystem Connection

No material improvement. The actual connection is:

`SHeightmapTrackerImageView::OnPaint` reads Slate attributes (`SLandscapeHeightmapTrackerPanel.cpp:67-105`) <- panel binds attributes (`:245-248`) <- panel writes `MarkerUV`/`bHasMarker` in `OnViewportClick` (`:449-454`) <- panel subscribed to module delegate (`:176`) <- editor mode broadcasts after trace (`LandscapeHeightmapTrackerEdMode.cpp:123-127`).

Graphify's controlled shortest paths are misleading because they pass through shared types and reverse-path symbols. CRG's semantic search found some right candidates but did not model delegate-to-callback or Slate attribute data flow.

## Dimension Delta

- Architecture discovery: Graphify improved freshness and corpus size, but controlled scores did not improve on these four questions. CRG embeddings added semantic candidates but did not materially change architecture conclusions.
- Exact symbol lookup: CRG remains stronger for known symbols; Graphify remains useful for broad symbol neighborhoods.
- Caller/callee correctness: unchanged. Use direct `rg` and source reads for caller lists.
- Dependency/impact analysis: unchanged. Both tools still generate candidate neighborhoods, not verified impact classifications.
- Cross-file exploration: Graphify remains broader; CRG remains local/tighter. Neither reconstructs Unreal delegate or Slate attribute runtime flow.
- Debugging usefulness: unchanged. Both help find files; source remains authoritative.
- Context efficiency: CRG exact lookups are compact; CRG semantic/impact and Graphify BFS/path outputs still require filtering.
- Unreal C++ parsing quality: unchanged in the critical areas: delegates, Slate lambdas/attributes, external Unreal APIs, and qualified static callers remain weak.
- False positives: still present, especially reverse-path symbols for forward marker questions.
- False negatives: still present, especially `MapWorldPosition` callers and delegate/attribute relationships.
- Misleading graph relationships: still present; shortest paths/flows must not be treated as execution proof.

## Recommendation Delta

The previous recommendation still holds: **task-based routing between both tools**.

Use Graphify first for broad architecture discovery and candidate file/symbol discovery, especially in unfamiliar areas. Use CRG first for exact current-source symbol lookup and local same-file flow hints. For exact callers, runtime flow, Unreal delegates, Slate attributes/lambdas, impact radius, and absence claims, use direct source verification as the authority.

No controlled-rerun evidence supports switching to Graphify-only, CRG-only, or a primary/fallback winner.
