# Test Plan

## Automation

Run editor automation tests matching:

```text
LandscapeHeightmapTracker.Mapper.*
LandscapeHeightmapTracker.ViewportTrace.*
LandscapeHeightmapTracker.ReverseMapping.*
LandscapeHeightmapTracker.HeightZone.*
```

The mapper tests cover center, corners, Flip X, Flip Y, transformed Landscapes, non-square bounds, outside rejection, and clamp mode.

The reverse mapping tests cover UV-to-local center, corners, Flip X inversion, Flip Y inversion, non-square bounds, invalid bounds, outside UV rejection, fitted-image click hit testing, and finite vertical trace segment construction.

The viewport trace tests cover:

- Perspective forward trace construction.
- Orthographic segment construction centered on the cursor origin.
- Reversed orthographic direction.
- Invalid zero-length direction rejection.

The height-zone tests cover:

- flat maps above and below the threshold;
- a single hill and depression;
- independent islands and an internal hole;
- a contour crossing the image boundary;
- deterministic saddle handling;
- a threshold exactly on a sample and negative heights;
- finite normalized contour coordinates;
- 8-bit expansion, canonical UE Landscape height decoding, actor Z scale and
  translation, and cache construction.

## Scenario 21 - Height Zone

1. Assign a Landscape and load the same source heightmap used to create it.
2. Enter a height within the displayed Landscape range.
3. Apply `Above`, then `Below`, then `Contour Only`.
4. Verify all islands and holes, fitted-image alignment after panel resize, and
   that hover/click markers remain above the zone.
5. Change Flip X and Flip Y and apply again.
6. Move or Z-scale the Landscape and apply again; verify the displayed range and
   zone update.
7. Click `Clear`; verify only the zone disappears and the entered height remains.

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

## Scenario 11 - Reverse Mapping Basic Center

1. Assign a known 1009x1009 Landscape.
2. Load the source PNG.
3. Click the center of the 2D image.
4. Verify a vertical editor-only line appears near Landscape center.
5. Verify diagnostics show Display UV, Landscape UV, Local XY, and World XYZ.

## Scenario 12 - Recognisable Landmarks

Click these 2D locations and verify the vertical line appears at matching 3D locations:

- Mountain peak.
- Valley.
- Ridge.
- Depression.
- Corner regions.

## Scenario 13 - Reverse Mapping Moved Landscape

1. Move the Landscape Actor.
2. Repeat center and landmark clicks.
3. Verify the vertical line follows the moved Landscape.

## Scenario 14 - Reverse Mapping Non-default XY Scale

1. Use or create a Landscape with non-default XY scale.
2. Repeat center and landmark clicks.
3. Verify Local XY to World XY mapping remains correct.

## Scenario 15 - Sculpted Landscape Surface

1. Import PNG.
2. Sculpt one area up or down.
3. Click the corresponding 2D XY.
4. Verify the vertical line base uses the current sculpted Landscape surface Z, not the PNG value.

## Scenario 16 - Blocking Mesh

1. Place a Static Mesh above the Landscape.
2. Click the underlying 2D location.
3. Verify the multi-trace still finds the assigned Landscape behind the blocker.

## Scenario 17 - Letterbox Input

1. Resize the tracker tab to a wide aspect ratio.
2. Click image center and verify correct mapping.
3. Click empty letterbox or pillarbox area.
4. Verify the click is rejected and the 3D marker does not move.

## Scenario 18 - Clear Marker

1. Create a 2D marker with a 3D Landscape click.
2. Create a 3D marker with a 2D heightmap click.
3. Click `Clear All Markers`.
4. Verify both the 2D crosshair and 3D vertical line disappear immediately.

## Scenario 19 - Map Change / Landscape Deletion

1. Create a reverse marker.
2. Switch map or delete the assigned Landscape.
3. Verify no crash and invalid marker state is not drawn.

## Scenario 20 - Regression

Re-test existing 3D-to-2D flows:

- Perspective center, corners, peak, and valley.
- Top view center, corners, peak, and valley.
- Flip X and Flip Y.
