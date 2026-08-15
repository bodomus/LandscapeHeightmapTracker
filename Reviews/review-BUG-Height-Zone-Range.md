# Review — Height Zone Range conversion bug

## Verdict

Ready for interactive verification. The concrete pre-mask rejection bug is fixed,
conversion and cache diagnostics are visible, and automated regression validation
passes.

## Root cause

The requested `100–800 m` interval overlaps the real calculated map range
`-588.728125…588.782031 m`, but the panel required both requested endpoints to be
inside that range. It therefore stopped before `HeightMetersCache → Range Mask →
Overlay`. Range validation now uses interval overlap.

## Review findings

- Range mask still reads only `HeightMetersCache`.
- Raw16 decoding and world-height conversion follow UE 5.7 engine behavior.
- Preview BGRA8 pixels do not participate in mask generation.
- Marching Squares is unchanged by this bugfix.
- Partially overlapping and reversed intervals are covered by direct assertions;
  fully disjoint ranges remain rejected.
- Coordinate Diagnostics uses the same cache index as the mask and additionally
  compares it with the traced Landscape surface Z.
- No lifecycle, delegate, ownership, Landscape asset, material, or render-thread
  changes were introduced.

## Validation

- UE57Editor Development build: passed.
- Targeted Height Zone automation: 8/8 passed.
- Full plugin automation: 22/22 passed.
- Post-change CRG: updated; no affected flows or unexpected subsystem escape.
- `git diff --check`: passed apart from line-ending notices.

## Interactive check

Assign `Landscape_1`, load the recorded 505×505 PNG, apply `100–800 m`, and
confirm the status reports approximately `35017` selected pixels and `13.73%`
coverage. Click several points and verify that the calculated/surface delta is
small when the PNG and Landscape still match.

