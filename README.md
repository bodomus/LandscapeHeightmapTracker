# LandscapeHeightmapTracker

LandscapeHeightmapTracker is an Unreal Engine 5.7 Editor plugin that maps clicked positions on a 3D Landscape to corresponding coordinates on the source 2D heightmap.

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
