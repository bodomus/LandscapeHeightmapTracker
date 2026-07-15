# Ticket: Fix hover clear behavior outside assigned Landscape

## Project

Unreal Engine C++ Editor Plugin:

`LandscapeHeightmapTracker`

## Context

Continuous Landscape hover tracking has been implemented and manually tested.

Current behavior is mostly correct:

- moving the mouse over the assigned Landscape shows the live marker on the heightmap;
- clicking the Landscape stores a click marker on the heightmap;
- reverse Heightmap → Landscape flow still works.

However, one UX issue remains.

## Problem

When the user clicks a point on the Landscape, the click marker is stored and shown on the heightmap.

Then, if the user moves the mouse outside the assigned Landscape without clicking again, the live hover marker disappears, but the heightmap marker immediately falls back to the last clicked marker.

Observed behavior:

```text
Hover over Landscape
    ↓
click point A
    ↓
marker shows point A
    ↓
move mouse outside Landscape
    ↓
marker jumps back to point A
```

This is not desired for continuous hover mode.

Expected behavior:

```text
Hover over Landscape
    ↓
click point A
    ↓
marker shows point A
    ↓
move mouse outside Landscape
    ↓
heightmap marker should disappear
```

If the user has never clicked and simply moves the cursor outside the Landscape, the marker already disappears correctly. Keep that behavior.

## Desired UX

For the current continuous hover feature:

- while the mouse is over the assigned Landscape, show the hover marker;
- when the mouse leaves the assigned Landscape area, hide the visible forward marker;
- do not automatically fall back to the last click marker during hover tracking;
- the last click marker may remain stored internally if useful, but it must not become visible just because hover cleared after leaving the Landscape;
- clicking the Landscape should still update the stored click marker;
- reverse marker behavior must remain independent.

In other words:

```text
Hover marker valid:
    display hover marker

Hover marker invalid because cursor is outside Landscape:
    display no forward marker

Explicit click:
    update click marker

But:
    do not display click marker as automatic fallback after hover clears
```

## Important distinction

This is not asking to remove click behavior.

Click flow should still work:

```text
Left Mouse Press
    ↓
TraceLandscapeClick
    ↓
OnViewportClickResult
    ↓
OnViewportClick
    ↓
persistent click marker state updated
```

The requested change is only about display precedence / fallback behavior after hover clear.

## Required investigation before editing

Before changing code, inspect the current implementation and the prior analysis artifacts.

Read:

```text
AGENTS.md
evaluation/hover-tracking/investigation.md
evaluation/hover-tracking/reviewer-validation.md
evaluation/hover-tracking/revised-implementation-plan.md
evaluation/hover-tracking/implementation-report.md
evaluation/hover-tracking/code-review.md
evaluation/hover-tracking/code-review-verdict.md
```

Also use the existing repository analysis databases according to `AGENTS.md`:

- `graphify-repository-analysis` for architecture orientation and related symbols;
- `code-review-graph-analysis` / CRG for exact symbol lookup and local context;
- `rg` and direct source reading as the source of truth.

Do not treat Graphify or CRG graph relationships as runtime-flow proof.

## Likely area to inspect

Focus on marker display state and Slate attributes in:

```text
Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h
Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
```

Likely relevant symbols:

```text
bHasMarker
MarkerUV
bHasHoverMarker
HoverMarkerUV
ClearHoverMarker
SetHoverMarkerUV
InvalidateHeightmapMarkerPaint
SHeightmapTrackerImageView
HasMarker attribute
MarkerUV attribute
HasHoverMarker attribute
HoverMarkerUV attribute
OnViewportHover
OnViewportClick
ClearMarker
```

Also inspect EdMode hover clear sources:

```text
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
MouseMove
MouseLeave
Tick
```

## Required design decision

The current implementation apparently uses a display rule similar to:

```text
if hover marker exists:
    display hover marker
else if click marker exists:
    display click marker
else:
    display nothing
```

This fallback is the source of the unwanted jump.

Replace it with behavior appropriate for continuous hover mode.

A likely better display model:

```text
if tracking/hover mode is active and the last hover state is known:
    if hover marker is valid:
        display hover marker
    else:
        display no marker
else:
    display click marker if one exists
```

However, do not implement this blindly.

First inspect actual code and choose the smallest safe change.

Possible implementation strategies:

### Option A — introduce explicit hover visibility state

Add a state such as:

