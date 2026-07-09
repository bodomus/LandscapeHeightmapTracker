# UE5-10: Bidirectional Heightmap → Landscape Point Tracking

## Summary

Extend `LandscapeHeightmapTracker` with reverse point mapping:

```text
2D Heightmap Click
→ UV
→ Landscape Local XY
→ World XY
→ vertical trace against Assigned Landscape
→ current Landscape surface World Z
→ editor-only vertical line in 3D viewport
```

The existing direction must continue to work:

```text
3D Landscape Click
→ World Position
→ Landscape Local
→ UV / Pixel
→ 2D marker
```

The new 3D marker must be a lightweight editor-only vertical line. Do not spawn Actors, Components, Static Meshes, cubes, spheres, decals, Niagara systems, billboards, labels, or persistent scene objects.

The final World Z must come from the current Landscape surface via a vertical trace. Do not calculate Z from source PNG values.

---

## 1. Goal

After this ticket, the plugin supports both directions:

```text
3D → 2D
Landscape viewport click
→ map to UV/pixel
→ show marker on heightmap
```

```text
2D → 3D
Heightmap image click
→ map to Landscape XY
→ find current Landscape surface Z
→ show vertical editor-only line in viewport
```

The reverse operation must use the currently assigned Landscape and its current sculpted surface.

---

## 2. Hard Requirements

Implementation remains:

- Unreal Engine 5.7;
- native C++;
- Editor-only;
- Slate UI;
- no C#;
- no Python runtime dependency;
- no external executable;
- no IPC or sockets;
- no Blueprint-only implementation;
- no persistent scene objects.

The vertical marker must not dirty the level or create transaction/save changes.

---

## 3. Preserve Existing Architecture

Do not redesign working UE5-8 and UE5-9 functionality.

Existing responsibilities:

```text
FLandscapeHeightmapTrackerEdMode
    viewport click observation
    viewport interaction
    suitable place for editor viewport drawing

SLandscapeHeightmapTrackerPanel
    UI state
    Landscape assignment
    heightmap display
    diagnostics
    2D marker state

FLandscapeCoordinateMapper
    World → Local → UV → Pixel mapping

FViewportTraceRayBuilder
    Perspective / Orthographic trace construction
```

Extend this structure cleanly.

Do not move coordinate math into Slate event handlers or paint functions.

Do not duplicate Flip X / Flip Y logic in multiple places.

---

## 4. Mandatory Graphify Workflow

Graphify is part of the implementation workflow for this repository.

### 4.1 Before changes

From repository root:

```powershell
graphify update .
```

Then inspect the current graph before editing.

Required investigation targets:

```text
SHeightmapTrackerImageView
SLandscapeHeightmapTrackerPanel
FLandscapeCoordinateMapper
FLandscapeHeightmapTrackerEdMode
OnViewportClick
TraceLandscapeClick
FViewportTraceRayBuilder
```

Suggested commands/queries:

```text
graphify explain "SHeightmapTrackerImageView"
graphify explain "SLandscapeHeightmapTrackerPanel"
graphify explain "FLandscapeCoordinateMapper"
graphify explain "FLandscapeHeightmapTrackerEdMode"

graphify query "Where is the heightmap image draw rectangle calculated and how is MarkerUV rendered?"
graphify query "How does a viewport click flow from FLandscapeHeightmapTrackerEdMode to FLandscapeCoordinateMapper?"
graphify query "Where are Flip X and Flip Y applied?"
graphify query "How is AssignedLandscape validated and how are Landscape bounds calculated?"
```

Use Graphify to understand structure and communities, but do not treat a structural `path` result as execution-flow proof. If a path runs through file/namespace nodes, verify the runtime call chain by direct source inspection.

Record Graphify findings in the final review report.

### 4.2 After changes

Run:

```powershell
graphify update .
```

Verify that new reverse-mapping functions/classes are present and connected.

The final review report must include:

