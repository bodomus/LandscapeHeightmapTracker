# UE5-20 Investigation

## Baseline

- Repository root: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`
- Initial branch: `main`
- Working branch: `codex/ue5-20-contour-3d-markers`
- Initial commit: `7b272b83857367623d05f01ce63881657c6b5c43`
- Initial working tree: clean
- Workflow level: 2 (new editor actor/component workflow, Slate UI, coordinate
  conversion, transactions, tests)

## Repository intelligence

### Graphify

- Confirmed CLI: `graphify --help`.
- Existing graph answered scoped queries for `FHeightContour`,
  `SLandscapeHeightmapTrackerPanel`, height ranges, contour generation, and the
  coordinate mapper.
- Refreshed with `graphify update .` on 2026-09-23.
- Updated graph: 1764 nodes, 2518 edges, 130 communities.
- Focused query identified the panel, `FHeightContour`,
  `FHeightZoneGenerator`, `FHeightRangeGenerator`, mapper, settings, and tests as
  the owning working set.

### code-review-graph

- Confirmed CLI: `code-review-graph --help`.
- Initial index was stale: branch `codex/UE5-19-scanvault-importer-v1`, commit
  `54804f9ec75b`.
- Rebuilt with `code-review-graph build`.
- Current index: 60 files, 403 nodes, 3489 edges; branch
  `codex/ue5-20-contour-3d-markers`, commit `7b272b838573`.
- `code-review-graph detect-changes --base main --brief` analyzed the ticket
  artifact but its rich console panel hit a Windows CP1251 encoding error after
  emitting the machine-readable summary. No production symbols were changed at
  preflight time.

## Current behavior

1. `FHeightZoneGenerator` creates connected `FHeightContour` polylines with
   normalized display-UV points, boundary height, color, and `bClosed`.
2. `FHeightRangeGenerator` creates the unique lower/upper boundary heights for
   enabled ranges and appends every connected contour component into one flat
   `FMultiHeightRangeResult::Contours` array.
3. `SHeightmapTrackerImageView::OnPaint` renders those contours on the 2D image.
4. `SLandscapeHeightmapTrackerPanel` owns the assigned Landscape, mapping flags,
   heightmap dimensions, generated contour arrays, and the existing list UI.
5. The existing list is a **height-range list**, not a contour list. A row is a
   `FHeightRangeDefinition` displayed as `Min - Max m`.
6. One range can create two boundary heights; each boundary can create several
   disconnected open or closed `FHeightContour` objects.
7. Reverse mapping already uses
   `FLandscapeCoordinateMapper::MapUVToLocalPosition`, then the Landscape actor
   transform. The height cache converts canonical Landscape raw height to world
   meters.
8. There is no persistent 3D actor, contour identifier, instancing component,
   box-placement math, or Create/Update/Delete lifecycle.

## Expected behavior

- Configure Length/Height/Thickness/Gap/Z Offset with ticket defaults.
- Expose Create/Update/Delete for one selected connected contour.
- Convert normalized contour UV to Landscape-local XY and then world space.
- Resample by accumulated world-space arc length and emit one transform per
  complete box segment.
- Use yaw-only orientation and set the bottom face at contour world Z plus offset.
- Keep one editor-only actor with one HISM/ISM per managed contour.
- Identify actors without relying on label or rounded height.
- Support transaction-safe create/update/delete and safe manual actor deletion.
- Limit instance count and keep all UObject changes on the editor thread.

## Architectural gap

The ticket says to add actions to each existing isoline row, but the current UI
has no rows for individual connected isolines. Mapping one height-range row to an
operation is ambiguous because it may address two heights and many disconnected
polylines. This directly affects actor granularity, identifiers, button state,
delete scope, same-height isolation, and acceptance tests.

## Smallest coherent implementation after UI decision

1. Add pure placement settings/result types and a generator independent of Slate
   and UObject creation.
2. Add stable source identity to generated contour rows.
3. Add one editor-only `ALHTContourBoxesActor` with a HISM component and stored
   contour identity/metadata.
4. Add a small editor service for find/create/update/delete with transactions,
   naming, Outliner folder, mesh setup, collision/game visibility, and limits.
5. Bind panel controls and row buttons to that service.
6. Add pure math tests plus editor integration tests where the host allows them.

## Directly affected symbols

- `FHeightContour`
- `FHeightRangeDefinition` / boundary generation identity propagation
- `FHeightZoneGenerator::Generate`
- `FHeightRangeGenerator::Generate`
- `FLandscapeCoordinateMapper::MapUVToLocalPosition` (reuse, not necessarily modify)
- `SLandscapeHeightmapTrackerPanel::GenerateHeightRangeRow` or a new contour-row
  generator, depending on the UI decision
- `SLandscapeHeightmapTrackerPanel::RebuildMultiHeightRangeVisualization`
- `SLandscapeHeightmapTrackerPanel::ApplyHeightZone`
- new placement generator, actor, and editor service

## Adjacent impact

- Existing height-zone/range generator tests because contour identity becomes part
  of generated data.
- Existing UI screenshot test because row layout changes.
- Plugin module build dependencies and generated-header compilation.
- Map/level changes and weak Landscape references.
- Undo/Redo, level package dirtiness, Outliner labels/folders, PIE visibility.

## Source validation

- Graphify's candidate ownership matched direct source inspection.
- The current range-row/contour mismatch was established from
  `GenerateHeightRangeRow`, `FHeightRangeGenerator::Generate`, and
  `FHeightContour` source, not inferred from graph connectivity.
- Coordinate conversion was verified in `LandscapeCoordinateMapper.cpp` and
  `HeightmapWorldHeightCache.cpp`.
- Existing editor transaction style was inspected in
  `LandscapePaintLayerService.cpp`.

## Tests and build environment

- Host project: `J:/Projects/UE_Projects/UE57Editor/UE57Editor.uproject`.
- Engine documented by prior project reports: `C:/Program Files/Epic Games/UE_5.7`.
- Editor target: `UE57EditorEditor Win64 Development`.
- Existing automation groups include `LandscapeHeightmapTracker.HeightZone.*`,
  `LandscapeHeightmapTracker.Mapper.*`, and `LandscapeHeightmapTracker.UI.*`.
- Required new group: `LandscapeHeightmapTracker.ContourBoxes.*`.

## Product decision

Approved on 2026-09-23: after generation, show one dedicated row per connected
`FHeightContour`. Each row owns exactly one actor and independently satisfies the
same-height and delete-isolation requirements.
