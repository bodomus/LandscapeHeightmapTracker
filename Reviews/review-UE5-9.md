# Review UE5-9

## Root Cause

`FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick()` used the same trace segment for Perspective and Orthographic editor viewports:

```text
Start = Cursor.GetOrigin()
End = Start + Cursor.GetDirection() * HALF_WORLD_MAX
```

That works for Perspective because the ray starts at the camera/cursor origin and travels forward into the scene. In Orthographic views, the cursor origin can sit inside the orthographic ray plane, while the ray direction is parallel for all pixels. A forward-only segment can therefore start on the wrong side of scene geometry and produce unrelated Landscape hit positions.

The coordinate mapper was not changed. Perspective validation and mapper automation coverage indicate the `World -> Landscape Local -> UV -> Pixel` math is not the primary defect.

## Graphify Investigation Before Changes

Graphify was run before code changes from the repository root.

Findings:

- click input entry point: `FLandscapeHeightmapTrackerEdMode::InputKey()` observes left mouse button presses.
- ray construction location: `FLandscapeHeightmapTrackerEdMode::TraceLandscapeClick()`.
- delegate/broadcast path: `TraceLandscapeClick()` broadcasts `FLandscapeHeightmapTrackerModule::OnViewportClickResult()`.
- Landscape filtering location: `SLandscapeHeightmapTrackerPanel::IsAssignedLandscapeHit()`.
- mapper call location: `SLandscapeHeightmapTrackerPanel::OnViewportClick()` calls `FLandscapeCoordinateMapper::MapWorldPosition()`.
- Perspective/Orthographic branch present before fix: no.
- suspected root cause: projection-agnostic forward-only trace segment in the editor mode.

Graphify `path "FLandscapeHeightmapTrackerEdMode" "FLandscapeCoordinateMapper"` returned structural connectivity through include/file nodes, not execution-flow proof, so source inspection was used for the runtime call chain.

## Code Changed

- Added `Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.h`.
- Added `Source/LandscapeHeightmapTracker/Private/ViewportTraceRayBuilder.cpp`.
- Updated `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp`.
- Added `Source/LandscapeHeightmapTracker/Private/Tests/ViewportTraceRayBuilderTests.cpp`.
- Updated `Docs/ARCHITECTURE.md`.
- Updated `Docs/TEST_PLAN.md`.
- Updated `CHANGELOG.md`.
- Added local ticket copy: `Tickets/UE5-9-Fix-Orthographic-Viewport-Click-Mapping-in-LandscapeHeightmapTracker.md`.

## Perspective vs Orthographic Strategy

Perspective behavior remains a forward ray from cursor origin along normalized cursor direction.

Orthographic behavior now uses a finite segment centered on cursor origin:

```text
Start = Origin - Direction * (TraceDistance / 2)
End = Origin + Direction * (TraceDistance / 2)
```

This supports Top, Bottom, Front, Back, Left, and Right views using the viewport-provided direction instead of hardcoded viewport-specific UV swaps or flips.

## Trace Construction

`FViewportTraceRayBuilder::BuildTraceSegment()`:

- normalizes cursor direction;
- rejects near-zero direction vectors;
- rejects non-positive trace distance;
- keeps Perspective forward-oriented;
- makes Orthographic traces span both directions around cursor origin;
- avoids using `HALF_WORLD_MAX` in plugin trace construction.

`TraceLandscapeClick()` now logs viewport type, perspective flag, mouse position, origin, direction, trace start/end, hit actor, hit component, and impact point at `Verbose` level only.

## Tests Added or Updated

Added automation tests:

- `LandscapeHeightmapTracker.ViewportTrace.Perspective`
- `LandscapeHeightmapTracker.ViewportTrace.Orthographic`
- `LandscapeHeightmapTracker.ViewportTrace.ReversedOrthographic`
- `LandscapeHeightmapTracker.ViewportTrace.InvalidDirection`

Existing mapper tests were preserved:

- `LandscapeHeightmapTracker.Mapper.Basic`
- `LandscapeHeightmapTracker.Mapper.OptionsAndTransforms`

## Build Command

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin -Plugin='J:\Projects\UE_Projects\LandscapeHeightmapTracker\LandscapeHeightmapTracker.uplugin' -Package='J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\Package-UE5-9' -TargetPlatforms=Win64 -StrictIncludes
```

## Build Result

Success.

Notes:

- UE emitted the existing Visual Studio 2026 preferred-version warning.
- UE emitted deprecation warnings from engine headers.
- No plugin compile errors.

## Automation Test Command

The existing `Build\TestHost` initially loaded stale plugin binaries and only discovered the two old mapper tests. A direct UBT rebuild of host projects was blocked by active Live Coding, so automation was run against a temporary ignored host using the successfully packaged UE5-9 plugin binaries.

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\UE5-9AutomationHost\UE5_9AutomationHost.uproject' -unattended -nop4 -nosplash -NullRHI -ExecCmds='Automation RunTests LandscapeHeightmapTracker; Quit' -TestExit='Automation Test Queue Empty' -log
```

## Automation Test Result

Success, exit code `0`.

The log found 6 tests:

- `LandscapeHeightmapTracker.Mapper.Basic` - Success
- `LandscapeHeightmapTracker.Mapper.OptionsAndTransforms` - Success
- `LandscapeHeightmapTracker.ViewportTrace.InvalidDirection` - Success
- `LandscapeHeightmapTracker.ViewportTrace.Orthographic` - Success
- `LandscapeHeightmapTracker.ViewportTrace.Perspective` - Success
- `LandscapeHeightmapTracker.ViewportTrace.ReversedOrthographic` - Success

## Manual Validation

Not performed in this run.

Required manual verification remains:

- Perspective center/corners/peak/valley.
- Top View center/corners/peak/valley.
- Optional Bottom/Front/Back/Left/Right validation where terrain visibility is meaningful.
- Regression checks for Flip X, Flip Y, Clear Marker, tracking enable/disable, wrong actor rejection, and marker alignment after tab resize.

## Graphify Validation After Changes

Graphify was refreshed after implementation:

```powershell
graphify update .
```

Result:

```text
281 nodes, 380 edges, 18 communities
graph.json, graph.html and GRAPH_REPORT.md updated in graphify-out
```

After-change queries found:

- `FViewportTraceRayBuilder`
- `FViewportTraceSegment`
- `BuildTraceSegment`
- `ViewportTraceRayBuilderTests.cpp`
- `TraceLandscapeClick`
- `LandscapeHeightmapTracker.ViewportTrace.*` tests

Graphify now shows `LandscapeHeightmapTrackerEdMode.cpp` importing `ViewportTraceRayBuilder.h`, and the new tests calling `BuildTraceSegment()`.

## Known Limitations

- Interactive Top View validation with a real Landscape and source heightmap is still required.
- `LineTraceSingleByChannel()` behavior is unchanged: if a static mesh is the first blocking hit, the panel rejects it and does not search behind it for the assigned Landscape.
- Side orthographic views can be ambiguous on steep or occluded terrain, but trace construction is now geometrically valid for reversed orthographic directions.
- Existing Live Coding state blocked direct UBT rebuild of the old `Build\TestHost`; tests were run through a temporary ignored host with packaged plugin binaries.

## Recommended Next Ticket

Perform manual UE 5.7 validation on a real Landscape using known 1009x1009 source heightmap landmarks, then decide whether to add `LineTraceMulti` support to select the first hit belonging to the assigned Landscape behind unrelated blockers.