- graph node count;
- edge count;
- community count;
- queries used;
- new classes/functions discovered;
- brief explanation of the reverse-mapping path visible in the graph.

---

## 5. User Workflow

Required workflow:

```text
Open Landscape Heightmap Tracker
    ↓
Assign Landscape
    ↓
Load source heightmap PNG
    ↓
Click point on 2D heightmap
    ↓
Plugin converts click to Display UV
    ↓
Undo Flip X / Flip Y
    ↓
UV → Landscape Local XY
    ↓
Local XY → World XY
    ↓
Vertical trace through Assigned Landscape
    ↓
Find current surface World Z
    ↓
Store 3D marker WorldPosition
    ↓
Draw vertical editor-only line
    ↓
Update diagnostics/status
```

2D image clicking must work whenever:

- a valid Landscape is assigned;
- a valid heightmap image is loaded.

If prerequisites are missing, report an inline status message.

---

## 6. 2D Image Click Handling

The current image widget already computes a fitted image rectangle using:

- widget size;
- image aspect ratio;
- `DrawSize`;
- centered `DrawOffset`.

Use exactly the same draw rectangle for reverse hit testing.

Do not calculate UV from the raw widget rectangle.

Required logic:

```text
Mouse Local Position
→ verify point is inside actual image draw rect
→ subtract DrawOffset
→ divide by DrawSize
→ Display UV [0..1]
```

Conceptually:

```text
LocalImageX = MouseX - DrawOffset.X
LocalImageY = MouseY - DrawOffset.Y

U = LocalImageX / DrawSize.X
V = LocalImageY / DrawSize.Y
```

If the click is in letterbox/pillarbox empty space:

- reject it;
- do not move the 3D marker;
- keep existing marker unchanged;
- optionally show `Click is outside the heightmap image area.`

The image widget should emit a clean callback/delegate containing normalized UV.

Suggested shape:

```cpp
SLATE_EVENT(FOnHeightmapClicked, OnHeightmapClicked)
```

The image widget must not access Landscape state directly.

---

## 7. Reverse Coordinate Mapping

Extend `FLandscapeCoordinateMapper` with a pure reverse mapping operation.

Suggested responsibility:

```text
Input:
- Landscape local bounds
- displayed UV
- Flip X / Flip Y options

Output:
- Landscape UV after flip correction
- Landscape Local XY
- valid/invalid result
```

Suggested API:

```cpp
static FLandscapeTrackerReverseMappingResult MapUVToLocalPosition(
    const FLandscapeTrackerBounds& LocalBounds,
    const FVector2D& DisplayUV,
    const FLandscapeTrackerMappingOptions& Options);
```

Required order:

```text
Displayed UV
→ undo Flip X
→ undo Flip Y
→ map UV into Local XY bounds
```

Conceptually:

```text
if FlipX:
    U = 1 - U

if FlipY:
    V = 1 - V

LocalX = MinX + U * (MaxX - MinX)
LocalY = MinY + V * (MaxY - MinY)
```

Do not duplicate this inverse flip logic in panel/UI code.

Reject invalid bounds.

Reject UV outside `[0,1]` by default.

---

## 8. Landscape Local XY → World XY

Use the Assigned Landscape transform.

Pipeline:

```text
Landscape Local XY
→ Landscape Actor Transform
→ World XY seed
```

Do not assume:

- Landscape at world origin;
- XY Scale = 100;
- zero rotation;
- square Landscape;
- 1009×1009 dimensions.

Moved and scaled Landscapes must work.

---

## 9. Height Resolution — Required Variant A

Use a vertical world trace to find current Landscape surface Z.

Do not:

- read Z from PNG;
- decode source grayscale into height;
- directly sample Landscape height data in this ticket;
- reconstruct terrain height from source image.

Required concept:

```text
World XY
    ↓
Trace Start above terrain
    ↓
Vertical trace downward
    ↓
find hit belonging to Assigned Landscape
    ↓
Hit.ImpactPoint = final World XYZ
```

