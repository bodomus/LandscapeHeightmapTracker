# UE5-12 Investigation

## Scope and workflow

- Ticket: UE5-12 — height zones on the 2D Landscape map.
- Workflow level: 2 (new height-data, contour-generation, Slate rendering, and UI behavior).
- Baseline: clean `main` at `81ad9c2412a633d6732d8ae18b35141853a78646`.
- Working branch: `codex/UE5-12-height-zones` (the repository has no `master`; `main` is the only primary branch).

## Repository intelligence

### Graphify

The existing `graphify-out/graph.json` is readable and contains the current panel,
mapper, analyzer, and tests. Focused queries and explanations identified:

- `SHeightmapTrackerImageView` as the fitted-image and marker paint owner;
- `SLandscapeHeightmapTrackerPanel::LoadPngTexture` as the PNG decode boundary;
- `FLandscapeCoordinateMapper` as the existing display-UV/Landscape-local transform boundary;
- `FHeightmapImageInfoAnalyzer` as analysis-only code that currently discards decoded samples.

Graphify was used for candidate discovery only. Runtime and data-flow conclusions
below were verified in source.

### CRG

CRG was incrementally refreshed on the ticket branch and matched HEAD. It reports
29 indexed source/build files, 160 nodes, and 1111 edges. Exact caller analysis
confirmed that `LoadPngTexture` is reached only from panel construction (restoring
the saved image) and the file-picker handler. The panel remains the narrow
integration boundary. CRG misses some Unreal/Slate relationships and did not link
existing macro-generated automation tests to `Analyze`; direct source inspection
is authoritative for those paths.

## Current behavior

- `LoadPngTexture` decodes 8-bit or 16-bit grayscale samples, computes information,
  converts the image to BGRA8, uploads it to a transient texture, and then drops the
  grayscale samples.
- No height-in-meters cache or raw-height-to-world-height conversion exists.
- `SHeightmapTrackerImageView::OnPaint` draws the fitted base image and then hover
  or click crosshair markers.
- The fitted rectangle is shared by drawing and click mapping, so normalized map
  coordinates stay aligned during panel resize.
- `FLandscapeCoordinateMapper::MapUVToLocalPosition` already centralizes display
  flips and conversion to Landscape-local XY.
- The current widget has fit-to-window resizing but no independent zoom/pan feature.

## Expected behavior

Applying a target height must build an Above/Below mask and all interpolated
Marching Squares contours once, display them above the base map and below markers,
and clear them without disturbing existing marker state. Heights must be true world
Z meters derived from Landscape height encoding and actor transform.

## Implementation gap and smallest coherent change

1. Preserve decoded grayscale samples after PNG load.
2. Add a dedicated height-data cache builder that:
   - expands 8-bit samples to the UE 16-bit height domain;
   - uses UE 5.7 `LandscapeDataAccess::GetLocalHeight`;
   - uses existing display-UV-to-local-XY mapping;
   - applies the Landscape actor transform;
   - converts world centimeters to meters;
   - records minimum and maximum.
3. Add a UI-independent height-zone generator for mask and Marching Squares
   contours, including a deterministic saddle rule and open boundary contours.
4. Upload the generated mask to a transient overlay texture and pass the prepared
   brush/contours to the existing image view.
5. Draw base image, mask, contours, then existing markers in increasing Slate
   layers. `OnPaint` performs no height or contour computation.
6. Add synthetic automation tests for masks, islands, holes, boundaries, saddle
   cells, equal vertices, negative heights, finiteness, and height conversion.

## Formula

For each displayed sample:

```text
8-bit sample -> sample * 257
16-bit sample -> uint16 sample
LocalZ = LandscapeDataAccess::GetLocalHeight(sample16)
DisplayUV -> FLandscapeCoordinateMapper::MapUVToLocalPosition -> LocalXY
WorldZCm = LandscapeTransform.TransformPosition(LocalXY, LocalZ).Z
HeightMeters = WorldZCm / 100
```

Using `TransformPosition` (not only Z scale) also handles translation and rotated
Landscape actors consistently.

## Direct impact and risks

- Direct: panel header/implementation, new height-data and generator types/tests,
  mapper tests if the conversion boundary is extended there.
- Adjacent: transient texture lifetime, Slate invalidation, flip changes,
  Landscape reassignment and transform changes.
- Existing marker delegates and editor-mode lifecycle do not need modification.
- External PNG values can differ from a subsequently sculpted Landscape; the zone
  intentionally represents the loaded heightmap source, matching the ticket input
  model. Manual validation must use the source PNG imported into the Landscape.
- Large maps are calculated synchronously on Apply. Nothing runs per frame; worker
  threading remains a future optimization if editor responsiveness requires it.

