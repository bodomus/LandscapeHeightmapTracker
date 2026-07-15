# UE5-11 Hover Clear Fix Report

## Summary

Implemented Option A: the panel now records whether continuous hover has supplied a viewport state. A known invalid hover state hides the forward marker instead of revealing the stored click marker.

## Root cause

The image widget previously rendered hover when `bHasHoverMarker` was true and otherwise unconditionally fell back to `bHasMarker`. `ClearHoverMarker()` therefore made an old stored click visible after any hover miss or viewport leave.

## Files changed

- `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`
- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`

## State model before/after

Before:

```text
valid hover -> hover marker
no valid hover + stored click -> click marker
```

After:

```text
known hover viewport state + valid hover -> hover marker
known hover viewport state + invalid hover -> no forward marker
no governing hover viewport state + stored click -> click marker
valid explicit click -> update stored click and make it the forward display state
```

The persistent click coordinates remain stored. Disabling tracking and full marker/Landscape/texture resets clear the hover viewport state. Reverse marker APIs and state were not modified.

## Exact rendering rule

The click-marker Slate attribute is true only when:

```cpp
bHasMarker && !bHoverTrackingHasViewportState
```

The hover marker remains controlled independently by `bHasHoverMarker`.

## Repository intelligence

- Workflow level: 2.
- Graphify 0.9.8: existing graph reused; focused hover-clear query executed.
- CRG 2.3.6: updated before and after implementation. Post-change database contains 146 nodes and 1004 edges.
- Source validation: panel Slate attributes, `OnViewportHover`, `OnViewportClick`, reset paths, EdMode `MouseMove`/`MouseLeave`/`Tick`, and reverse marker ownership inspected directly.
- Graph/source discrepancy: none affecting the implementation. Graph relations were not used as runtime proof.

## Build result

Command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' UE57EditorEditor Win64 Development -Project='J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -WaitMutex -NoHotReload
```

Result: `Succeeded` (exit code 0). Existing Unreal deprecation warnings and the non-preferred Visual Studio 2026 compiler warning were emitted; no compilation or link errors occurred.

## Automation test result

Command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoSplash -NullRHI -NoSound -ExecCmds='Automation RunTests LandscapeHeightmapTracker; Quit' -TestExit='Automation Test Queue Empty'
```

Result: exit code 0; 9/9 selected tests completed with `Result={Success}`:

- Mapper.Basic
- Mapper.OptionsAndTransforms
- ReverseMapping.FittedImageClick
- ReverseMapping.UVToLocal
- ReverseMapping.VerticalTraceSegment
- ViewportTrace.InvalidDirection
- ViewportTrace.Orthographic
- ViewportTrace.Perspective
- ViewportTrace.ReversedOrthographic

## Manual verification result

`NOT VERIFIED`: no interactive Unreal Editor session was controlled during this task.

Pending manual checks:

- no-prior-click hover leave;
- prior-click hover leave without fallback;
- explicit click after hover clear;
- tracking disable;
- reverse heightmap click;
- panel close/reopen;
- multiple viewport behavior.

## Known limitations

The existing automation suite does not directly instantiate the private Slate panel state machine, so the exact visual transition is compile-verified and source-reviewed but still requires the manual UX checklist.

## git diff --stat

```text
Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp | 32 ++++++++++++++++++++--
Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h    |  1 +
2 files changed, 31 insertions(+), 2 deletions(-)
```