Use a finite trace distance.

Do not use `HALF_WORLD_MAX`.

### Required filtering behavior

Prefer:

```text
LineTraceMulti
→ inspect ordered hits
→ accept first hit belonging to Assigned Landscape
→ ignore unrelated blockers
```

This ticket should not repeat the current `LineTraceSingleByChannel()` limitation where an unrelated first blocking hit prevents reaching the Landscape.

Conceptual implementation:

```text
Start = WorldXY + FVector(0, 0, +TraceHalfHeight)
End   = WorldXY + FVector(0, 0, -TraceHalfHeight)

LineTraceMultiByChannel(...)
    ↓
for each hit:
    if hit belongs to Assigned Landscape:
        accept
        stop
```

The final World Z must be taken from the accepted `Hit.ImpactPoint`.

---

## 10. Shared Assigned Landscape Hit Validation

Reuse or centralize Assigned Landscape hit validation.

A hit is valid only for the currently assigned Landscape.

Avoid maintaining separate, slightly different hit-validation rules for:

```text
3D → 2D viewport click
```

and:

```text
2D → 3D vertical trace
```

If useful, extract a shared helper.

Do not broaden acceptance to arbitrary Landscapes.

---

## 11. 3D Marker Visualization

Display the reverse-mapped point as one vertical editor-only line.

Required visual concept:

```text
          |
          |
          |
          |
          X  Landscape surface point
```

Required behavior:

```text
MarkerBase = SurfacePoint
MarkerTop  = SurfacePoint + FVector(0, 0, MarkerHeight)
```

Implementation should use Editor viewport drawing through the existing Editor Mode or another native editor-only draw path.

Requirements:

- no Actor;
- no Component;
- no Static Mesh;
- no sphere/cube;
- no persistent UObject marker;
- no level dirty state;
- no save changes;
- no transaction entry.

The marker must appear immediately after a successful 2D click.

The marker must disappear immediately after clear.

---

## 12. Viewport Redraw

After successful reverse mapping:

1. store marker WorldPosition;
2. request/redraw the editor viewport;
3. vertical line appears immediately.

After clear:

1. clear marker state;
2. request redraw;
3. vertical line disappears immediately.

Avoid permanent Tick work if an event-driven redraw is sufficient.

---

## 13. Marker State

Keep 2D and 3D markers logically separate.

For MVP:

```text
3D click:
    updates 2D marker

2D click:
    updates 3D vertical line
```

Both may exist simultaneously.

Suggested controls:

```text
[Clear 2D Marker]
[Clear 3D Marker]
```

or a single:

```text
[Clear All Markers]
```

A single clear action is acceptable if behavior is explicit and documented.

Do not remove existing clear behavior without preserving equivalent functionality.

---

## 14. UI and Status

After successful reverse mapping:

```text
Heightmap click mapped to Landscape surface.
```

Required failure states:

```text
No Landscape assigned.
No heightmap loaded.
Click is outside the heightmap image area.
Could not resolve Landscape surface at selected XY.
No hit on Assigned Landscape.
```

Routine failures must use inline status, not modal dialogs.

---

## 15. Diagnostics

After successful 2D → 3D mapping show or preserve:

```text
Display UV
Landscape UV after flip correction
Landscape Local X/Y
World X/Y/Z
```

The final World Z must be the current surface hit result.

---

## 16. EdMode Responsibility

Extend `FLandscapeHeightmapTrackerEdMode` carefully.

A clean design is:

```text
Panel:
    receives image click
    Display UV → reverse mapper
    Local XY → World XY
    vertical Assigned Landscape trace
    updates diagnostics/status
    sends final marker WorldPosition

EdMode:
    stores lightweight marker state
    draws vertical line
    clears marker
```

Alternative clean ownership is acceptable if justified after Graphify/source inspection.

