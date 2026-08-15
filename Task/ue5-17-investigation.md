# UE5-17 Investigation

## Baseline

- Workflow level: 2 (new reusable data model/generator, Slate list state,
  rendering behavior and validation).
- Repository: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
- Branch: `main`.
- Initial commit: `c5d278ee80573128475ad1959953a8cc9642a716`.
- The working tree contains the completed but uncommitted UE5-16 Range work and
  its conversion bugfix. UE5-17 depends on that state and must preserve it.

## Ticket intent

Add a model-owned, sorted collection of non-overlapping colored height ranges.
All enabled ranges render simultaneously, can be toggled/removed, and remain
usable by future non-Slate consumers.

## Graphify

- Existing graph reused.
- Scoped queries identified the Height Zone community and the
  `HeightMetersCache -> FHeightZoneGenerator -> FHeightZoneResult -> panel`
  boundary.
- Graphify does not yet contain multi-range symbols; its findings were used only
  for orientation and verified directly in source.

## CRG

- Existing graph is current at the initial commit and includes the uncommitted
  UE5-16 code through its last incremental update.
- Direct structural scope: Height Zone types/generator, panel construction,
  Apply/Clear, overlay texture creation, paint path and generator tests.
- The panel is graph-adjacent to tracking and mapping, but source proves those
  paths consume only the shared image widget/marker state and need no behavior
  change.

## Source-verified current behavior

- `FHeightZoneSettings` owns one pair of inputs and one mode.
- `FHeightZoneGenerator` produces a byte mask and contours for one threshold or
  interval.
- The panel creates one BGRA overlay texture from that mask.
- `SHeightmapTrackerImageView::OnPaint` draws base texture, overlay, contours,
  then hover/click markers.
- Range heights already come exclusively from `HeightMetersCache`.
- The project already uses `FMessageDialog` for explicit editor confirmation.
- No ListView currently exists in the Height Zone panel.

## Architecture decision

1. Add `FHeightRangeDefinition` and `FMultiHeightRangeResult` to the public
   Height Zone data types.
2. Add UI-independent `FHeightRangeGenerator` for normalization, duplicate /
   strict-overlap validation, pixel ownership and deduplicated contours.
3. Keep `TArray<FHeightRangeDefinition>` as the source of truth; ListView items
   are only stable indices rebuilt after sorting.
4. Use one `RangeIndexByPixel` array. Each pixel is owned by at most one enabled
   range and the panel maps that owner to its stored color.
5. Use `[Min, Max)` for every enabled range except the enabled range with the
   greatest maximum, which uses `[Min, Max]`. A shared boundary therefore
   belongs to the upper range.
6. Generate each distinct enabled boundary once by reusing the existing contour
   generator. For a shared boundary, use the upper range's color.
7. Preserve the existing single-zone generator and Apply path when the
   multi-range model is empty or a non-Range mode is selected.

## Expected blast radius

Direct:

- `HeightZoneTypes.h`
- new `HeightRangeGenerator.h/.cpp`
- `SLandscapeHeightmapTrackerPanel.h/.cpp`
- new generator tests
- module build/test documentation

Adjacent:

- `FHeightContour` gains color metadata used by the existing paint path.
- Existing Height Zone generator tests validate legacy compatibility.

Excluded:

- height conversion and mapping;
- editor mode / delegates;
- Landscape materials, layers, geometry and assets;
- PCG / foliage / persistence.

