# UE5-8: LandscapeHeightmapTracker - UE 5.7 Editor Plugin

URL: https://bodomus.youtrack.cloud/issue/UE5-8

Status at start: `Open` / `Открыта`

Assignee: `ChatGPT`

## Summary

Create a native Unreal Engine 5.7 C++ Editor Plugin named `LandscapeHeightmapTracker`.

The plugin must allow an Unreal Editor user to select or assign a Landscape Actor, load an external heightmap image, open a dockable Unreal Editor tab, click a point on the 3D Landscape in the editor viewport, convert that world-space point into Landscape-local normalized coordinates, and display the corresponding point on the 2D heightmap image.

The primary purpose is terrain inspection and debugging. This is an Editor-only plugin and must not add runtime gameplay systems or packaged-game dependencies.

## Core Requirements

- Unreal Engine 5.7 native C++ Editor plugin.
- Editor-only module.
- Slate UI.
- No WPF, WinForms, C#, external executable, Python dependency, Blueprint-only implementation, network bridge, localhost socket, UDP/TCP IPC, or external viewer.
- Exact plugin/module/repository name: `LandscapeHeightmapTracker`.
- Dockable tab named `Landscape Heightmap Tracker`.
- Tools menu entry: `Tools -> Landscape Heightmap Tracker`.
- Landscape selector and `Use Selected Landscape` flow.
- External PNG heightmap loading and preview.
- `Track Landscape Clicks` toggle and `Clear Marker`.
- Flip X and Flip Y controls.
- World, local, UV, pixel diagnostics.
- Marker overlay aligned to fitted image draw rect, including resized/letterboxed panels.
- Dedicated coordinate mapper independent from Slate UI.
- Reject outside Landscape bounds by default.
- Validate hits against assigned Landscape only.
- Safe weak object references for editor map changes/deletions.
- Dedicated log category.
- No per-frame heavy work or log spam.
- Pure mapping automation tests where possible.
- Documentation: `README.md`, `Docs/ARCHITECTURE.md`, `Docs/INSTALLATION.md`, `Docs/TEST_PLAN.md`.

## Acceptance Snapshot

The ticket is complete when a user can install the plugin in an Unreal Engine 5.7 project, compile, open the tab, assign a Landscape, load the original PNG heightmap, enable tracking, click the 3D Landscape, and see the corresponding marker and diagnostics on the 2D image. Mapping must survive Landscape translation/scale, multiple components, wrong-actor clicks, map changes, and tab reopening without crashes.

## Implementation Notes

The full YouTrack description contains detailed sections 1-27 covering architecture, UI, mapping, viewport interaction, testing, documentation, deliverables, and Definition of Done. This local file preserves the actionable ticket text used for execution and is saved according to the project rule in `Tickets/`.
