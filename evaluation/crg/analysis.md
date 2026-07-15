# CRG Analysis

## What CRG Did Well

- Exact symbol lookup worked for named functions/classes:
  - `StartupModule`
  - `SLandscapeHeightmapTrackerPanel`
  - `TraceLandscapeClick`
  - `LandscapeCoordinateMapper`
  - `MapLocalPosition`
- Hub and bridge analysis correctly pointed to high-value orientation targets:
  - `SLandscapeHeightmapTrackerPanel::Construct`
  - `TraceLandscapeClick`
  - `OnViewportClick`
  - `OnHeightmapClicked`
  - `RefreshLandscapeBounds`
  - `StartupModule`
- Flow detection was useful for local, same-file mapper relationships:
  - `MapWorldPosition -> MapLocalPosition -> IsValid -> UVToPixel`
  - `MapUVToLocalPosition -> IsValid`
- Community membership was useful as an index of relevant private code, but too broad to answer runtime questions directly.

## What CRG Missed Or Misrepresented

- No direct caller/callee MCP tool was available. The tool hints suggested `query_graph`, but it was not exposed.
- Rename preview failed as a caller finder for qualified static C++ calls:
  - `MapWorldPosition` preview found only the definition and missed production/test callers.
  - `MapUVToLocalPosition` preview found only the definition and missed panel/tests.
  - `MapLocalPosition` preview found definition plus internal unqualified call only.
- Multi-term search was brittle because embeddings were absent:
  - Query `"StartupModule RegisterNomadTabSpawner RegisterMode editor mode tab"` returned 0.
  - Query `"SLandscapeHeightmapTrackerPanel SHeightmapTrackerImageView marker SetMarker SetReverseMarker"` returned 0.
  - Query `"cursor landscape viewport mouse trace HitResult LandscapeSurfaceTraceHelper TraceLandscapeClick"` returned 0.
- Flow detection did not model cross-file Unreal runtime paths involving:
  - module static delegate storage,
  - multicast delegate broadcast/subscription,
  - Slate lambdas/attributes,
  - tab spawner owning a panel widget,
  - editor mode registration producing later `InputKey` calls.
- Impact outputs were noisy:
  - `get_impact_radius` overincluded unrelated editor/module/style files for mapper semantics.
  - `detect_changes` called many impacted nodes "changed functions/classes" even when only mapper files were specified.
  - `get_review_context` contradicted `get_impact_radius`/`detect_changes` for mapper cpp only.
- Parser artifacts:
  - Several headers were treated as language `c`.
  - Several C++ classes in headers became pseudo-function nodes named `class`.

## Verified Runtime Facts

- Initialization and registration:
  - `FLandscapeHeightmapTrackerModule::StartupModule` initializes style/commands, maps UI command, registers editor mode, menus, and nomad tab.
  - `OnSpawnPluginTab` owns initial creation of `SLandscapeHeightmapTrackerPanel`.
- Main image widget:
  - `SHeightmapTrackerImageView` is private to `SLandscapeHeightmapTrackerPanel.cpp`.
  - It draws marker from `MarkerUV` and `HasMarker` attributes in `OnPaint`.
- Coordinate acquisition:
  - `TraceLandscapeClick` reads `GetCursorWorldLocationFromMousePos()`, builds a trace, line-traces, then broadcasts `Hit.ImpactPoint`.
- Marker update:
  - Actual path is click-based, not hover/move-based.
  - `OnViewportClick` maps click world position to image UV/pixel and updates panel marker state.
- Coordinate conversion:
  - World -> local: `FTransform::InverseTransformPosition`.
  - Local -> display UV: `(LocalPosition.X/Y - Bounds.Min.X/Y) / (Bounds.Max.X/Y - Bounds.Min.X/Y)`.
  - Display UV -> pixel: round `UV * (ImageSize - 1)`, then clamp.
  - Display UV -> local: inverse flip/clamp, then `Min + U/V * Width/Height`.

## Benchmark Takeaway

CRG is strong as a codebase orientation tool but weak as the sole source of truth for Unreal editor plugin runtime behavior. It needs source verification for delegates, Slate widget ownership, qualified static call sites, and semantic questions about transforms or coordinate systems.