Do not let the image widget call EdMode APIs directly.

Do not create unmanaged global mutable state.

Document communication and lifecycle in `Docs/ARCHITECTURE.md`.

Important UX rule:

The 3D marker should remain visible after a 2D click even when `Track Landscape Clicks` is disabled, unless Unreal API constraints make that impossible.

Prefer decoupling:

```text
3D click tracking toggle
```

from:

```text
3D marker rendering
```

---

## 17. Lifecycle Safety

New functionality must be safe across:

- tab close;
- panel destruction;
- plugin shutdown;
- map change;
- Assigned Landscape deletion;
- tracking mode disable;
- Editor Mode deactivation/reactivation.

Requirements:

- no stale raw UObject pointer;
- no dangling delegate;
- no leftover draw callback after shutdown;
- marker clears when Assigned Landscape becomes invalid;
- no crash on map switch.

---

## 18. Required Tests

### 18.1 Reverse mapper tests

Add automation coverage.

#### A. Center

```text
Bounds Min=(0,0)
Bounds Max=(1000,1000)
Display UV=(0.5,0.5)
Expected Local=(500,500)
```

#### B. Corners

Verify:

```text
(0,0)
(1,0)
(0,1)
(1,1)
```

map to all four local bounds corners.

#### C. Flip X

Displayed:

```text
U=0.25
```

with Flip X enabled must map to Landscape U:

```text
0.75
```

#### D. Flip Y

Displayed:

```text
V=0.25
```

with Flip Y enabled must map to Landscape V:

```text
0.75
```

#### E. Non-square bounds

```text
X=0..2000
Y=0..1000
UV=(0.25,0.75)
```

Expected:

```text
LocalX=500
LocalY=750
```

#### F. Invalid bounds

Reject zero-width or zero-height bounds.

#### G. UV outside range

Reject by default.

---

## 19. Fitted Image Click Tests

Where practical, extract fitted-image click math into testable pure code.

Example:

```text
Widget = 1000×500
Image  = 1000×1000
```

Expected fitted image:

```text
DrawSize   = 500×500
DrawOffset = (250,0)
```

Verify:

```text
Mouse (250,0)   → UV (0,0)
Mouse (500,250) → UV (0.5,0.5)
Mouse (750,500) → UV (1,1)
Mouse (100,250) → outside
Mouse (900,250) → outside
```

---

## 20. Surface Trace Helper Tests

At minimum test pure/helper logic for:

- finite vertical segment construction;
- positive trace distance validation;
- start Z above end Z;
- X/Y unchanged through vertical segment creation.

If full editor-world collision integration tests are not practical, document that boundary honestly.

---

## 21. Manual Test Plan

Update `Docs/TEST_PLAN.md`.

### Scenario 11 — Basic center

1. Assign known 1009×1009 Landscape.
2. Load source PNG.
3. Click center of 2D image.
4. Verify vertical line appears near Landscape center.
5. Verify diagnostics.

### Scenario 12 — Recognisable landmarks

Click on 2D:

- mountain peak;
- valley;
- ridge;
- depression;
- corner regions.

Verify line appears at matching 3D locations.

### Scenario 13 — Moved Landscape

Move Landscape Actor and repeat.

### Scenario 14 — Non-default XY Scale

Change/use non-default XY scale and repeat.

### Scenario 15 — Sculpted Landscape

1. Import PNG.
2. Sculpt one area up/down.
3. Click corresponding 2D XY.
4. Verify line base uses current sculpted Landscape surface Z.

This scenario is mandatory.

### Scenario 16 — Blocking mesh

1. Place Static Mesh above Landscape.
2. Click corresponding 2D location.
3. Verify multi-trace still finds Assigned Landscape behind blocker.

### Scenario 17 — Letterbox input

1. Resize tab to wide aspect.
2. Click image center: correct mapping.
3. Click empty letterbox area: rejected, marker unchanged.

