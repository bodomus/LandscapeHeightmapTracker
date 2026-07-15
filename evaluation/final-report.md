# Final Comparative Benchmark Report

Workspace: `J:\Projects\UE_Projects\UE57Editor\Plugins\LandscapeHeightmapTracker`

Date: 2026-07-09

Production code was not modified. Plugin bugs were not fixed. No commit was made.

## Benchmark Methodology

The benchmark used the required three-agent sequence.

Phase A ran two independent researchers in parallel:

- `graphify_researcher` used Graphify as the primary analysis method.
- `crg_researcher` used code-review-graph (CRG) as the primary analysis method.
- Both received the same benchmark questions and scoring criteria.
- Each was instructed not to read the other tool's output.

Phase B ran `benchmark_reviewer` only after both researchers completed. The reviewer read both result sets, independently inspected source code, validated major claims, identified false positives and false negatives, and compared tool usefulness.

Phase C inspected all generated benchmark artifacts and synthesized this report:

- `evaluation/graphify-results.md`
- `evaluation/graphify/raw-results.md`
- `evaluation/graphify/analysis.md`
- `evaluation/graphify/metrics.md`
- `evaluation/crg/crg-results.md`
- `evaluation/crg/raw-results.md`
- `evaluation/crg/analysis.md`
- `evaluation/crg/metrics.md`
- `evaluation/reviewer-validation.md`
- prior `evaluation/final-report.md`

Runtime claims were accepted only when source showed direct evidence: a call, delegate subscription or broadcast, Slate binding, widget construction, Unreal lifecycle registration, or data write/read. Graph proximity, graph paths, communities, impact neighborhoods, and hub scores were treated as candidate evidence only.

## Tool Versions

Graphify:

- Version: `graphify 0.9.8`
- Executable: `C:\Users\bodom\.local\bin\graphify.exe`
- Existing graph: `graphify-out/graph.json`
- Graph built at commit: `f17cf66490dc1af63aea15b651ba1fa069b9d69c`
- Current source `HEAD`: `93b214fbe2ea3f78286e2240d6b117eb6812c36d`
- Key caveat: Graphify graph was stale relative to current source.

CRG:

- Version: `code-review-graph 2.3.6`
- Executable reported by researcher: `C:\Python312\Scripts\code-review-graph.exe`
- Graph commit: `93b214fbe2ea`
- Last updated: `2026-07-09T20:17:25`
- Embeddings: `0` nodes embedded, so semantic search used keyword/hybrid fallback.

## Indexing Scope

Graphify reported:

- 30 files in the graph report
- About 7,315 words
- 376 nodes
- 538 edges
- 18 communities
- 31 distinct source/doc/resource paths observed in `graph.json`
- Directed: `False`
- Multigraph: `False`

CRG reported:

- 25 files
- 125 nodes
- 916 edges
- Languages: `csharp`, `cpp`, `c`
- Node kinds: 12 classes, 25 files, 88 functions
- Edge kinds: 715 `CALLS`, 105 `CONTAINS`, 95 `IMPORTS_FROM`, 1 `INHERITS`

## Verified Source Baseline

The reviewer independently verified these source facts:

- Plugin lifecycle and editor registration are in `FLandscapeHeightmapTrackerModule::StartupModule()` in `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp:125`.
- `OnSpawnPluginTab()` creates `SLandscapeHeightmapTrackerPanel` in `LandscapeHeightmapTrackerModule.cpp:169`.
- The main image/marker widget is private class `SHeightmapTrackerImageView` in `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp:40`.
- `SLandscapeHeightmapTrackerPanel::Construct()` creates that widget with `SNew(SHeightmapTrackerImageView)` at `SLandscapeHeightmapTrackerPanel.cpp:245`.
- Cursor acquisition is click-driven through `FLandscapeHeightmapTrackerEdMode::InputKey()` and `TraceLandscapeClick()`.
- `TraceLandscapeClick()` reads `ViewportClient->GetCursorWorldLocationFromMousePos()` at `LandscapeHeightmapTrackerEdMode.cpp:71`.
- The verified marker update path is left-click based, not hover or continuous mouse-move based.
- `FLandscapeCoordinateMapper::MapWorldPosition()` converts world space to local space with `InverseTransformPosition`, then delegates to `MapLocalPosition()`.
- `UVToPixel()` maps UV endpoints to inclusive pixel indices using `ImageSize - 1`.
- No production source usage was found for Landscape component size, vertex count, subsection count, section base, or resolution metadata beyond `GetBoundingRect()` and loaded image size.