```cpp
bool bHoverTrackingHasViewportState = false;
```

or a better-named equivalent.

Meaning:

- true after the panel receives hover information from the viewport;
- true even if the latest hover result is a miss/non-assigned hit/mapping failure;
- false before any hover tracking activity, or after tracking is disabled / panel reset.

Rendering rule:

```text
if bHoverTrackingHasViewportState:
    display hover marker only if bHasHoverMarker
    otherwise display no marker
else:
    display click marker if bHasMarker
```

### Option B — clear visible click marker when hover leaves Landscape

This is simpler but may destroy useful persistent click state. Prefer this only if the current codebase makes explicit display state unnecessarily complicated.

### Option C — split stored click marker from displayed click marker

Only use this if the current state model already suggests it. Avoid overengineering.

## Preferred approach

Prefer Option A if it can be implemented cleanly.

The important semantic is:

```text
Hover clear caused by continuous tracking should not reveal the old click marker.
```

## Required behavior cases

Implement and verify these cases:

### Case 1 — no prior click

```text
move mouse over assigned Landscape
    -> marker appears

move mouse outside assigned Landscape
    -> marker disappears
```

### Case 2 — prior click exists

```text
click point A on assigned Landscape
    -> marker appears at A

move mouse over point B on assigned Landscape
    -> marker follows hover at B

move mouse outside assigned Landscape
    -> marker disappears

expected:
    marker must NOT jump back to A
```

### Case 3 — click after hover clear

```text
move mouse outside Landscape
    -> no marker

click point C on assigned Landscape
    -> marker appears at C
```

### Case 4 — tracking disabled

```text
tracking disabled
    -> hover marker/state clears
```

Confirm whether the existing click marker should appear or remain hidden according to current product behavior. Do not change more than required.

### Case 5 — heightmap reverse click

```text
click on heightmap
    -> reverse marker still works
    -> no hover/click fallback regression
```

### Case 6 — assigned Landscape change / heightmap reload

Existing clear behavior must remain safe.

## Non-goals

Do not:

- redesign the whole marker system;
- change coordinate mapping;
- change trace logic;
- change reverse marker behavior;
- change EdMode lifecycle unless required;
- add timers;
- add new Graphify/CRG infrastructure;
- add broad lifecycle hooks;
- modify unrelated files;
- commit changes.

## Expected production diff

Most likely limited to:

```text
Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h
Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp
```

Possibly:

```text
Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.cpp
```

only if the current hover clear event does not provide enough information to distinguish hover-active miss from no-hover-state.

Do not touch:

```text
LandscapeCoordinateMapper.*
ViewportTraceRayBuilder.*
LandscapeSurfaceTraceHelper.*
HeightmapImageClickMapper.*
LandscapeHeightmapTrackerModule.*
LandscapeHeightmapTracker.Build.cs
```

unless direct source inspection proves it is necessary.

## Build and test requirements

After implementation, run the normal build:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat' UE57EditorEditor Win64 Development -Project='J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -WaitMutex -NoHotReload
```

Run existing plugin automation tests:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\UE57Editor\UE57Editor.uproject' -Unattended -NoSplash -NullRHI -NoSound -ExecCmds='Automation RunTests LandscapeHeightmapTracker; Quit' -TestExit='Automation Test Queue Empty'
```

Report exact results.

## Manual verification checklist

If interactive Unreal Editor validation is possible, verify:

```text
[ ] no prior click: hover outside Landscape hides marker
[ ] prior click: hover outside Landscape does not jump back to clicked marker
[ ] click still updates stored marker
[ ] hover over Landscape still moves marker
[ ] reverse heightmap click still works
[ ] tracking disable does not leave stale hover marker
[ ] panel close/reopen has no stale callbacks
[ ] multiple viewport behavior remains acceptable
```

If manual validation is not possible, mark these as `NOT VERIFIED`.

## Required report

Create:

```text
evaluation/hover-tracking/fix-hover-clear-report.md
```

Include:

```text
Summary
Root cause
Files changed
State model before/after
Exact rendering rule after fix
Build result
Automation test result
Manual verification result
Known limitations
git diff --stat
```

## Final response

Report concisely:

1. what changed;
2. root cause;
3. whether click marker state is still stored;
4. whether old click marker still appears after hover leaves Landscape;
5. whether reverse flow was preserved;
6. build result;
7. automation test result;
8. manual verification result;
9. changed files;
10. report path;
11. confirmation that no commit was created.