### Scenario 18 — Clear marker

Create marker, clear it, verify immediate disappearance.

### Scenario 19 — Map change / Landscape deletion

Create marker, switch map or delete Landscape, verify no crash and invalid marker state is cleared.

### Scenario 20 — Regression

Re-test existing 3D → 2D:

- Perspective center/corners/peak/valley;
- Top center/corners/peak/valley.

---

## 22. Logging

Use existing plugin logging conventions.

Suggested `Verbose` details:

```text
Image click local position
Image draw rect
Display UV
Landscape UV after flip correction
Local XY
World XY
Trace Start / End
Trace hit count
Accepted Landscape hit
Final World XYZ
```

Do not log Slate paint calls.

Do not log per frame.

Do not add hover tracking.

---

## 23. Documentation

Update:

```text
README.md
CHANGELOG.md
Docs/ARCHITECTURE.md
Docs/TEST_PLAN.md
```

README must explain:

```text
3D Landscape click → 2D marker
2D heightmap click → 3D vertical line
```

Architecture must document:

```text
2D fitted image rect
→ Display UV
→ undo flips
→ Local XY
→ World XY
→ vertical LineTraceMulti
→ Assigned Landscape hit
→ World XYZ
→ editor viewport vertical line
```

Explicitly state that Z comes from current Landscape surface collision/hit result, not from PNG grayscale.

---

## 24. Acceptance Criteria

### Reverse mapping

- [ ] 2D heightmap image is clickable.
- [ ] Click uses fitted image draw rect.
- [ ] Letterbox/pillarbox clicks are rejected.
- [ ] Click produces normalized Display UV.
- [ ] Flip X inverse works.
- [ ] Flip Y inverse works.
- [ ] UV maps to correct Landscape Local XY.
- [ ] Local XY maps through Landscape transform.
- [ ] Moved Landscape works.
- [ ] Non-default XY scale works.
- [ ] Non-square bounds work.

### Height resolution

- [ ] World Z comes from current Assigned Landscape surface.
- [ ] PNG is not used to calculate Z.
- [ ] Vertical trace is finite.
- [ ] `HALF_WORLD_MAX` is not used.
- [ ] Trace finds Assigned Landscape specifically.
- [ ] Unrelated blocking mesh does not prevent finding Landscape behind it.
- [ ] No valid Landscape hit produces clear inline failure state.

### 3D visualization

- [ ] Valid 2D click creates visible vertical line.
- [ ] No Actor is spawned.
- [ ] No Component is added.
- [ ] No level object is created.
- [ ] Level is not dirtied.
- [ ] Marker appears immediately.
- [ ] Marker clears immediately.
- [ ] Marker state is safe across map change and Landscape deletion.

### Regression

- [ ] Perspective 3D → 2D still works.
- [ ] Top View 3D → 2D still works.
- [ ] Flip X / Flip Y still work.
- [ ] Existing mapper tests pass.
- [ ] Existing viewport trace tests pass.
- [ ] Plugin builds as UE 5.7 Editor plugin.

### Graphify

- [ ] `graphify update .` run before implementation.
- [ ] Graphify queries used to inspect existing architecture.
- [ ] Runtime call chain verified by source inspection.
- [ ] `graphify update .` run after implementation.
- [ ] New reverse-mapping classes/functions appear in graph.
- [ ] Review report includes updated graph counts and findings.

---

## 25. Out of Scope

Do not implement:

- camera teleport;
- camera focus;
- viewport animation;
- double-click focus;
- hover tracking;
- continuous mouse tracking;
- Actor markers;
- spheres/cubes;
- marker labels;
- multiple markers;
- marker history;
- bookmarks;
- Landscape editing;
- heightmap editing;
- PNG pixel value → Z conversion;
- direct Landscape height-data sampling;
- Gaea integration;
- QGIS integration;
- World Partition overview;
- multi-Landscape mode.

Keep scope focused on:

