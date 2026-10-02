# Implementation Report

## Ticket

UE5-20 — LandscapeHeightmapTracker: 3D-маркеры вдоль изолинии.

## Workflow

- Level: 2 (new editor actor/service, Slate workflow, coordinate mapping, tests).
- Graphify skill: used before implementation and refreshed after implementation.
- CRG skill: used before implementation and rebuilt after implementation.
- Repository root: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
- Branch: `codex/ue5-20-contour-3d-markers`.
- Initial commit: `7b272b83857367623d05f01ce63881657c6b5c43` (`main`).
- Initial working tree: clean.

## Investigation

- Existing contour generation produced display-UV polylines but did not expose a
  stable identity or any world actor lifecycle.
- The existing `FLandscapeCoordinateMapper::MapUVToLocalPosition` and Landscape
  actor transform are reused for XY placement. Contour boundary heights already
  come from the world-height cache in meters.
- The smallest coherent change required a pure placement layer, an editor actor,
  an editor transaction service, and one Slate row per connected contour.
- Expected impact was confined to the LandscapeHeightmapTracker editor module,
  contour generation, its panel, and their automation tests.

## Changes

- Added deterministic IDs for connected `FHeightContour` components. Contours at
  the same height are sorted spatially and receive distinct component IDs.
- Added `FContourBoxPlacement`, which:
  - validates dimensions, gap, offsets, and the 10,000-instance limit;
  - resamples by accumulated world-XY length;
  - uses `BoxLength + GapLength` as the step;
  - emits only complete boxes;
  - applies yaw-only tangent orientation;
  - places the bottom at contour world Z plus the configured offset;
  - handles a closed seam without emitting a duplicate instance.
- Added editor-only `ALHTContourBoxesActor` with one HISM using Unreal's built-in
  cube, no collision, hidden-in-game state, a Landscape soft reference, contour
  ID, color, tag, readable label, and Outliner folder.
- Added `FContourBoxesEditorService` for exact Landscape + contour-ID lookup and
  transactional create/update/delete. Repeated identical Create is a no-op and
  does not dirty the level. Update preserves a user-adjusted label and folder.
- Added 3D-box settings plus a dedicated row per connected contour in the Slate
  panel. Each row exposes Create, Update, and Delete; Update/Delete are disabled
  while the managed actor is absent.
- Added placement and editor lifecycle automation tests, same-height contour ID
  coverage, architecture documentation, and test-plan documentation.

## Graph validation

- Pre-change Graphify: refreshed and queried for the panel, contour generation,
  mapping, and editor lifecycle context.
- Post-change Graphify command: `graphify update .`.
  Result: 1,896 nodes, 2,769 edges, 138 communities.
- Focused Graphify evidence found the direct one-hop path
  `SLandscapeHeightmapTrackerPanel::CreateContourBoxes() -> CreateOrUpdate`.
- Post-change CRG commands:
  - `$env:PYTHONUTF8='1'; code-review-graph update --base main --brief`
  - `$env:PYTHONUTF8='1'; code-review-graph build`
  - `$env:PYTHONUTF8='1'; code-review-graph detect-changes --base main --brief`
- Full CRG rebuild result: 60 files, 412 nodes, 3,684 edges.
- Source validation confirmed the active call chain:
  Slate row callback -> panel create/update/delete method -> editor service ->
  coordinate mapper and pure placement -> one actor/HISM rebuild.
- Graphify's longer service-to-actor paths included file/import proximity and were
  not treated as runtime proof; direct source calls were authoritative.

## Post-change impact

- Direct impact: contour identity, the heightmap tracker panel, placement math,
  editor actor/service, and their tests.
- Adjacent impact: existing height-zone generation and multi-range UI refresh.
- No runtime module or external plugin dependency was added.
- No unexpected production dependants were found.
- CRG reported test-gap heuristics for Slate construction/helper symbols even
  though the panel has an offscreen rendered UI test and the service/placement
  paths have dedicated automation coverage.

## Validation

### Build

- Host editor build:
  `Build.bat UE57EditorEditor Win64 Development -Project=J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject -WaitMutex -NoHotReloadFromIDE`
  — succeeded.
- Strict standalone packaging:
  `RunUAT.bat BuildPlugin -Plugin=...\LandscapeHeightmapTracker.uplugin -Package=C:\Temp\LHT20-260923b -TargetPlatforms=Win64 -StrictIncludes`
  — succeeded with unity and shared PCH disabled. Package archive was created at
  `C:\Temp\LHT20-260923b\HostProject\Plugins\LandscapeHeightmapTracker\build\LandscapeHeightmapTracker-Win64-Development.zip`.
- An earlier package attempt under the repository `Saved/Package` path failed
  before compilation because generated action paths exceeded the Windows
  260-character limit; using the short package path resolved the infrastructure
  issue.

### Automation

- `LandscapeHeightmapTracker.ContourBoxes`: 6 succeeded, 0 failed, 0 warnings.
- Final `LandscapeHeightmapTracker.` regression: 35 succeeded, 2 succeeded with
  expected NullRHI warnings, 0 failed, 0 not run.
- `LandscapeHeightmapTracker.UI.MultipleHeightRangesScreenshot` with
  `-RenderOffscreen`: 1 succeeded, 0 warnings, 0 failed.
- `git diff --check`: passed; only the repository's expected LF-to-CRLF notices
  were emitted.
- UI screenshots are stored in `Task/UE5-20/artifacts/`.

## Remaining risks and manual validation

- A real Landscape scene still needs user visual verification of 2D-to-3D
  alignment, material color, gaps, tangent orientation, bottom Z, Outliner
  organization, manual actor deletion recovery, Update, Undo/Redo, PIE hiding,
  and cooked-build exclusion.
- Contour IDs are deterministic for a fixed generated topology. A topology
  split/merge can change spatial component ordering and therefore remap IDs;
  existing generated actors should be deleted/recreated after such a topology
  change.
- The ticket remains in the working state and must not be merged until the visual
  verification is accepted.
