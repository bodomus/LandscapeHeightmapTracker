# Test Plan

## Automation

Run editor automation tests matching:

```text
LandscapeHeightmapTracker.Mapper.*
LandscapeHeightmapTracker.ViewportTrace.*
```

The mapper tests cover center, corners, Flip X, Flip Y, transformed Landscapes, non-square bounds, outside rejection, and clamp mode.

The viewport trace tests cover:

- Perspective forward trace construction.
- Orthographic segment construction centered on the cursor origin.
- Reversed orthographic direction.
- Invalid zero-length direction rejection.

## Scenario 1 - Basic 1009 landscape

1. Create/open UE 5.7 C++ test project.
2. Install plugin in `Project/Plugins/LandscapeHeightmapTracker`.
3. Compile Editor target.
4. Enable plugin.
5. Create/import a Landscape from a known 1009x1009 heightmap.
6. Open tracker tab.
7. Select Landscape.
8. Click `Use Selected Landscape`.
9. Load the same source heightmap.
10. Enable tracking.
11. Click near four corners and center.
12. Verify marker positions.

## Scenario 2 - Moved Landscape

1. Move Landscape actor in world space.
2. Repeat corner and center clicks.
3. Verify mapping remains correct.

## Scenario 3 - Non-default XY scale

1. Use a Landscape with non-default XY scale.
2. Verify click-to-image mapping remains correct.

## Scenario 4 - Wrong actor hit

1. Place Static Mesh above or near Landscape.
2. Click the mesh.
3. Verify tracker does not move marker as if the Landscape were clicked.

## Scenario 5 - Different image dimensions

Test at least:

- 505x505.
- 1009x1009.
- 2017x2017.

The plugin must not contain hardcoded 1009 assumptions.

## Scenario 6 - Panel resizing

1. Put tracker tab in a narrow dock.
2. Resize it.
3. Move it to a floating window.
4. Resize again.
5. Verify marker remains aligned.

## Scenario 7 - Flip controls

Verify Flip X and Flip Y visually.

## Scenario 8 - Map change

1. Assign Landscape.
2. Open another map.
3. Verify plugin handles invalid references safely.
4. No crash.
5. UI indicates that Landscape must be reassigned.

## Scenario 9 - Orthographic Top View

1. Assign the same Landscape and source heightmap used for Perspective validation.
2. Enable `Track Landscape Clicks`.
3. Switch the editor viewport to `Top`.
4. Click the center, four corner regions, a distinctive peak, and a distinctive valley.
5. Verify marker positions match the same locations on the 2D heightmap.
6. Switch back to Perspective and verify no regression.

## Scenario 10 - Other Orthographic Views

Where terrain visibility makes the result meaningful, repeat click validation from:

- Bottom.
- Front.
- Back.
- Left.
- Right.

The trace should be geometrically valid for each view, but side views can be visually ambiguous on steep or occluded terrain.