```text
2D click
→ Landscape XY
→ current surface Z
→ vertical viewport line
```

---

## 26. Suggested Decomposition

Codex must inspect Graphify/source first and adjust names based on current repository.

Reasonable direction:

```text
FLandscapeCoordinateMapper
    + reverse UV → Local XY mapping

SHeightmapTrackerImageView
    + click handling
    + fitted-image hit testing
    + OnHeightmapClicked(UV)

SLandscapeHeightmapTrackerPanel
    + receive image UV
    + call reverse mapper
    + Local XY → World XY
    + request current surface trace
    + update diagnostics/status
    + pass marker WorldPosition to viewport visualization owner

New/extracted helper:
FLandscapeSurfaceTraceHelper
    + finite vertical trace
    + LineTraceMulti
    + Assigned Landscape filtering

FLandscapeHeightmapTrackerEdMode
    + store marker position
    + draw editor-only vertical line
    + clear marker
```

Do not follow this blindly if Graphify/source inspection shows cleaner existing ownership.

---

## 27. Build and Test

Build using existing UE 5.7 workflow, for example:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat' `
  BuildPlugin `
  -Plugin='J:\Projects\UE_Projects\LandscapeHeightmapTracker\LandscapeHeightmapTracker.uplugin' `
  -Package='J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\Package-UE5-10' `
  -TargetPlatforms=Win64 `
  -StrictIncludes
```

Run all plugin tests, not only new tests.

Expected families:

```text
LandscapeHeightmapTracker.Mapper.*
LandscapeHeightmapTracker.ViewportTrace.*
LandscapeHeightmapTracker.ReverseMapping.*
```

Use final names appropriate to implementation.

If Live Coding or stale binaries interfere:

- stop Live Coding and rebuild correctly; or
- use a clean ignored test host with packaged plugin binaries;
- document exactly what was done.

Do not claim success unless intended new tests were discovered and executed.

---

## 28. Review Report

Create:

```text
Reviews/review-UE5-10.md
```

Include:

1. Summary.
2. Graphify investigation before changes.
3. Existing runtime call chain.
4. Files changed.
5. Reverse mapping architecture.
6. Image click → UV strategy.
7. Flip inversion strategy.
8. Local XY → World XY strategy.
9. Vertical Assigned Landscape trace strategy.
10. Why PNG is not used for Z.
11. Vertical line rendering strategy.
12. Lifecycle handling.
13. Tests added.
14. Build command/result.
15. Automation command/result.
16. Exact discovered tests.
17. Manual validation performed or not performed.
18. Graphify update after changes.
19. Updated graph counts.
20. Known limitations.
21. Recommended next ticket.

Be explicit about anything not manually validated.

---

## 29. Definition of Done

The ticket is done when this exact scenario works:

```text
1. Start UE 5.7 Editor.
2. Open Landscape Heightmap Tracker.
3. Assign Landscape.
4. Load its source PNG.
5. Click center of 2D image.
6. Vertical editor-only line appears at corresponding 3D location.
7. Click known mountain peak on 2D image.
8. Line moves to corresponding 3D peak.
9. Click known valley.
10. Line moves to corresponding 3D valley.
11. Sculpt one location after import.
12. Click same XY on 2D image.
13. Line base uses current sculpted Landscape surface Z.
14. Place Static Mesh above Landscape.
15. Click underlying 2D location.
16. Multi-trace still finds Assigned Landscape.
17. Resize plugin tab.
18. 2D click mapping remains correct.
19. Click outside fitted image rectangle.
20. Marker does not move.
21. Clear marker.
22. Vertical line disappears immediately.
23. Existing Perspective 3D → 2D still works.
24. Existing Top View 3D → 2D still works.
25. All automation tests pass.
26. Graphify is updated and reverse mapping appears in the graph.
```

Any implementation that spawns scene Actors or calculates World Z from the source PNG does not satisfy this ticket.
