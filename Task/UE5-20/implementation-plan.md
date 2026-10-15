# UE5-20 Implementation Plan

Status: approved on 2026-09-23. Implement one dedicated row and one actor for each
connected `FHeightContour`.

## 1. Model and pure placement math

- Add `FContourBoxSettings` defaults in Unreal centimeters:
  1000 length, 1000 height, 200 thickness, 1000 gap, zero Z offset.
- Validate finite positive dimensions, nonnegative gap, and a configurable maximum
  instance count.
- Convert display-UV contour points with the existing mapper and Landscape
  transform.
- Normalize open/closed polylines, remove zero-length segments, compute cumulative
  world-XY arc length, and place only full box spans.
- Sample box centers at `BoxLength / 2 + N * (BoxLength + GapLength)`.
- Derive the tangent across the occupied span and emit yaw-only transforms.
- Set world center Z to boundary world Z + half height + offset.
- Avoid a second sample at the closed seam.

## 2. Stable contour identity

- Give every source height range a persistent runtime GUID when it is added.
- Propagate source range/boundary identity into generated contours.
- Add a deterministic component discriminator for disconnected contours at the
  same height.
- Store the complete identifier on the generated actor as a UPROPERTY and actor
  tag; never locate actors only by label, height, or folder.
- Document the behavior when topology splits/merges during regeneration.

## 3. Editor-only actor

- Add `ALHTContourBoxesActor` with one root HISM component.
- Mark actor/component editor-only, hidden in game, non-colliding, non-replicated,
  and excluded from cooked runtime through the editor module.
- Load Unreal's built-in cube mesh without creating a Content Browser asset.
- Persist source contour identity, boundary height, settings, and source Landscape
  reference needed for safe update.
- Set stable label and folder under
  `LandscapeHeightmapTracker/GeneratedContours/Height_<Value>`.

## 4. Editor service and lifecycle

- Find exact managed actor by class + plugin tag + stored contour identity.
- Create and update as one `FScopedTransaction`; call `Modify()` before mutation.
- Rebuild HISM instances from pure generator output.
- Delete only the exact managed actor in one transaction.
- Treat manually deleted actors as absent and refresh row state safely.
- Mark package/level dirty only when an actor or instance set really changes.
- Reject invalid Landscape/world/level, non-game-thread mutation, invalid settings,
  short contours, and excessive instance counts with actionable errors.

## 5. Slate UI

- Add numeric controls for Length, Height, Thickness, Gap, and Z Offset.
- Add Create/Update/Delete buttons at the granularity selected by the user.
- Compute button enablement from actor lookup; Create is idempotent and Update/
  Delete are disabled when the actor is absent.
- Do not create or update actors from paint/tick.
- Preserve current overlay rendering, range selection, and marker behavior.

## 6. Automated tests

- Default step: 1000 + 1000 = 2000 cm.
- Straight polyline placement count and centers.
- Horizontal, vertical, and diagonal yaw.
- Center Z with height and offset.
- Closed seam without duplication.
- Too-short open and closed fragments.
- Same height with different stable identities.
- Instance-limit rejection.
- Editor-world Create/Create/Update/Delete isolation and transaction behavior where
  the automation environment supports world actors and undo.
- Regression runs for HeightZone, HeightRanges, Mapper, and UI groups.

## 7. Documentation and validation

- Update architecture and test plan.
- Rebuild the UE57Editor editor target.
- Run `LandscapeHeightmapTracker.ContourBoxes.*`, then affected existing groups,
  then the full plugin automation namespace if time/environment permit.
- Update CRG, inspect changed-symbol impact, and refresh Graphify because this adds
  a new actor/service subsystem and new panel-to-world relationships.
- Produce `Task/UE5-20/implementation-report.md` and `Reviews/review-UE5-20.md`.
- Update YouTrack fields, attach required artifacts, and leave the ticket awaiting
  visual verification; do not merge.
