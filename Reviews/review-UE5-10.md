# UE5-10 Review

## 1. Summary

Implemented bidirectional heightmap tracking:

- 3D Landscape click still maps to the 2D heightmap marker.
- 2D heightmap click now maps to Landscape local XY, resolves current surface World Z via vertical multi-trace, and draws an editor-only vertical line in the viewport.
- No Actors, Components, meshes, decals, labels, Niagara systems, or persistent scene objects are created.

## 2. Graphify Investigation Before Changes

Before editing, ran:

```powershell
graphify update .
```

Initial graph: 281 nodes, 380 edges, 18 communities.

Queries/explains used:

- `graphify explain "SHeightmapTrackerImageView"`
- `graphify explain "SLandscapeHeightmapTrackerPanel"`
- `graphify explain "FLandscapeCoordinateMapper"`
- `graphify explain "FLandscapeHeightmapTrackerEdMode"`
- `graphify query "Where is the heightmap image draw rectangle calculated and how is MarkerUV rendered?"`
- `graphify query "How does a viewport click flow from FLandscapeHeightmapTrackerEdMode to FLandscapeCoordinateMapper?"`
- `graphify query "Where are Flip X and Flip Y applied?"`
- `graphify query "How is AssignedLandscape validated and how are Landscape bounds calculated?"`

Findings:

- `SHeightmapTrackerImageView` owned image drawing and 2D marker rendering.
- `SLandscapeHeightmapTrackerPanel` owned UI state, Landscape assignment, bounds, texture loading, and viewport click handling.
- `FLandscapeCoordinateMapper` only supported World/Local to UV/Pixel mapping.
- `FLandscapeHeightmapTrackerEdMode` observed viewport left clicks and used `FViewportTraceRayBuilder`.

## 3. Existing Runtime Call Chain

Verified by source inspection:

```text
FLandscapeHeightmapTrackerEdMode::InputKey
-> TraceLandscapeClick
-> FViewportTraceRayBuilder::BuildTraceSegment
-> LineTraceSingleByChannel
-> FLandscapeHeightmapTrackerModule::OnViewportClickResult
-> SLandscapeHeightmapTrackerPanel::OnViewportClick
-> IsAssignedLandscapeHit
-> RefreshLandscapeBounds
-> FLandscapeCoordinateMapper::MapWorldPosition
-> FLandscapeCoordinateMapper::MapLocalPosition
-> 2D marker state
```

## 4. Files Changed