## Graphify Results

Graphify was strongest as a broad orientation tool. It quickly surfaced the main code regions:

- `FLandscapeHeightmapTrackerModule`
- `SLandscapeHeightmapTrackerPanel`
- `SHeightmapTrackerImageView`
- `FLandscapeHeightmapTrackerEdMode`
- `FLandscapeCoordinateMapper`
- `FHeightmapImageClickMapper`
- `FLandscapeSurfaceTraceHelper`
- mapper and image-click tests

Best results:

- Q1, Q2, Q4, and Q9 were high-signal for files and symbols.
- Cross-file candidate discovery was useful.
- It highlighted the panel as the central integration point.
- It included docs/source context that helped architecture orientation.

Main failures:

- The graph was stale, so line numbers and relationships needed verification.
- `GetCursorWorldLocationFromMousePos` was absent as a graph node.
- Shortest paths were structural proximity, not verified runtime flow.
- Some relationships were directionally misleading, including reported call direction around `TraceAssignedLandscapeSurface` and `OnHeightmapClicked`.
- It did not reliably enumerate actual callers.
- It did not answer absence claims, such as lack of component-size or vertex-count usage, without source search.

Graphify average usefulness from its own metrics was 3.3/5. The reviewer also rated Graphify overall 3/5 because it required source verification for benchmark-grade conclusions.

## CRG Results

CRG was strongest for exact current-source symbol lookup and compact local graph facts.

Best results:

- Exact symbol search worked for `StartupModule`, `SLandscapeHeightmapTrackerPanel`, `TraceLandscapeClick`, `MapLocalPosition`, and mapper-related symbols.
- Local mapper flows were useful, especially `MapWorldPosition -> MapLocalPosition -> IsValid -> UVToPixel`.
- Graph freshness matched the current checkout.
- Output for exact queries was concise.

Main failures:

- No embeddings were available, and multi-term semantic searches often returned zero results.
- Rename preview was not reliable as caller discovery and missed qualified static C++ call sites.
- CRG did not model cross-file Unreal runtime flow through delegates, tab spawning, editor mode lifecycle, Slate lambdas, or Slate attributes.
- Impact outputs were noisy and internally inconsistent for mapper changes.
- Parser artifacts included header `class` pseudo-functions and some headers treated as `c`.

CRG average usefulness from its own metrics was 3/5. The reviewer also rated CRG overall 3/5, with exact lookup as its strongest mode and caller/runtime tracing as its weakest mode.

## Reviewer Validation

The reviewer confirmed that both researchers reached the main source facts after source verification. The most important validation point is that the benchmark phrase "when the cursor moves over the Landscape" does not match current implementation.

Verified forward marker path:

1. `SLandscapeHeightmapTrackerPanel::Construct()` subscribes to `FLandscapeHeightmapTrackerModule::OnViewportClickResult()` at `SLandscapeHeightmapTrackerPanel.cpp:176`.
2. The panel enables tracking state through module settings.
3. `FLandscapeHeightmapTrackerEdMode::InputKey()` handles left mouse press.
4. `TraceLandscapeClick()` reads the cursor ray and line-traces the world.
5. `TraceLandscapeClick()` broadcasts `FViewportClickResult`.
6. `SLandscapeHeightmapTrackerPanel::OnViewportClick()` receives the delegate callback.
7. `OnViewportClick()` validates the assigned Landscape and calls `FLandscapeCoordinateMapper::MapWorldPosition()` at `SLandscapeHeightmapTrackerPanel.cpp:449`.
8. On success, it sets `bHasMarker = true` and `MarkerUV = LastMapping.NormalizedUV`.
9. `SHeightmapTrackerImageView::OnPaint()` draws the marker from `MarkerUV`.

Verified reverse marker path:

