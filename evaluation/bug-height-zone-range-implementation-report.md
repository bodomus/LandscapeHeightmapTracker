# Implementation Report

## Ticket

BUG — Diagnose incorrect Height Zone Range height conversion.

## Workflow

- Level: 2.
- Graphify skill: used; existing graph reused for scoped Height Zone/cache
  queries because subsystem boundaries did not change.
- CRG skill: used before and after implementation.
- Working tree before changes: dirty with the in-progress UE5-16 Range
  implementation and unrelated untracked artifacts; all were preserved.

## Preflight

- Repository root: `J:/Projects/UE_Projects/UE57Editor/Plugins/LandscapeHeightmapTracker`.
- Branch: `main`.
- Initial commit: `c5d278ee80573128475ad1959953a8cc9642a716`.
- Graphify graph: existing and queryable.
- CRG graph: existing, current at the initial commit, then incrementally updated.
- Investigation: `Task/bug-height-zone-range-investigation.md`.
- Plan: `Task/bug-height-zone-range-implementation-plan.md`.

## Investigation

- Current behavior: `ApplyHeightZone` rejected Range whenever either requested
  endpoint was outside the calculated map range. Therefore a partially
  overlapping `100–800 m` selection returned before mask generation when the
  map maximum was below `800 m`.
- Expected behavior: a Range is applicable whenever the requested interval and
  available height interval overlap. The inclusive mask should then select the
  available samples inside the requested interval.
- Root cause: over-strict pre-mask endpoint validation, not Raw16 conversion,
  preview rendering, or Marching Squares.
- Main symbols: `FHeightmapWorldHeightCache::Build`,
  `FHeightZoneGenerator::DoesRangeOverlap`, `ApplyHeightZone`, and panel
  diagnostic getters.
- Owning subsystem: Height Zone/cache path in `SLandscapeHeightmapTrackerPanel`.

## Real-data diagnosis

Inputs used:

- PNG: `heightmap_-125.30244282141354_61.54080204875157_4.png`, 505×505,
  16-bit grayscale.
- Landscape: `/Game/NewWorld`, actor `Landscape_1`.
- Transform: location Z `0 cm`, rotation `0`, scale Z `230`.
- Raw16 range: `4…65535`.
- Calculated height range: `-588.728125…588.782031 m`.

For requested Range `100…800 m`:

- Total pixels: `255025`.
- Pixels inside Range: `35017`.
- Coverage: `13.730811%`.

Control samples:

| Pixel X/Y | Raw16 | Normalized | Calculated meters | Inside 100–800 m |
|---|---:|---:|---:|---|
| 0 / 0 | 29455 | 0.449454 | -59.530469 | NO |
| 120 / 0 | 40243 | 0.614069 | 134.316406 | YES |
| 216 / 0 | 65535 | 1.000000 | 588.782031 | YES |

## Changes

- Retained Raw16 samples and their min/max alongside `HeightMetersCache`.
- Centralized normalized-height and local-Z diagnostics on the same canonical
  `LandscapeDataAccess::GetLocalHeight` conversion used by cache construction.
- Added full-map raw/calculated range output to Coordinate Diagnostics and logs.
- Added selected-pixel Raw16, normalized height, local Z, calculated world Z and
  meters, traced surface Z, delta, Range bounds, and Inside Range output.
- Added Apply Range total pixels, selected pixels and coverage output.
- Changed Range validation from “both endpoints must be inside” to inclusive
  interval-overlap validation. Legacy single-threshold modes keep their prior
  validation.
- Added overlap and cache diagnostic automation coverage.
- Marching Squares was not changed by this bugfix.

## Graph validation

- Graphify located the existing cache → generator → result → panel path; source
  confirmed the runtime calls and that BGRA8 preview data is not used by Range.
- UE 5.7 engine source confirmed native-endian PNG Raw16 decoding and canonical
  `LandscapeDataAccess::GetLocalHeight` usage.
- Post-change CRG update: 10 files, 98 nodes, 1029 edges refreshed; graph total
  243 nodes / 1986 edges / 45 files.
- CRG reported risk 0.60 and no affected flows. Its test-gap heuristic did not
  connect the overlap helper to its direct automation assertions; source and the
  successful Range test confirm that coverage.
- No graph/source disagreement changed the implementation decision.

## Validation

Build command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' UE57EditorEditor Win64 Development '-Project=J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -WaitMutex -NoHotReloadFromIDE
```

Result: succeeded; plugin DLL and package were produced.

Automation command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoSplash -NullRHI -NoSound -NoP4 '-ExecCmds=Automation RunTests LandscapeHeightmapTracker.; Quit' '-TestExit=Automation Test Queue Empty' -log
```

Result: 22/22 successful, exit code 0. Targeted Height Zone run also passed 8/8.

`git diff --check`: passed; only repository line-ending notices were emitted.

## Remaining risks

- Traced surface Z/delta requires interactive point selection in the Editor; the
  new UI and log output expose it, but unattended NullRHI validation cannot
  perform a real viewport surface click.
- If the source PNG no longer matches a sculpted Landscape, diagnostics will now
  expose the resulting per-point surface delta; Range intentionally remains
  based on the loaded heightmap cache as required.