- `Source/LandscapeHeightmapTracker/Public/LandscapeCoordinateMapper.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeCoordinateMapper.cpp`
- `Source/LandscapeHeightmapTracker/Private/HeightmapImageClickMapper.*`
- `Source/LandscapeHeightmapTracker/Private/LandscapeSurfaceTraceHelper.*`
- `Source/LandscapeHeightmapTracker/Public/LandscapeHeightmapTrackerModule.h`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerModule.cpp`
- `Source/LandscapeHeightmapTracker/Private/LandscapeHeightmapTrackerEdMode.*`
- `Source/LandscapeHeightmapTracker/Public/SLandscapeHeightmapTrackerPanel.h`
- `Source/LandscapeHeightmapTracker/Private/SLandscapeHeightmapTrackerPanel.cpp`
- `Source/LandscapeHeightmapTracker/Private/Tests/*`
- `README.md`
- `CHANGELOG.md`
- `Docs/ARCHITECTURE.md`
- `Docs/TEST_PLAN.md`

## 5. Reverse Mapping Architecture

```text
SHeightmapTrackerImageView::OnMouseButtonDown
-> FHeightmapImageClickMapper::CalculateFittedImageRect
-> FHeightmapImageClickMapper::MapLocalPositionToDisplayUV
-> SLandscapeHeightmapTrackerPanel::OnHeightmapClicked
-> FLandscapeCoordinateMapper::MapUVToLocalPosition
-> Assigned Landscape transform
-> FLandscapeSurfaceTraceHelper::TraceAssignedLandscapeSurface
-> FLandscapeHeightmapTrackerModule::SetReverseMarker
-> FLandscapeHeightmapTrackerEdMode::Render
```

## 6. Image Click to UV Strategy

The image widget now uses the same fitted draw rectangle for paint and click hit testing. Letterbox/pillarbox clicks are rejected and do not move the 3D marker.

## 7. Flip Inversion Strategy

`FLandscapeCoordinateMapper::MapUVToLocalPosition()` owns inverse Flip X / Flip Y logic. Panel and Slate code pass options only; they do not duplicate flip math.

## 8. Local XY to World XY Strategy

Reverse local XY is transformed with `AssignedLandscape->GetActorTransform().TransformPosition()`. This avoids origin, scale, rotation, and square-Landscape assumptions.

## 9. Vertical Assigned Landscape Trace Strategy

`FLandscapeSurfaceTraceHelper` builds a finite vertical segment and uses `LineTraceMultiByChannel`. It scans hits and accepts the first hit belonging to the assigned Landscape.

## 10. Why PNG Is Not Used For Z

The PNG is used only for 2D XY selection. Final World Z comes from `FHitResult::ImpactPoint` on the assigned Landscape surface, so current sculpted terrain is respected when collision is current.

## 11. Vertical Line Rendering Strategy

`FLandscapeHeightmapTrackerEdMode::Render()` draws one foreground vertical line with `FPrimitiveDrawInterface`. The module keeps the EdMode active while either click tracking is enabled or a reverse marker exists.

## 12. Lifecycle Handling

- Landscape is stored as weak pointer in the panel.
- Reverse marker owner is stored as weak actor pointer in the module.
- Marker clears on panel destruction, clear-all, Landscape reassignment, invalid owner, and module shutdown. Invalid-owner cleanup is deferred from `Render()` into EdMode `Tick()` so mode activation/redraw state changes do not run directly inside the draw path.
- Tracking toggle no longer controls reverse marker visibility.

## 13. Tests Added

- `LandscapeHeightmapTracker.ReverseMapping.UVToLocal`
- `LandscapeHeightmapTracker.ReverseMapping.FittedImageClick`
- `LandscapeHeightmapTracker.ReverseMapping.VerticalTraceSegment`

## 14. Build Command / Result

Command:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat' BuildPlugin -Plugin='J:\Projects\UE_Projects\LandscapeHeightmapTracker\LandscapeHeightmapTracker.uplugin' -Package='J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\Package-UE5-10' -TargetPlatforms=Win64 -StrictIncludes
```

Result: succeeded.

Note: first run failed due missing `AActor` forward declaration in `LandscapeSurfaceTraceHelper.h`; fixed and reran successfully.

## 15. Automation Command / Result

Used `Build\TestHost\TestHost.uproject` with the freshly packaged plugin copied into `Build\TestHost\Plugins\LandscapeHeightmapTracker`.

Commands:

```powershell
& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\TestHost\TestHost.uproject' -unattended -nop4 -nosplash -NullRHI -ExecCmds='Automation RunTests LandscapeHeightmapTracker.Mapper; Quit' -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\TestHost\TestHost.uproject' -unattended -nop4 -nosplash -NullRHI -ExecCmds='Automation RunTests LandscapeHeightmapTracker.ViewportTrace; Quit' -TestExit='Automation Test Queue Empty'

& 'C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' 'J:\Projects\UE_Projects\LandscapeHeightmapTracker\Build\TestHost\TestHost.uproject' -unattended -nop4 -nosplash -NullRHI -ExecCmds='Automation RunTests LandscapeHeightmapTracker.ReverseMapping; Quit' -TestExit='Automation Test Queue Empty'
```

Result: all three command groups exited with code 0.

Note: `Automation RunTests LandscapeHeightmapTracker` only discovered the two Mapper tests in this host, so the required families were run explicitly by prefix.

## 16. Exact Discovered Tests

Mapper:

- `LandscapeHeightmapTracker.Mapper.Basic` - Success
- `LandscapeHeightmapTracker.Mapper.OptionsAndTransforms` - Success

ViewportTrace:

- `LandscapeHeightmapTracker.ViewportTrace.InvalidDirection` - Success
- `LandscapeHeightmapTracker.ViewportTrace.Orthographic` - Success
- `LandscapeHeightmapTracker.ViewportTrace.Perspective` - Success
- `LandscapeHeightmapTracker.ViewportTrace.ReversedOrthographic` - Success

ReverseMapping:

- `LandscapeHeightmapTracker.ReverseMapping.FittedImageClick` - Success
- `LandscapeHeightmapTracker.ReverseMapping.UVToLocal` - Success
- `LandscapeHeightmapTracker.ReverseMapping.VerticalTraceSegment` - Success

## 17. Manual Validation

Not manually validated in an interactive UE viewport. Automation and packaged plugin build were completed. Scenarios requiring visual inspection, sculpted Landscape collision, and blocker mesh validation still need manual editor verification.

## 18. Graphify Update After Changes

Ran:

```powershell
graphify update .
```

## 19. Updated Graph Counts

Post-change graph: 376 nodes, 538 edges, 18 communities.

Post-change Graphify checks:

- `graphify explain "FLandscapeCoordinateMapper::MapUVToLocalPosition"`
- `graphify explain "FHeightmapImageClickMapper"`
- `graphify explain "FLandscapeSurfaceTraceHelper"`
- `graphify query "How does the new 2D heightmap click reverse mapping path reach the editor viewport vertical line?"`
- `graphify query "Which new tests cover reverse mapping fitted image clicks and vertical trace construction?"`

New graph-visible nodes/functions:

- `FLandscapeCoordinateMapper::MapUVToLocalPosition`
- `FHeightmapImageClickMapper`
- `CalculateFittedImageRect`
- `MapLocalPositionToDisplayUV`
- `FLandscapeSurfaceTraceHelper`
- `BuildVerticalTraceSegment`
- `HitBelongsToAssignedLandscape`
- `TraceAssignedLandscapeSurface`
- `SLandscapeHeightmapTrackerPanel::OnHeightmapClicked`
- `FLandscapeHeightmapTrackerEdMode::Render`
- `SetReverseMarker`
- `ClearReverseMarker`

The graph shows the reverse path from image click handling through fitted image UV, reverse coordinate mapping, surface trace helper, module marker state, and EdMode viewport rendering.

## 20. Known Limitations

- Full editor-world collision behavior is not automation-tested; only pure trace segment construction is covered.
- Manual validation for moved Landscape, non-default XY scale, sculpted Z, blocker mesh, and map-change visual behavior remains required.
- The automation CLI behaved better when each test family was run by explicit prefix.

## 21. Recommended Next Ticket

Add an editor integration test map or deterministic test harness for `LineTraceMultiByChannel` against a generated Landscape and blocker mesh, so current surface Z and blocker-pass-through behavior can be validated without manual steps.
