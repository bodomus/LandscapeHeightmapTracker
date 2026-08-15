# Height Zone Range conversion investigation

## Baseline

- Workflow level: 2 (Landscape height conversion, mapper consistency, Slate
  diagnostics and tests).
- Repository: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
- Branch: `main`.
- Initial commit: `c5d278ee80573128475ad1959953a8cc9642a716`.
- The working tree already contained the in-progress UE5-16 Range implementation;
  it is preserved and treated as the baseline for this bugfix.

## Graph evidence

- Graphify identified the Height Zone community and the architectural path from
  the panel-owned height cache through `FHeightZoneGenerator` to the overlay.
- CRG reports the direct change surface around `ApplyHeightZone`,
  `EnsureHeightMetersCache`, `FHeightZoneGenerator::Generate`, overlay creation,
  public Height Zone types and their tests.
- Direct source inspection confirms that Range already reads only
  `HeightMetersCache`; preview BGRA8 data is used only to create the displayed
  texture.

## Source and engine validation

- UE 5.7's PNG wrapper swaps 16-bit PNG network byte order on little-endian
  platforms before returning `ERGBFormat::Gray` samples.
- `LandscapeDataAccess::GetLocalHeight` is the canonical Raw16-to-local-Z
  conversion used by the Landscape runtime and editor.
- The plugin applies the assigned Landscape actor transform and converts world
  centimeters to meters.
- The current cache discards Raw16 values and exposes only calculated meters.
- Coordinate Diagnostics exposes the mapped pixel and traced surface position,
  but not the height-cache sample used by Range.
- Apply reports contour count, but not selected pixels or coverage.

## Root cause / diagnostic gap

No independent conversion formula or preview-derived Range mask was found. The
observable bug cannot currently be localized because the Raw16 input, cached
world height, traced Landscape surface height and Range membership are not shown
together. The smallest safe fix is to retain Raw16 alongside the meter cache and
derive all diagnostics and Range statistics from those same cached samples.

## Expected blast radius

- Direct: world-height cache data/helper, panel diagnostics/status, focused tests.
- Adjacent: existing in-progress Range generator and UI code.
- Excluded: Marching Squares, editor mode lifecycle, delegates, Landscape data,
  materials and source heightmap assets.