1. `SHeightmapTrackerImageView::OnMouseButtonDown()` maps widget-local click to display UV.
2. `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked()` maps display UV to Landscape local position.
3. It transforms local position to world using `AssignedLandscape->GetActorTransform().TransformPosition(...)`.
4. `FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface()` resolves surface position.
5. `FLandscapeHeightmapTrackerModule::SetReverseMarker()` stores the reverse marker for editor viewport rendering.

## Disagreements And Corrections

| Topic | Researcher/tool behavior | Reviewer correction |
|---|---|---|
| Cursor movement | Both researchers flagged the wording. | Source has click-driven tracking only; no continuous hover/move marker update path was verified. |
| Graphify freshness | Graphify found many right files. | Its graph was stale, so line numbers and relationships could not be trusted without source reads. |
| Graphify path output | Paths connected marker/widget nodes to tracing nodes. | These were graph proximity paths, not executable control/data flow. |
| Graphify Q5 | Direct Q5 output emphasized `OnHeightmapClicked()` and `TraceAssignedLandscapeSurface`. | That is the reverse image-click path, not the forward viewport-click image marker path. |
| CRG caller discovery | Rename preview found definitions and some internal calls. | It missed production/test qualified static callers of `MapWorldPosition` and `MapUVToLocalPosition`. |
| CRG impact radius | Impact outputs included many files and high risk. | Source filtering showed a narrower direct semantic radius centered on mapper, panel handlers, display UV adjacency, surface trace adjacency, and tests. |
| Component/vertex dimensions | Tools surfaced dimension-adjacent files. | Source search found no production component-size, subsection, section-base, or vertex-count math. |
| Runtime evidence | Both tools surfaced useful graph neighborhoods. | Runtime flow required source proof through calls, delegates, Slate bindings, and state updates. |

## Benchmark Table

| Question | Verified answer summary | Graphify | CRG | Reviewer note |
|---|---|---:|---:|---|
| Q1 initialization/register editor functionality | `StartupModule()` registers style, commands, editor mode, menus, and tab spawner; `OnSpawnPluginTab()` creates the panel. | 4 | 4 | Both good after module source read. |
| Q2 main visualization widget/owner | `SHeightmapTrackerImageView`, created by panel `Construct()`; panel created by module tab. | 4 | 4 | Both found key files/symbols; ownership needed source verification. |
| Q3 cursor/landscape acquisition | `InputKey()` -> `TraceLandscapeClick()` -> `GetCursorWorldLocationFromMousePos()` -> trace -> broadcast. | 3 | 3 | Both found pieces; trace builder and external API needed source search. |
| Q4 world/local to image/pixel mapping | `MapWorldPosition()` -> `MapLocalPosition()` -> `UVToPixel()`; reverse via `MapUVToLocalPosition()`. | 4 | 4 | Both useful; formulas required direct source read. |
| Q5 visual marker update path | Click-driven delegate path from editor mode to panel marker state to widget paint. | 2 | 2 | Both struggled with cross-file delegate/Slate runtime flow. |
| Q6 actual callers | `MapWorldPosition()` callers are panel `OnViewportClick()` and two tests. | 3 | 1 | Exact `rg` was authoritative; CRG rename preview failed badly. |
| Q7 impact radius | Medium-high inside plugin: mapper, panel, display UV adjacency, surface trace adjacency, tests. | 3 | 3 | Both provided candidates but overincluded noise. |
| Q8 transform sensitivity | `InverseTransformPosition`, `TransformPosition`, actor transform, transform tests. | 3 | 2 | Source search needed to identify exact transform-sensitive code. |
| Q9 dimensional assumptions | Bounds width/height, positive image size, inclusive `ImageSize - 1`, fitted image rect; no component-size math. | 4 | 2 | Graphify better at candidate discovery; absence required source search. |
| Q10 marker widget to acquisition | Widget attributes bind to panel fields updated from viewport-click delegate result. | 3 | 3 | Both found pieces but not verified runtime chain. |

Per-question averages:

- Graphify: 3.3/5
- CRG: 2.8/5

Reviewer overall tool assessment:

- Graphify: 3/5
- CRG: 3/5

The difference is mostly scoring granularity. Graphify scored higher per question because it oriented better on broad architecture. The reviewer still recommended task-based routing rather than a single winner because both tools failed on caller completeness and Unreal runtime flow.

