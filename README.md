# LandscapeHeightmapTracker

LandscapeHeightmapTracker is an Unreal Engine 5.7 Editor plugin that maps positions between a 3D Landscape and its source 2D heightmap.

## Installation

Place this repository in a project plugin directory:

```text
<MyProject>/
└── Plugins/
    └── LandscapeHeightmapTracker/
        ├── LandscapeHeightmapTracker.uplugin
        └── Source/
```

Then:

1. Close Unreal Editor.
2. Place the plugin in the project `Plugins` directory.
3. Regenerate project files if required.
4. Build `Development Editor | Win64`.
5. Start the project.
6. Enable the plugin if it is not already enabled.
7. Open `Tools -> Landscape Heightmap Tracker`.

## Basic Usage

```text
Select Landscape
-> Open Tracker
-> Use Selected Landscape
-> Load Heightmap
-> Enable Track Landscape Clicks
-> Click Landscape
-> Inspect marker and coordinates
```

The reverse workflow is also supported:

```text
Select Landscape
-> Open Tracker
-> Use Selected Landscape
-> Load Heightmap
-> Click the 2D heightmap image
-> Inspect the vertical editor-only line in the 3D viewport
```

The normal workflow does not require manually entering Landscape dimensions. The plugin reads Landscape-local extents from the selected `ALandscapeProxy`.

## Mapping Convention

The mapper converts:

```text
World hit position
-> inverse Landscape actor transform
-> Landscape local XY
-> normalized U/V from Landscape local bounds
-> optional Flip X / Flip Y
-> round(U * (Width - 1)), round(V * (Height - 1))
```

`Flip Y` defaults to enabled because PNG display coordinates are top-left oriented while Landscape local Y commonly reads as bottom-to-top for source heightmaps. Use the visible Flip controls if the imported terrain orientation differs.

For 2D heightmap clicks, the plugin uses the fitted image rectangle that Slate actually draws, rejects clicks in letterbox or pillarbox space, undoes Flip X / Flip Y, maps UV into Landscape-local XY bounds, transforms that point through the assigned Landscape actor transform, then runs a finite vertical `LineTraceMultiByChannel`.

The 3D marker's World Z comes from the current assigned Landscape surface hit. PNG grayscale values are not used to calculate height, so sculpted Landscape changes are reflected when collision is up to date.

## Limitations

- External PNG files only.
- Image is displayed for coordinate tracking; height values are not edited.
- RGB/RGBA PNGs display normally, but grayscale value diagnostics are not part of the MVP.
- Click tracking only; no live hover tracking.
- No Landscape sculpt brush synchronization.
- No multi-Landscape simultaneous tracking.
- No image-to-Landscape camera focus yet.

## Future Roadmap

- Click 2D image to focus the editor camera on the corresponding Landscape position.
- Hover tracking and zoom/pan in the image viewer.
- Height value display and source-vs-Landscape comparison.
- Tiled Landscape and World Partition overview support.
- Multiple synchronized overlay layers.
