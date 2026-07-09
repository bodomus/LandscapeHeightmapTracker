# UE5-9: Fix Orthographic Viewport Click Mapping in LandscapeHeightmapTracker

URL: https://bodomus.youtrack.cloud/issue/UE5-9

Status at start: `Open` / `Открыта`

Assignee: `ChatGPT`

## Summary

Fix incorrect click-to-heightmap mapping in orthographic Unreal Editor viewports, starting with Top View.

Perspective viewport mapping currently works correctly: center, corner regions, distinctive mountain peaks, and valleys map to the expected 2D heightmap positions. In Top View, clicks map to incorrect and apparently unrelated positions.

The likely defect is in viewport click ray construction inside the editor-mode integration layer, not in `World -> Landscape Local -> UV -> Pixel` mapping.

## Core Requirements

- Preserve existing Perspective viewport behavior.
- Fix Top orthographic viewport click mapping.
- Support standard orthographic viewport directions where possible: Top, Bottom, Front, Back, Left, Right.
- Do not add projection-specific hacks to `FLandscapeCoordinateMapper`.
- Construct projection-aware trace segments.
- Normalize direction vectors and reject invalid near-zero directions.
- Add low-noise Verbose/VeryVerbose diagnostics for ray construction and hit data.
- Keep assigned Landscape hit filtering intact.
- Add or update tests for trace construction where practical.
- Existing mapper automation tests must continue to pass.
- Build against Unreal Engine 5.7.
- Refresh Graphify after implementation and include before/after Graphify findings in `Reviews/review-UE5-9.md`.

## Required Investigation

Use Graphify before code changes to inspect:

- viewport click input flow;
- editor mode registration and activation;
- click delegate broadcast path;
- Landscape hit filtering;
- coordinate mapping call chain;
- Perspective/Orthographic handling.

Required query equivalents:

```powershell
graphify query "How does LandscapeHeightmapTracker receive a viewport click and turn it into a heightmap marker?"
graphify query "Where is the editor viewport ray constructed for LandscapeHeightmapTracker and which functions depend on it?"
graphify query "How does FLandscapeHeightmapTrackerEdMode connect to SLandscapeHeightmapTrackerPanel and FLandscapeCoordinateMapper?"
graphify query "What code handles Perspective versus Orthographic editor viewport projection in LandscapeHeightmapTracker?"
graphify path "FLandscapeHeightmapTrackerEdMode" "FLandscapeCoordinateMapper"
```

## Deliverables

1. Source code fix.
2. Tests.
3. Updated documentation if viewport behavior or limitations changed.
4. Updated `CHANGELOG.md`.
5. Review report: `Reviews/review-UE5-9.md`.

## Review Report Required Sections

```text
# Review UE5-9
## Root Cause
## Graphify Investigation Before Changes
## Code Changed
## Perspective vs Orthographic Strategy
## Trace Construction
## Tests Added or Updated
## Build Command
## Build Result
## Automation Test Command
## Automation Test Result
## Manual Validation
## Graphify Validation After Changes
## Known Limitations
## Recommended Next Ticket
```