## Debugging Scenario Comparison

Scenario: marker appears in the wrong place after clicking the Landscape.

- Start with Graphify to locate the panel, editor mode, mapper, image widget, and helper files.
- Use CRG for exact symbol lookup once names are known.
- Verify runtime path in source: `InputKey`, `TraceLandscapeClick`, `OnViewportClickResult`, `OnViewportClick`, `MapWorldPosition`, `MarkerUV`, and `OnPaint`.
- Use direct source reads for transform and bounds math.

Scenario: changing coordinate-conversion semantics.

- Do not trust either tool's impact radius alone.
- Use Graphify/CRG only to collect candidate files.
- Use exact `rg` for `MapWorldPosition`, `MapLocalPosition`, `MapUVToLocalPosition`, and `UVToPixel`.
- Classify impact as direct, adjacent, test-only, or false positive.

Scenario: proving there is no hover tracking.

- Neither graph tool is enough for absence claims.
- Use exhaustive source search for `MouseMove`, `OnMouseMove`, `InputAxis`, `Tick`, `Hover`, `Cursor`, `GetCursorWorldLocationFromMousePos`, and viewport input APIs.
- Inspect editor mode and Slate handlers directly.

Scenario: tracing image-click-to-Landscape reverse marker.

- Graphify can find the image widget, panel, mapper, and surface trace helper quickly.
- CRG can identify local mapper flow.
- Source verification must connect `OnMouseButtonDown`, `MapLocalPositionToDisplayUV`, `OnHeightmapClicked`, `MapUVToLocalPosition`, `TransformPosition`, `TraceAssignedLandscapeSurface`, and `SetReverseMarker`.

## Limitations

- This benchmark covers one Unreal Engine plugin and ten targeted questions.
- Graphify's graph was stale relative to current `HEAD`.
- CRG had no embeddings, weakening broad semantic search.
- Both tools struggled with Unreal-specific constructs: editor mode lifecycle, module registration, static delegates, Slate `SNew`, Slate attributes, lambdas, macros, and class-qualified static calls.
- Both tools required direct source verification for runtime flow.
- Neither tool was reliable for absence claims.
- No production code was changed, so possible implementation issues found during inspection were not fixed.

## Controlled Rerun After Index Normalization

The controlled rerun preserved the original benchmark artifacts and reran only Q3, Q5, Q6, and Q10 after normalizing index conditions.

Artifacts:

- `evaluation/control-rerun/environment-before.md`
- `evaluation/control-rerun/graphify-refresh.md`
- `evaluation/control-rerun/crg-embedding-diagnosis.md`
- `evaluation/control-rerun/graphify/raw-results.md`
- `evaluation/control-rerun/graphify/analysis.md`
- `evaluation/control-rerun/graphify/metrics.md`
- `evaluation/control-rerun/crg/raw-results.md`
- `evaluation/control-rerun/crg/analysis.md`
- `evaluation/control-rerun/crg/metrics.md`
- `evaluation/control-rerun/reviewer-delta.md`

Old environment condition:

- Graphify was stale: the original graph report said it was built from `f17cf664`, while source `HEAD` was `93b214fbe2ea3f78286e2240d6b117eb6812c36d`.
- CRG already matched `HEAD`, but had `0` embedded nodes, so semantic search lacked expected vector coverage.

New environment condition:

- Graphify was refreshed with `graphify update .`; `GRAPH_REPORT.md` now reports `Built from commit: 93b214fb`, matching current `HEAD` by prefix.
- Refreshed Graphify stats: `503` nodes, `645` edges, `38` communities.
- CRG local embeddings were enabled by installing the supported `code-review-graph[embeddings]` extra, then running `code-review-graph embed --repo . --provider local`.
- CRG was rebuilt with `code-review-graph build --repo .`.
- Final CRG stats: `125` nodes, `916` edges, `25` files, `100` embeddings using `local:all-MiniLM-L6-v2`.
- CRG embedded Function/Class nodes; File nodes remained unembedded by observed CRG behavior.

Controlled rerun scores:

