# Review UE5-8

## Ticket

LandscapeHeightmapTracker - UE 5.7 Editor Plugin

URL: https://bodomus.youtrack.cloud/issue/UE5-8

## Work Completed

- Created standalone Unreal Engine 5.7 Editor plugin repository in `j:\Projects\UE_Projects\LandscapeHeightmapTracker`.
- Added `LandscapeHeightmapTracker.uplugin` with Editor-only module configuration.
- Added native C++ module `LandscapeHeightmapTracker`.
- Added Slate dockable tab opened from `Tools -> Landscape Heightmap Tracker`.
- Added selected Landscape assignment via `Use Selected Landscape`.
- Added Landscape object picker for direct assignment.
- Added PNG heightmap file loading through Unreal Editor APIs.
- Added transient texture display without importing files into the Content Browser.
- Added fitted image viewer with marker overlay using normalized UV coordinates and letterbox-aware draw rect.
- Added `Track Landscape Clicks`, `Clear Marker`, `Flip X`, and `Flip Y` controls.
- Added coordinate diagnostics for world position, Landscape-local position, UV, pixel, and image size.
- Added safe weak Landscape reference handling.
- Added native editor mode `FLandscapeHeightmapTrackerEdMode` for scoped viewport click observation.
- Added hit filtering so only the assigned Landscape or components owned by it are accepted.
- Added independent coordinate mapper `FLandscapeCoordinateMapper`.
- Added automation tests for mapper center/corners, flips, transforms, non-square bounds, outside rejection, and clamp mode.
- Added documentation: `README.md`, `CHANGELOG.md`, `Docs/ARCHITECTURE.md`, `Docs/INSTALLATION.md`, `Docs/TEST_PLAN.md`.
- Added local ticket markdown: `Tickets/UE5-8-LandscapeHeightmapTracker-UE-5.7-Editor-Plugin.md`.

## Architecture Summary

The module registers commands, style, `Tools` menu integration, a Nomad tab spawner, and a hidden editor mode. The Slate panel owns UI state, Landscape assignment, PNG loading, and marker display. The editor mode observes left-clicks only while tracking is enabled, performs an editor-world visibility trace, broadcasts the hit, and returns `false` so normal editor handling can continue.

Coordinate math is isolated in `FLandscapeCoordinateMapper` and does not depend on Slate.

## Viewport Extension Point

Implemented with a scoped `FEdMode`:

- Registered in `StartupModule()`.
- Activated only when `Track Landscape Clicks` is enabled.
- Deactivated when tracking is disabled, panel is destroyed, or module shuts down.
- Unregistered in `ShutdownModule()`.

No global mouse hooks, polling, sockets, IPC, Python, C#, or external viewer are used.

## Coordinate Convention

Mapping pipeline:

```text
World hit position
-> inverse Landscape actor transform
-> Landscape local XY
-> normalize against ALandscapeProxy::GetBoundingRect()
-> optional Flip X / Flip Y
-> round(U * (Width - 1)), round(V * (Height - 1))
```

`Flip Y` defaults to enabled. Users can adjust `Flip X` and `Flip Y` in the panel.

## Supported Image Formats

- PNG display via UE `ImageWrapper`.
- PNG data is converted to BGRA8 for transient texture display.
- RGB/RGBA PNGs display.
- 8-bit and 16-bit grayscale PNGs are accepted when UE's PNG decoder can convert them to BGRA8.

## Build

Command:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat BuildPlugin -Plugin="j:\Projects\UE_Projects\LandscapeHeightmapTracker\LandscapeHeightmapTracker.uplugin" -Package="j:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\Package" -TargetPlatforms=Win64 -StrictIncludes
```

Result: success.

Note: UE emitted a warning that the installed Visual Studio 2026 compiler version is not Epic's preferred compiler version for UE 5.7, but the plugin built successfully.

## Tests

Command:

```text
C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe j:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\TestHost\TestHost.uproject -unattended -nop4 -nosplash -NullRHI -ExecCmds="Automation RunTests LandscapeHeightmapTracker.Mapper; Quit" -TestExit="Automation Test Queue Empty" -log
```

Result: success, exit code `0`.

Passed tests:

- `LandscapeHeightmapTracker.Mapper.Basic`
- `LandscapeHeightmapTracker.Mapper.OptionsAndTransforms`

## Not Fully Manually Verified

The plugin was compiled and automation-tested in UE 5.7, but a full interactive manual Editor scenario with a real Landscape and PNG click verification was not performed in this environment.

## Known Limitations

- External PNG only.
- No heightmap editing.
- No live hover tracking.
- No image-to-Landscape camera focus.
- No multi-Landscape simultaneous tracking.
- No Content Browser asset workflow for heightmaps.
- Full World Partition/tiled Landscape workflows are documented as future work.

## Recommended Next Ticket

Create an interactive UE 5.7 validation ticket with a real test project and known heightmaps covering:

- 505x505, 1009x1009, and 2017x2017 images.
- Moved/scaled Landscapes.
- Wrong actor hit rejection.
- Map change and Landscape deletion.
- Visual marker alignment under dock resize/floating window resize.
