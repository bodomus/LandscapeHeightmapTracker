# UE5-16 Investigation

## Baseline

- Workflow level: 2 (multi-file UI, data model, generation and rendering change).
- Repository: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
- Branch: `main` (the repository has no `master` branch).
- Initial commit: `c5d278ee80573128475ad1959953a8cc9642a716`.
- Pre-existing untracked files: UE5-14 screenshots under `Reviews/` and `images/`;
  they are unrelated and must remain untouched.

## Ticket intent

Add a second height and a `Range` mode which selects all cached world heights in
the inclusive, normalized interval between the two values. Preserve Above,
Below, Contour Only, map transforms, tracking, markers and diagnostics.

## Graphify

- Existing graph reused; scoped query successfully found the Height Zone
  community and the `HeightMetersCache → FHeightZoneGenerator →
  FHeightZoneResult → SLandscapeHeightmapTrackerPanel` boundary.
- Relevant Graphify commands:
  - `graphify query "Height Zone overlay HeightZoneGenerator SLandscapeHeightmapTrackerPanel height cache contours" --budget 3000`
  - `graphify explain FHeightZoneGenerator`
- Graph findings were treated as navigation hints and validated in source.

## CRG

- CRG initially represented commit `3a033d8`; it was incrementally updated to
  current `main` with `code-review-graph update --brief --base 3a033d8066ca`.
- Update completed for 11 files and rebuilt FTS. The final console-panel print
  hit a Windows cp1251 Unicode encoding error after the successful update.
- Graph metadata now reports `head_matches_build: true` at `c5d278e`.
- CRG identifies `ApplyHeightZone` callees including cache preparation, overlay
  texture update, clear-on-error, status update and paint invalidation.
- CRG does not connect the static C++ call to `FHeightZoneGenerator::Generate`;
  direct source inspection confirms it at the Apply path. This is the only
  relevant graph/source coverage discrepancy.

## Source-verified current behavior

- `FHeightZoneSettings` stores one target, mode, opacity, line thickness and
  enabled state.
- `FHeightZoneGenerator::Generate` owns threshold mask and Marching Squares.
- `ApplyHeightZone` validates the target against the cached Landscape range and
  invokes the generator once; no generation happens in `OnPaint`.
- `SHeightmapTrackerImageView::OnPaint` draws base map, mask texture, contours,
  then markers, keeping all layers in the same fitted image rectangle.
- Cache invalidation already covers Landscape transform, bounds, image size,
  bit depth and flip options.
- Existing automation tests cover flat masks, hills/pits, islands/hole,
  boundary/saddle, exact thresholds and negative heights.

## Smallest correct change

1. Extend the public data object with two input heights, normalized result bounds
   and the `Range` enum value.
2. Extend the generator API to accept both heights. For Range, generate an
   inclusive mask and append contours for both normalized boundaries. Avoid
   duplicate contour generation when both boundaries are equal.
3. Add the second numeric control and Range combo option. For non-Range modes,
   preserve use of the first height.
4. Give Range a distinct overlay color while retaining the existing layer order.
5. Add focused tests for inclusive masking, reverse-input normalization, two
   boundaries and equal-boundary behavior.

## Expected blast radius

Direct:

- `HeightZoneTypes.h`
- `HeightZoneGenerator.h/.cpp`
- `SLandscapeHeightmapTrackerPanel.cpp`
- `HeightZoneGeneratorTests.cpp`

Adjacent:

- `SLandscapeHeightmapTrackerPanel.h` only if declarations/state names require it.
- Existing Height Zone tests and the compact-layout compile path.

No changes are expected in coordinate mapping, height cache, editor-mode input,
marker delegates or Landscape asset data.

## 3D visualization investigation

The plugin already has `FLandscapeHeightmapTrackerEdMode::Render`, but it only
draws a cheap diagnostic line through `FPrimitiveDrawInterface`. An exact filled
range surface would require a maintained dynamic mesh/view extension (including
sampling, ownership, invalidation and render-thread lifecycle) or material-based
rendering forbidden by the ticket. Drawing only boundary debug lines would not
satisfy the stated surface-highlight semantics.

Therefore UE5-16 uses the ticket's fallback: complete the 2D Range mode and make
the normalized bounds, mask and threshold-tagged contours reusable by a future
editor-only 3D consumer. No Landscape material, layer, weightmap, heightmap,
geometry, Actor or Component is modified.

