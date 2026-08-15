# BUG — Diagnose incorrect Height Zone Range height conversion

## Problem

`Height Zone` in `Range` mode does not select the expected heightmap regions for
`100–800 m`.

## Requirements

- Extend Coordinate Diagnostics with pixel coordinates, Raw16 value, normalized
  height, Landscape local/world Z, calculated meters, selected range and an
  inside/outside verdict.
- Report the complete raw and calculated height ranges after the height cache is
  built.
- Build the Range mask exclusively from `HeightMetersCache` using an inclusive,
  normalized interval.
- Use the canonical Unreal Landscape conversion: unsigned 16-bit height,
  midpoint `32768`, Landscape Z scale, actor transform/location and centimeters.
- Compare the calculated height with the corresponding traced Landscape surface.
- After Apply Range, report total pixels, pixels inside the interval and coverage.
- Do not change Marching Squares in this ticket.

## Acceptance

- Calculated minimum and maximum height are visible.
- Selected pixels expose X/Y, Raw16, calculated meters and Inside Range YES/NO.
- Apply Range exposes pixel count and coverage.
- The concrete fault is isolated to conversion, masking or rendering and fixed at
  the responsible stage.

