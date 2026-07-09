# Architecture

## Module

`LandscapeHeightmapTracker` is an Editor-only module. `StartupModule()` registers style resources, UI commands, the `Tools` menu entry, a Nomad tab spawner, and a hidden editor mode used only for viewport click observation. `ShutdownModule()` disables tracking, clears delegates, unregisters menus, unregisters the tab spawner, unregisters the editor mode, and releases style resources.

## Viewport Click Integration

The plugin uses a scoped `FEdMode` implementation: `FLandscapeHeightmapTrackerEdMode`.

The Slate panel enables the mode when `Track Landscape Clicks` is checked and disables it when unchecked or when the panel is destroyed. The module also disables the mode during shutdown. The editor mode handles left mouse button press, creates a projection-aware trace segment from the active editor viewport cursor location, runs a visibility line trace in the editor world, and broadcasts the hit result through a module multicast delegate.

Perspective viewports trace forward from the cursor origin along the cursor direction. Orthographic viewports trace a finite segment centered on the cursor origin and spanning both directions along the normalized cursor direction, matching the parallel-ray nature of Top, Bottom, Front, Back, Left, and Right editor views. This keeps projection handling in the viewport integration layer and avoids projection-specific corrections in coordinate mapping.

Trace construction lives in `FViewportTraceRayBuilder` so the perspective and orthographic segment rules can be automation-tested without depending on editor-world collision.

The mode returns `false` from `InputKey` after observing the click so normal editor selection/navigation behavior can continue wherever Unreal permits it.

This approach is native, editor-scoped, reversible, and avoids global OS mouse hooks, polling, sockets, or external processes.

## Landscape Validation

The panel stores the selected Landscape as `TWeakObjectPtr<ALandscapeProxy>`. A viewport hit is accepted only when the hit actor is the assigned Landscape or the hit component is owned by the assigned Landscape. Static Meshes, other Landscapes, sky, foliage, and unrelated actors are rejected.

If the weak Landscape reference becomes invalid after deletion or map change, the UI reports that no Landscape is assigned and no mapping is attempted.

## Bounds

The MVP uses `ALandscapeProxy::GetBoundingRect()` to get deterministic Landscape-local XY extents. These values are topology-based Landscape coordinates before actor transform. World positions are converted into that same local coordinate domain through the inverse Landscape actor transform.

## Coordinate Mapping

`FLandscapeCoordinateMapper` is independent of Slate UI. It takes:

- Landscape transform.
- Landscape local XY bounds.
- World hit position.
- Image dimensions.
- Flip/clamp options.

It outputs:

- World position.
- Local position.
- Normalized UV.
- Pixel coordinate.
- inside/outside status and failure reason.

Pixel indexing uses:

```text
PixelX = round(U * (Width - 1))
PixelY = round(V * (Height - 1))
```

By default, outside positions are rejected rather than clamped. A clamp option exists for tests and future tooling, but the UI rejects outside-bounds clicks.

## Image Display

The PNG file is decoded with Unreal's `ImageWrapper` module and copied into a transient `UTexture2D` for Slate display. The image is not imported into the Content Browser.

The custom Slate image widget preserves aspect ratio and computes a centered fitted draw rectangle. Marker placement uses normalized UV inside that draw rectangle, so it remains aligned when the panel is resized or letterboxed.

## Dependencies

The module uses:

- `Landscape` for `ALandscapeProxy`.
- `UnrealEd`, `EditorFramework`, and `LevelEditor` for editor mode and menu integration.
- `Slate` and `SlateCore` for UI.
- `ImageWrapper` and `DesktopPlatform` for external PNG loading.
- `PropertyEditor` for the object picker widget.