| Q | Topic | Graphify original | Graphify controlled | CRG original | CRG controlled |
|---|---|---:|---:|---:|---:|
| Q3 | Cursor acquisition | 3 | 3 | 3 | 3 |
| Q5 | Marker runtime path | 2 | 2 | 2 | 2 |
| Q6 | Actual callers | 3 | 3 | 1 | 1 |
| Q10 | Cross-subsystem connection | 3 | 2 | 3 | 3 |

Graphify delta:

- Freshness improved candidate coverage, especially around `FViewportTraceRayBuilder::BuildTraceSegment`.
- It did not materially improve Q5, Q6, or Q10.
- It still needed direct source verification for `GetCursorWorldLocationFromMousePos`, `LineTraceSingleByChannel`, delegate broadcasts, Slate attributes, marker state writes, and exact callers.
- It still confused structural proximity with runtime flow, especially shortest paths through shared types and reverse-path symbols.

CRG delta:

- Embeddings became active and semantic search returned more candidate symbols.
- Semantic rankings remained noisy and did not materially improve the controlled question scores.
- Caller discovery for `FLandscapeCoordinateMapper::MapWorldPosition` did not improve; CRG still missed the production call at `SLandscapeHeightmapTrackerPanel.cpp:449` and test calls at `LandscapeCoordinateMapperTests.cpp:59` and `:65` before fallback validation.
- CRG still did not model the cross-file Unreal delegate path or Slate attribute data flow.

Reviewer validation:

- The verified forward marker path remains click-driven: `InputKey` -> `TraceLandscapeClick` -> viewport cursor ray -> line trace -> `OnViewportClickResult().Broadcast` -> panel `OnViewportClick` -> `MapWorldPosition` -> `bHasMarker`/`MarkerUV` writes -> `SHeightmapTrackerImageView::OnPaint`.
- The reverse image-click path through `OnMouseButtonDown`, `OnHeightmapClicked`, `TraceAssignedLandscapeSurface`, and `SetReverseMarker` is separate and must not be treated as the forward image marker update.
- No hover or mouse-move marker update path was verified.

Final conclusion after controlled rerun:

- The final recommendation did not change.
- Graphify remains useful for broad architecture and candidate discovery.
- CRG remains useful for exact/local symbol lookup when names are known.
- Direct `rg` plus source inspection remains authoritative for exact callers, runtime flow, Unreal delegates, Slate attributes, impact classification, and absence claims.

## Final Recommendation

Use task-based routing between both tools, with direct source verification as the authority.

Use Graphify first for:

- unfamiliar subsystem orientation;
- broad architecture discovery;
- cross-file candidate discovery;
- finding integration hubs and likely related tests/docs.

Use CRG first for:

- exact symbol lookup when the name is known;
- compact current-source file and symbol inventory;
- local same-file flow hints;
- candidate impact lists, with source filtering.

Use direct `rg` and line-numbered source reads for:

- exact callers and callees;
- verified runtime execution flow;
- Unreal delegate subscription/broadcast paths;
- Slate ownership, attributes, and event wiring;
- Unreal lifecycle behavior;
- impact radius classification;
- absence claims such as "no hover tracking" or "no component-size usage".

Do not present Graphify paths, Graphify proximity, CRG flows, CRG communities, or CRG impact radius as verified runtime behavior unless source code proves the relationship.

## Proposed AGENTS.md Routing Policy

```md
## Code Intelligence Routing

- Use Graphify first for architecture discovery, cross-file exploration, and unfamiliar subsystem orientation.
- Use code-review-graph first for exact symbol lookup, compact symbol inventory, local same-file flow hints, and candidate impact lists.
- For exact callers and callees, use direct `rg` plus source inspection as the authority; graph tools are candidate generators only.
- For Unreal Slate, editor mode, module lifecycle, macros, static delegates, and event flows, never treat graph proximity as runtime execution flow without source-level evidence.
- For impact analysis, separate direct semantic impact from graph proximity. Verify direct callers, tests, UI reads/writes, delegate paths, and user-visible state changes in source.
- For absence claims, require targeted exhaustive searches over `Source/` and relevant tests.
- If a graph index is stale relative to current `HEAD`, do not trust graph line numbers or relationships until verified in source.
- When Graphify and CRG disagree, prefer the claim supported by direct source evidence and record the disagreement in the task notes.
```
